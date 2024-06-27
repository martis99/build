#ifndef EXTLIB_H
#define EXTLIB_H

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

#if defined(C_WIN) && defined(EXTLIB_BUILD_DLL)
	#define EXTLIBAPI __declspec(dllexport)
#elif defined(C_WIN) && defined(EXTLIB_DLL)
	#define EXTLIBAPI __declspec(dllimport)
#elif defined(C_LINUX) && defined(EXTLIB_BUILD_DLL)
	#define EXTLIBAPI __attribute__((visibility("default")))
#else
	#define EXTLIBAPI
#endif

#ifdef __cplusplus
extern "C" {
#endif

int extlib_args(int argc, char **argv);
EXTLIBAPI int dextlib_args(int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif
