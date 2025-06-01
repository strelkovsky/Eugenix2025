#pragma once

#include <iostream>

#include "CompileConfig.h"
#include "Platform.h"

namespace Eugenix
{
	enum struct LogSeverity
	{
		Info,
		Warning,
		Error,
		Verbose,
		Success
	};

	enum struct LogLevel
	{
		Silent,
		ErrorOnly,
		Warnings,
		All
	};

#ifndef EUGENIX_LOG_LEVEL
	constexpr LogLevel CurrentLogLevel = LogLevel::All;
#else
	constexpr LogLevel CurrentLogLevel = static_cast<LogLevel>(EUGENIX_LOG_LEVEL);
#endif

	template<LogSeverity Severity, typename... Args>
	void LogFmt(Args&&... args)
	{
		if constexpr (
			(Severity == LogSeverity::Error && CurrentLogLevel >= LogLevel::ErrorOnly) ||
			(Severity == LogSeverity::Warning && CurrentLogLevel >= LogLevel::Warnings) ||
			(Severity == LogSeverity::Info && CurrentLogLevel >= LogLevel::All) ||
			(Severity == LogSeverity::Verbose && CurrentLogLevel == LogLevel::All) ||
			(Severity == LogSeverity::Success && CurrentLogLevel >= LogLevel::All))
		{
#if defined(EUGENIX_PLATFORM_WINDOWS)
			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			CONSOLE_SCREEN_BUFFER_INFO info;
			GetConsoleScreenBufferInfo(hConsole, &info);

			WORD original = info.wAttributes;
			if constexpr (Severity == LogSeverity::Error) SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
			else if constexpr (Severity == LogSeverity::Warning) SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
			else if constexpr (Severity == LogSeverity::Success) SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
#endif

			if constexpr (Severity == LogSeverity::Error)   std::cerr << "[Error] ";
			if constexpr (Severity == LogSeverity::Warning) std::cerr << "[Warn] ";
			if constexpr (Severity == LogSeverity::Info)    std::cerr << "[Info] ";
			if constexpr (Severity == LogSeverity::Verbose) std::cerr << "[Verbose] ";
			((std::cerr << std::forward<Args>(args)), ...) << '\n';

#if defined(EUGENIX_PLATFORM_WINDOWS)
			SetConsoleTextAttribute(hConsole, original);
#endif
		}
	}

	inline void LogInfo(auto&&... args) { LogFmt<LogSeverity::Info>(std::forward<decltype(args)>(args)...); }
	inline void LogWarn(auto&&... args) { LogFmt<LogSeverity::Warning>(std::forward<decltype(args)>(args)...); }
	inline void LogError(auto&&... args) { LogFmt<LogSeverity::Error>(std::forward<decltype(args)>(args)...); }
	inline void LogVerbose(auto&&... args) { LogFmt<LogSeverity::Verbose>(std::forward<decltype(args)>(args)...); }
	inline void LogSuccess(auto&&... args) { LogFmt<LogSeverity::Success>(std::forward<decltype(args)>(args)...); }
}