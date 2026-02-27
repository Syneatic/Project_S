#pragma once

namespace
{
    template <typename T, typename = void>
    struct is_container : std::false_type {};

    template <typename T>
    struct is_container<T, std::void_t<decltype(std::declval<T>().begin()),
        decltype(std::declval<T>().end())>>
        : std::true_type {};

    template <typename T>
    void PrintElement(const T& val) {
        if constexpr (is_container<T>::value && !std::is_same_v<T, std::string>) {
            std::cout << "[ ";
            for (const auto& i : val) { std::cout << i << " "; }
            std::cout << "]";
        }
        else {
            std::cout << val;
        }
    }
}

namespace Debug
{
#ifdef _DEBUG
    #define DEBUG_TIMER(name) Debug::ScopedTimer timer##__LINE__(name)
    #define DEBUG_FUNC_TIMER() DEBUG_TIMER(__FUNCTION__)
#else
    #define DEBUG_TIMER(name)
    #define DEBUG_FUNC_TIMER()
#endif


#ifdef _DEBUG
    template<typename... Args>
    void Log(Args&&... args)
    {
        (..., (PrintElement(std::forward<Args>(args)), std::cout << " "));
        std::cout << std::endl;
    }
#else
    //compiler will optimize this away
    template<typename... Args>
    inline void Log(Args&&...) {} //empty function inline means nothing is written
#endif


    struct TimerResult {
        std::string Name;
        double Milliseconds;
    };


    void RecordTimerResult(const TimerResult& result);

    std::vector<TimerResult> GetResultsAndClear();

    class ScopedTimer
    {
    public:
        ScopedTimer(std::string_view name)
            : m_Name(name), m_StartTime(std::chrono::high_resolution_clock::now()) {
        }

        ~ScopedTimer()
        {
            auto endTime = std::chrono::high_resolution_clock::now();
            auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTime).time_since_epoch().count();
            auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTime).time_since_epoch().count();

            double ms = (end - start) * 0.001;

            // Instead of just logging, we send it to the central registry
            RecordTimerResult({ std::string(m_Name), ms });
        }

    private:
        std::string_view m_Name;
        std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTime;
    };
}