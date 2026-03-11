#include "debug.hpp"

namespace Debug
{
    static std::vector<TimerResult> s_Results;
    static std::mutex s_TimerMutex;

    //called on destructor of ScopedTimer
    void RecordTimerResult(const TimerResult& result)
    {
        //allows multithread
        std::lock_guard<std::mutex> lock(s_TimerMutex);
        //push the results
        s_Results.push_back(result);

        //log it
        Debug::Log("[TIMER]", result.Name, ":", result.Milliseconds, "ms");
    }

    std::vector<TimerResult> GetResultsAndClear()
    {
        std::lock_guard<std::mutex> lock(s_TimerMutex);
        std::vector<TimerResult> frameData = std::move(s_Results);
        s_Results.reserve(frameData.size());

        return frameData;
    }
}

namespace FrameProfiler
{
    static std::mutex          s_Mutex;
    static size_t              s_TargetFrames = 60;   // default: 60 frames
    static size_t              s_FrameCount = 0;
    static std::vector<double> s_Samples;
    static bool s_Armed = false;

    void Start()
    {
        std::lock_guard<std::mutex> lock(s_Mutex);
        s_Samples.clear();
        s_FrameCount = 0;
        s_Armed = true;
    }

    void Stop()
    {
        std::lock_guard<std::mutex> lock(s_Mutex);
        s_Armed = false;
        s_Samples.clear();
        s_FrameCount = 0;
    }

    bool IsRunning()
    {
        std::lock_guard<std::mutex> lock(s_Mutex);
        return s_Armed;
    }


    static double CalcMean(std::vector<double>& v)
    {
        if (v.empty()) return 0.0;
        return std::accumulate(v.begin(), v.end(), 0.0) / static_cast<double>(v.size());
    }

    static double CalcMedian(std::vector<double> v)   // copy so we can sort
    {
        if (v.empty()) return 0.0;
        std::sort(v.begin(), v.end());
        size_t n = v.size();
        if (n % 2 == 0)
            return (v[n / 2 - 1] + v[n / 2]) / 2.0;
        else
            return v[n / 2];
    }

    static std::string BuildReport()
    {
        std::lock_guard<std::mutex> lock(s_Mutex);

        std::ostringstream ss;
        ss << std::fixed << std::setprecision(4);

        ss << "========================================\n";
        ss << "  Frame Profiler Report\n";
        ss << "  Frames sampled : " << s_FrameCount << "\n";
        ss << "  Total samples  : " << s_Samples.size() << "\n";
        ss << "========================================\n\n";

        if (s_Samples.empty())
        {
            ss << "  No timer data recorded.\n";
        }
        else
        {
            double mean = CalcMean(s_Samples);
            double median = CalcMedian(s_Samples);  // CalcMedian takes a copy

            // sort a copy for min/max/percentiles
            std::vector<double> sorted = s_Samples;
            std::sort(sorted.begin(), sorted.end());

            double minVal = sorted.front();
            double maxVal = sorted.back();

            // p95 index
            size_t p95idx = static_cast<size_t>(std::ceil(0.95 * sorted.size())) - 1;
            double p95 = sorted[std::min(p95idx, sorted.size() - 1)];

            ss << "  All timer samples (flat list)\n";
            ss << "  --------------------------------\n";
            ss << "  Mean      : " << mean << " ms\n";
            ss << "  Median    : " << median << " ms\n";
            ss << "  Min       : " << minVal << " ms\n";
            ss << "  Max       : " << maxVal << " ms\n";
            ss << "  95th pct  : " << p95 << " ms\n";
            ss << "\n";

            // Raw sample dump  (one per line, easy to import elsewhere)
            ss << "  Raw samples (ms):\n";
            for (size_t i = 0; i < sorted.size(); ++i)
                ss << "    [" << std::setw(5) << i << "] " << sorted[i] << "\n";
        }

        ss << "\n========================================\n";

        // clear state so profiler can be reused
        s_Samples.clear();
        s_FrameCount = 0;

        return ss.str();
    }

    void SetTargetFrameCount(size_t frameCount)
    {
        std::lock_guard<std::mutex> lock(s_Mutex);
        s_TargetFrames = frameCount > 0 ? frameCount : 1;
    }

    void CommitFrame()
    {
        // Drain whatever the ScopedTimers recorded this frame
        std::vector<Debug::TimerResult> frameResults = Debug::GetResultsAndClear();

        std::lock_guard<std::mutex> lock(s_Mutex);
        if (!s_Armed) return;  // <-- just bail early if not started

        for (auto& r : frameResults)
            s_Samples.push_back(r.Milliseconds);

        ++s_FrameCount;
    }

    bool IsReady()
    {
        std::lock_guard<std::mutex> lock(s_Mutex);
        return s_FrameCount >= s_TargetFrames;
    }

    std::string FlushToString()
    {
        return BuildReport(); 
    }

    void FlushToFile(const std::string& filePath)
    {
        std::string report = FlushToString();

        std::ofstream file(filePath);
        if (!file.is_open())
        {
            Debug::Log("[FrameProfiler] ERROR: could not open file:", filePath);
            return;
        }

        file << report;
        Debug::Log("[FrameProfiler] Report written to:", filePath);
    }
}