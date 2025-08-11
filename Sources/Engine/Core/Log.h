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

	template<LogSeverity Severity, class... Args>
	void LogFmt(std::format_string<Args...> fmt, Args&&... args)
	{
		if constexpr (
			(Severity == LogSeverity::Error && CurrentLogLevel >= LogLevel::ErrorOnly) ||
			(Severity == LogSeverity::Warning && CurrentLogLevel >= LogLevel::Warnings) ||
			(Severity == LogSeverity::Info && CurrentLogLevel >= LogLevel::All) ||
			(Severity == LogSeverity::Verbose && CurrentLogLevel == LogLevel::All) ||
			(Severity == LogSeverity::Success && CurrentLogLevel >= LogLevel::All))
		{
#if defined(EUGENIX_PLATFORM_WINDOWS)
			HANDLE hConsole = GetStdHandle(STD_ERROR_HANDLE);
			CONSOLE_SCREEN_BUFFER_INFO info;
			GetConsoleScreenBufferInfo(hConsole, &info);

			WORD original = info.wAttributes;
			if constexpr (Severity == LogSeverity::Error) SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
			else if constexpr (Severity == LogSeverity::Warning) SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
			else if constexpr (Severity == LogSeverity::Success) SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
#else 
			if constexpr (Severity == LogSeverity::Error)   std::cerr << "\033[31m";
			else if constexpr (Severity == LogSeverity::Warning) std::cerr << "\033[33m";
			else if constexpr (Severity == LogSeverity::Success) std::cerr << "\033[32m";
#endif

			if constexpr (Severity == LogSeverity::Error)   std::cerr << "[Error] ";
			if constexpr (Severity == LogSeverity::Warning) std::cerr << "[Warn] ";
			if constexpr (Severity == LogSeverity::Info)    std::cerr << "[Info] ";
			if constexpr (Severity == LogSeverity::Verbose) std::cerr << "[Verbose] ";

			std::cerr << std::format(fmt, std::forward<Args>(args)...) << '\n';

// reset color
#if defined(EUGENIX_PLATFORM_WINDOWS)
			SetConsoleTextAttribute(hConsole, original);
#else
			std::cerr << "\033[0m";
#endif
		}
	}

	template<class... Args> inline void LogInfo(std::format_string<Args...> fmt, Args&&... args) { LogFmt<Eugenix::LogSeverity::Info   >(fmt, std::forward<Args>(args)...); }
	template<class... Args> inline void LogWarn(std::format_string<Args...> fmt, Args&&... args) { LogFmt<Eugenix::LogSeverity::Warning>(fmt, std::forward<Args>(args)...); }
	template<class... Args> inline void LogError(std::format_string<Args...> fmt, Args&&... args) { LogFmt<Eugenix::LogSeverity::Error  >(fmt, std::forward<Args>(args)...); }
	template<class... Args> inline void LogVerbose(std::format_string<Args...> fmt, Args&&... args) { LogFmt<Eugenix::LogSeverity::Verbose>(fmt, std::forward<Args>(args)...); }
	template<class... Args> inline void LogSuccess(std::format_string<Args...> fmt, Args&&... args) { LogFmt<Eugenix::LogSeverity::Success>(fmt, std::forward<Args>(args)...); }
}