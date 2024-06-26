#ifndef LIBASM_H
#define LIBASM_H

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

#if defined(C_WIN) && defined(LIBASM_BUILD_DLL)
	#define LIBASMAPI __declspec(dllexport)
#elif defined(C_WIN) && defined(LIBASM_DLL)
	#define LIBASMAPI __declspec(dllimport)
#elif defined(C_LINUX) && defined(LIBASM_BUILD_DLL)
	#define LIBASMAPI __attribute__((visibility("default")))
#else
	#define LIBASMAPI
#endif

#ifdef __cplusplus
extern "C" {
#endif

int libasm_args(int argc, char **argv);
LIBASMAPI int dlibasm_args(int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif
