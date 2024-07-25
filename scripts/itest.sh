#!/bin/sh

if [ "$#" -eq 1 ]; then
	build="$1"
	debug="0"
elif [ "$#" -eq 2 ] && [ "$2" = "d" ]; then
	build="$1"
	debug="1"
else
	echo "Usage: $0 <build_exe> [d]"
	exit 1
fi

if [ "$debug" = "1" ]; then
	build_flags="-D 1 -C 1 -I 1"
	make_flags=""
else
	build_flags=""
	make_flags="-s"
fi

ret=0

status() {
	printf "%-5s %-6s %-7s %-6s " "$1" "$2" "$3" "$4"
}

check_exe() {
	status "$1" "$2" "$3" "$4"

	if ! res="$(cd ./itests/projects/"$4" && ../../bin/"$2"-"$3"/projects/"$4"/"$4" test)"; then
		printf "\033[0;31mFAILED\033[0m\n"
		ret=1
		return $ret
	fi

	if [ "$res" != "$5" ]; then
		printf "\033[0;31mFAILED\033[0m\n"
		printf "exp: '\033[0;31m%s\033[0m'\n" "$5"
		printf "act: '\033[0;31m%s\033[0m'\n" "$res"
		ret=1
	else
		printf "\033[0;32mOK\033[0m\n"
	fi
}

mk_build() {
	return
}

mk_check() {
	mkdir -p itests/tmp/build/mk/"$1"-"$2"
	if ! make -C itests ARCH="$1" CONFIG="$2" >itests/tmp/build/mk/"$1"-"$2"/build.log 2>&1; then
		printf "\033[0;31m[Make] Failed to build %s-%s\033[0m\n" "$1" "$2"
		cat itests/tmp/build/mk/"$1"-"$2"/build.log
		ret=1
		return
	fi
	
	check_exe "mk" "$1" "$2" "asm" "ASM: test"
	check_exe "mk" "$1" "$2" "asmc" "ASMC: test"
	check_exe "mk" "$1" "$2" "nasm" "NASM: test"
	check_exe "mk" "$1" "$2" "nasmc" "NASMC: test"
	check_exe "mk" "$1" "$2" "c" "C: test"
	check_exe "mk" "$1" "$2" "cpp" "CPP: test"
	check_exe "mk" "$1" "$2" "allp" "
ASM: test
LIBASM: test
DLIBASM: test
C: test
LIBC: test
DLIBC: test
CPP: test
LIBCPP: test
DLIBCPP: test
EXTLIB: test
DEXTLIB: test"

	status "mk" "$1" "$2" "hello"
	if ! res="$(cd ./itests/projects/hello && ../../bin/"$1"-"$2"/projects/hello/bin/hello)"; then
		printf "\033[0;31mFAILED\033[0m\n"
		ret=1
	fi

	if [ "$res" != "Hello, world!" ]; then
		printf "\033[0;31mFAILED\033[0m\n"
		printf "exp: '\033[0;31m%s\033[0m'\n" "Hello, world!"
		printf "act: '\033[0;31m%s\033[0m'\n" "$res"
		ret=1
	else
		printf "\033[0;32mOK\033[0m\n"
	fi

	#TODO: Add exe checks (ldd)

	status "mk" "$1" "$2" "fat12"
	mkdir -p itests/tmp/logs/"$1"-"$2"/projects/fat12
	if ! make $make_flags -C itests fat12/run_fat12 ARCH="$1" CONFIG="$2" >itests/tmp/logs/"$1"-"$2"/projects/fat12/run.log 2>&1; then
		printf "\033[0;31mFAILED\033[0m\n"
		ret=1
	else
		printf "\033[0;32mOK\033[0m\n"
	fi
	
	if ! make -C itests artifact ARCH="$1" CONFIG="$2" >itests/tmp/build/mk/"$1"-"$2"/artifact.log 2>&1; then
		printf "\033[0;31m[Make] Failed to create artifact %s-%s\033[0m\n" "$1" "$2"
		cat itests/tmp/build/mk/"$1"-"$2"/artifact.log
		ret=1
	else
		status "mk" "$1" "$2" "art"
		if [ -e "./itests/tmp/artifact/floppy.img" ]; then
			printf "\033[0;32mOK\033[0m\n"
		else
			printf "\033[0;31mFAILED\033[0m\n"
			ret=1
		fi
	fi

	if ! make -C itests clean ARCH="$1" CONFIG="$2" >itests/tmp/build/mk/"$1"-"$2"/clean.log 2>&1; then
		printf "\033[0;31m[Make] Failed to clean %s-%s\033[0m\n" "$1" "$2"
		cat itests/tmp/build/mk/"$1"-"$2"/clean.log
		ret=1
	fi
}

cm_mk_build() {
	printf "[CMake] Generating Makefiles: %s-%s\n" "$1" "$2"
	mkdir -p itests/tmp/build/cm_mk/"$1"-"$2"
	if ! cmake -S itests -B itests/tmp/build/cm_mk/"$1"-"$2" -G "Unix Makefiles" -DARCH="$1" -DCMAKE_BUILD_TYPE="$2" >itests/tmp/build/cm_mk/"$1"-"$2"/generate.log 2>&1; then
		printf "\r\033[0;31m[CMake] Failed to generate Makefiles\033[0m\n"
		cat itests/tmp/build/cm_mk/"$1"-"$2"/generate.log 
		ret=1
		return
	fi
}

cm_mk_check() {
	mkdir -p itests/tmp/build/cm_mk/"$1"-"$2"
	if ! make -C itests/tmp/build/cm_mk/"$1"-"$2" >itests/tmp/build/cm_mk/"$1"-"$2"/build.log 2>&1; then
		printf "\033[0;31m[CMake] [Make] Failed to build %s-%s\033[0m\n" "$1" "$2"
		cat itests/tmp/build/cm_mk/"$1"-"$2"/build.log
		ret=1
		return
	fi

	check_exe "cm_mk" "$1" "$2" "asm" "ASM: test"
	check_exe "cm_mk" "$1" "$2" "asmc" "ASMC: test"
	check_exe "cm_mk" "$1" "$2" "nasm" "NASM: test"
	check_exe "cm_mk" "$1" "$2" "nasmc" "NASMC: test"
	check_exe "cm_mk" "$1" "$2" "c" "C: test"
	check_exe "cm_mk" "$1" "$2" "cpp" "CPP: test"
	check_exe "cm_mk" "$1" "$2" "allp" "
ASM: test
LIBASM: test
DLIBASM: test
C: test
LIBC: test
DLIBC: test
CPP: test
LIBCPP: test
DLIBCPP: test
EXTLIB: test
DEXTLIB: test"

	if ! make -C itests/tmp/build/cm_mk/"$1"-"$2" clean>itests/tmp/build/cm_mk/"$1"-"$2"/clean.log 2>&1; then
		printf "\033[0;31m[CMake] [Make] Failed to clean %s-%s\033[0m\n" "$1" "$2"
		cat itests/tmp/build/cm_mk/"$1"-"$2"/clean.log
		ret=1
		return
	fi
}

rm -rf itests/bin itests/tmp

# shellcheck disable=SC2086
if ! $build -S itests -G M $build_flags; then
	printf "\033[0;31mFailed to generate Make files\033[0m\n"
	exit 1
else
	printf "\033[0;32mMake files generated successfully\033[0m\n"
fi
mk_build x86_64 Debug
mk_build x86_64 Release
mk_build i386 Debug
mk_build i386 Release

# shellcheck disable=SC2086
if ! $build -S itests -G C $build_flags; then
	printf "\033[0;31m[Build] Failed to generate CMake files\033[0m\n"
	exit 1
fi
cm_mk_build x86_64 Debug
cm_mk_build x86_64 Release
cm_mk_build i386 Debug
cm_mk_build i386 Release

printf "%-5s %-6s %-7s %-6s %s\n" gen arch config target status

mk_check x86_64 Debug
mk_check x86_64 Release
mk_check i386 Debug
mk_check i386 Release

cm_mk_check x86_64 Debug
cm_mk_check x86_64 Release
cm_mk_check i386 Debug
cm_mk_check i386 Release

exit $ret
