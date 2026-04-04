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