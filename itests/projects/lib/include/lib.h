#ifndef LIB_H
#define LIB_H

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

#if defined(C_WIN) && defined(LIB_BUILD_DLL)
	#define LIBAPI __declspec(dllexport)
#elif defined(C_WIN) && defined(LIB_DLL)
	#define LIBAPI __declspec(dllimport)
#elif defined(C_LINUX) && defined(LIB_BUILD_DLL)
	#define LIBAPI __attribute__((visibility("default")))
#else
	#define LIBAPI
#endif

#ifdef __cplusplus
extern "C" {
#endif

int lib_args(int argc, char **argv);
LIBAPI int dlib_args(int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif
