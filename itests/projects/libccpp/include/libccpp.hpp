#pragma once

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
	#define C_WIN
	#if defined(_WIN64)
		#define C_WIN64
	#else
		#define C_WIN32
	#endif
#elif __linux__
	#define C_LINUX
#else
	#error "Platform not supported"
#endif

#if defined(C_WIN) && defined(LIBCCPP_BUILD_DLL)
	#define LIBCCPPAPI __declspec(dllexport)
#elif defined(C_WIN) && defined(LIBCCPP_DLL)
	#define LIBCCPPAPI __declspec(dllimport)
#elif defined(C_LINUX) && defined(LIBCCPP_BUILD_DLL)
	#define LIBCCPPAPI __attribute__((visibility("default")))
#else
	#define LIBCCPPAPI
#endif

int libccpp_args(int argc, char **argv);
LIBCCPPAPI int dlibccpp_args(int argc, char **argv);
