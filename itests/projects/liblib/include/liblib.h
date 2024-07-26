#ifndef LIBLIB_H
#define LIBLIB_H

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

#if defined(C_WIN) && defined(LIBLIB_BUILD_DLL)
	#define LIBLIBAPI __declspec(dllexport)
#elif defined(C_WIN) && defined(LIBLIB_DLL)
	#define LIBLIBAPI __declspec(dllimport)
#elif defined(C_LINUX) && defined(LIBLIB_BUILD_DLL)
	#define LIBLIBAPI __attribute__((visibility("default")))
#else
	#define LIBLIBAPI
#endif

#ifdef __cplusplus
extern "C" {
#endif

int liblib_args(int argc, char **argv);
LIBLIBAPI int dliblib_args(int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif
