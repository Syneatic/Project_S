#pragma once

// ─────────────────────────────────────────────
//  Macros
// ─────────────────────────────────────────────
#define PROFILE_SCOPE(name)   ProfilerScope _prof_##__LINE__(name)
#define PROFILE_FUNCTION()    PROFILE_SCOPE(__FUNCTION__)
#define PROFILE_FRAME_BEGIN() Profiler::Get().BeginFrame()
#define PROFILE_FRAME_END()   Profiler::Get().EndFrame()

// ─────────────────────────────────────────────
//  Data structures
// ─────────────────────────────────────────────

struct ProfileSample
{
    const char*  name       = nullptr;
    double       startMs    = 0.0;   // relative to frame start
    double       durationMs = 0.0;
    int          depth      = 0;     // call-stack depth
    uint32_t     color      = 0;     // auto-assigned per name
};

struct FrameData
{
    std::vector<ProfileSample> samples;
    double                     frameTimeMs  = 0.0;
    size_t                     memAllocated = 0;   // bytes alive at frame end
    size_t                     memAllocCount = 0;  // live allocation count
};

// ─────────────────────────────────────────────
//  Profiler singleton
// ─────────────────────────────────────────────

class Profiler
{
public:
    static constexpr int kMaxFrameHistory = 256;

    static Profiler& Get()
    {
        static Profiler instance;
        return instance;
    }

    // Called once per frame by the game loop
    void BeginFrame()
    {
        m_frameStart  = Clock::now();
        m_depth       = 0;
        m_openSamples.clear();
        m_currentSamples.clear();
    }

    void EndFrame()
    {
        double frameMs = ToMs(Clock::now() - m_frameStart);

        FrameData fd;
        fd.samples       = std::move(m_currentSamples);
        fd.frameTimeMs   = frameMs;
        fd.memAllocated  = m_memAllocatedBytes;
        fd.memAllocCount = m_memAllocCount;

        if ((int)m_frames.size() >= kMaxFrameHistory)
            m_frames.erase(m_frames.begin());
        m_frames.push_back(std::move(fd));
    }

    // Called by ProfilerScope constructor / destructor
    void PushScope(const char* name)
    {
        OpenSample os;
        os.name  = name;
        os.start = Clock::now();
        os.depth = m_depth++;
        m_openSamples.push_back(os);
    }

    void PopScope()
    {
        if (m_openSamples.empty()) return;

        auto& os  = m_openSamples.back();
        double dur = ToMs(Clock::now() - os.start);

        ProfileSample s;
        s.name       = os.name;
        s.startMs    = ToMs(os.start - m_frameStart);
        s.durationMs = dur;
        s.depth      = os.depth;
        s.color      = ColorForName(os.name);

        m_currentSamples.push_back(s);
        m_openSamples.pop_back();
        --m_depth;
    }

    // Memory tracking (called from overridden new/delete)
    void TrackAlloc(size_t bytes)
    {
        m_memAllocatedBytes += bytes;
        ++m_memAllocCount;
    }

    void TrackFree(size_t bytes)
    {
        if (m_memAllocatedBytes >= bytes) m_memAllocatedBytes -= bytes;
        if (m_memAllocCount > 0)          --m_memAllocCount;
    }

    // Access recorded frames
    const std::vector<FrameData>& Frames()    const { return m_frames; }
    const FrameData*              LastFrame()  const
    {
        return m_frames.empty() ? nullptr : &m_frames.back();
    }

    bool IsPaused() const        { return m_paused; }
    void SetPaused(bool p)       { m_paused = p; }

private:
    Profiler() = default;

    using Clock     = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    static double ToMs(std::chrono::duration<double> d)
    {
        return d.count() * 1000.0;
    }

    // Simple deterministic color from name hash
    static uint32_t ColorForName(const char* name)
    {
        uint32_t hash = 2166136261u;
        for (const char* p = name; *p; ++p)
            hash = (hash ^ (uint8_t)*p) * 16777619u;

        uint8_t r = 100 + (hash & 0x7F);
        uint8_t g = 100 + ((hash >> 8) & 0x7F);
        uint8_t b = 100 + ((hash >> 16) & 0x7F);
        return (0xFF << 24) | ((uint32_t)b << 16) | ((uint32_t)g << 8) | r;
    }

    struct OpenSample { const char* name; TimePoint start; int depth; };

    TimePoint              m_frameStart;
    int                    m_depth = 0;
    std::vector<OpenSample> m_openSamples;
    std::vector<ProfileSample> m_currentSamples;
    std::vector<FrameData>     m_frames;

    size_t m_memAllocatedBytes = 0;
    size_t m_memAllocCount     = 0;
    bool   m_paused            = false;
};

// ─────────────────────────────────────────────
//  RAII scope guard
// ─────────────────────────────────────────────

struct ProfilerScope
{
    explicit ProfilerScope(const char* name) { Profiler::Get().PushScope(name); }
    ~ProfilerScope()                         { Profiler::Get().PopScope(); }
    ProfilerScope(const ProfilerScope&)            = delete;
    ProfilerScope& operator=(const ProfilerScope&) = delete;
};
