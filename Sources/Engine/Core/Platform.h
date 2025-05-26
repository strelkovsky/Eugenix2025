#pragma once

// Platform OS
#define EUGENIX_PLATFORM_WINDOWS 0
#define EUGENIX_PLATFORM_ANDROID 0
#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(WIN32) || defined(_WINDOWS)
#	undef  EUGENIX_PLATFORM_WINDOWS
#	define EUGENIX_PLATFORM_WINDOWS 1
#	define WIN32_LEAN_AND_MEAN
#	define WIN32_EXTRA_LEAN
#	define NOMINMAX
#	include <windows.h>
#elif defined(__ANDROID__) || defined(ANDROID) || defined(_ANDROID)
#	undef  EUGENIX_PLATFORM_ANDROID
#	define EUGENIX_PLATFORM_ANDROID __ANDROID_API__
#else
#	error Unknown platform.
#endif

// Debug
#define EUGENIX_DEBUG 0
#if (defined(_DEBUG) || defined(DEBUG)) && !defined(NDEBUG)
#	undef  EUGENIX_DEBUG
#	define EUGENIX_DEBUG 1
#endif // SE_DEBUG

// Compiler
#define EUGENIX_COMPILER_MSVC 0
#if defined(_MSC_VER) && (_MSC_VER >= 1900)
#	undef  EUGENIX_COMPILER_MSVC
#	define EUGENIX_COMPILER_MSVC _MSC_VER
#else
#	error "Unknown compiler."
#endif

#if EUGENIX_COMPILER_MSVC
#	define EUGENIX_PRAGMA_WARNING_PUSH             __pragma(warning(push))
#	define EUGENIX_PRAGMA_WARNING_LEVEL(level)     __pragma(warning(push, level))
#	define EUGENIX_PRAGMA_WARNING_POP              __pragma(warning(pop))
#	define EUGENIX_PRAGMA_WARNING_DISABLE_MSVC(id) __pragma(warning(disable: id))
#else
#	define EUGENIX_PRAGMA_WARNING_PUSH
#	define EUGENIX_PRAGMA_WARNING_LEVEL(level)
#	define EUGENIX_PRAGMA_WARNING_POP
#	define EUGENIX_PRAGMA_WARNING_DISABLE_MSVC(id)
#endif // SE_COMPILER_*