#ifndef LIBCC_H
#define LIBCC_H

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

#if defined(C_WIN) && defined(LIBCC_BUILD_DLL)
	#define LIBCCAPI __declspec(dllexport)
#elif defined(C_WIN) && defined(LIBCC_DLL)
	#define LIBCCAPI __declspec(dllimport)
#elif defined(C_LINUX) && defined(LIBCC_BUILD_DLL)
	#define LIBCCAPI __attribute__((visibility("default")))
#else
	#define LIBCCAPI
#endif

#ifdef __cplusplus
extern "C" {
#endif

int libcc_args(int argc, char **argv);
LIBCCAPI int dlibcc_args(int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif
