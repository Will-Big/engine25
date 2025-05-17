#pragma once
#include <memory>
#include <string>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace core
{
    class ILogSink
	{
	public:
		virtual ~ILogSink() = default;
		virtual void OnMessage(const std::string& msg, int level) = 0; // 들어온 메시지를 표현하는 사용자 정의 방식
    };

    class Log {
    public:
        static void Init();
        static void AddSink(std::shared_ptr<ILogSink> sink);
        static void DispatchToSinks(const std::string& msg, int level);
        static std::shared_ptr<spdlog::logger>& GetLogger() { return s_Logger; }

    private:
        static std::shared_ptr<spdlog::logger> s_Logger {};
        static std::vector<std::shared_ptr<ILogSink>> s_ExternalSinks {};
    };

#pragma region IMPLEMENTS
    inline void Log::Init()
    {
        // 다중 sink 설정
        std::vector<spdlog::sink_ptr> sinks;

#ifdef _MSC_VER
        // MSVC 전용 로그 sink
        sinks.push_back(std::make_shared<spdlog::sinks::msvc_sink_mt>());
#else
        // 일반 콘솔 로그 sink
        sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
#endif

        // 로그 파일 출력
        sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/engine.log", true));

        s_Logger = std::make_shared<spdlog::logger>("CORE", sinks.begin(), sinks.end());
        s_Logger->set_pattern("[%T] [%^%l%$] %v");
        s_Logger->set_level(spdlog::level::trace);  // 로그 수준 설정
    }

    inline void Log::AddSink(std::shared_ptr<ILogSink> sink)
    {
        s_ExternalSinks.push_back(std::move(sink));
    }

    inline void Log::DispatchToSinks(const std::string& msg, int level)
    {
        for (auto& sink : s_ExternalSinks)
            if (sink)
                sink->OnMessage(msg, level);
    }
#pragma endregion

#pragma region LOG_FUNCS
    // 전역 출력 래퍼
    template<typename... Args>
    void LogTrace(fmt::format_string<Args...> fmt, Args&&... args)
	{
		const std::string msg = fmt::format(fmt, std::forward<Args>(args)...);
        Log::GetLogger()->trace(msg);
        Log::DispatchToSinks(msg, 1);
    }

    template<typename... Args>
    void LogInfo(fmt::format_string<Args...> fmt, Args&&... args)
	{
		const std::string msg = fmt::format(fmt, std::forward<Args>(args)...);
        Log::GetLogger()->info(msg);
        Log::DispatchToSinks(msg, 2);
    }

    template<typename... Args>
    void LogWarn(fmt::format_string<Args...> fmt, Args&&... args)
	{
		const std::string msg = fmt::format(fmt, std::forward<Args>(args)...);
        Log::GetLogger()->warn(msg);
        Log::DispatchToSinks(msg, 3);
    }

    template<typename... Args>
    void LogError(fmt::format_string<Args...> fmt, Args&&... args)
	{
		const std::string msg = fmt::format(fmt, std::forward<Args>(args)...);
        Log::GetLogger()->error(msg);
        Log::DispatchToSinks(msg, 4);
    }

    template<typename... Args>
    void LogCritical(fmt::format_string<Args...> fmt, Args&&... args)
	{
		const std::string msg = fmt::format(fmt, std::forward<Args>(args)...);
        Log::GetLogger()->critical(msg);
        Log::DispatchToSinks(msg, 5);
    }

    template<typename... Args>
    void AssertFormat(bool condition, fmt::format_string<Args...> fmt, Args&&... args)
	{
        if (!condition) 
        {
            std::string msg = fmt::format(fmt, std::forward<Args>(args)...);
            LogError("Assertion Failed: {}", msg);
#ifdef _MSC_VER
            __debugbreak();
#else
            std::abort();
#endif
        }
    }

#ifdef _DEBUG
	#define CORE_ASSERT(x, ...) ::core::AssertFormat((x), __VA_ARGS__)
#else
	#define CORE_ASSERT(x, ...)
#endif
#pragma endregion
}
