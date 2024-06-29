#!/bin/sh

if [ "$#" -ne 2 ]; then
	echo "Usage: $0 <Debug/Release> <x86_64/i386>"
	exit 1
fi

config="$1"
arch="$2"

ret=0

#make -s build/compile CONFIG="$config" PLATFORM=x64

mk_check()
{
	printf "%s: " "$1"
	res="$(make -s -C itests "$1"/run CONFIG="$config" ARCH="$arch")"
	if [ "$res" != "$2" ]; then
		printf "\n"
		printf "exp: '\033[0;31m%s\033[0m'\n" "$2"
		printf "act: '\033[0;31m%s\033[0m'\n" "$res"
		ret=1
	else
		printf "'\033[0;32m%s\033[0m'\n" "$res"
	fi
}

./bin/"$config"-x64/build/build -S itests -G M -D 1 -C 1 -I 1

mk_check "asm" "ASM: test"
mk_check "asmc" "ASMC: test"
mk_check "nasm" "NASM: test"
mk_check "nasmc" "NASMC: test"
mk_check "c" "C: test"
mk_check "cpp" "CPP: test"
mk_check "all" "
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

#make -s -C itests binutils-2.40/compile ARCH="$arch"

make -s -C itests fat12/run CONFIG="$config" ARCH="$arch" > qemu.log
make -s -C itests binf/bin CONFIG="$config" ARCH="$arch"
make -s -C itests artifact CONFIG="$config" ARCH="$arch"

rm -rf itests/bin itests/build

./bin/"$config"-x64/build/build -S itests -G C -D 1 -C 1 -I 1

cmake -S itests -B itests/build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=$config -DARCH=$arch
make -C build

cm_check()
{
	printf "%s: " "$1"
	res="$(./itests/bin/"$config"-"$arch"/projects/$1/$1 test)"
	if [ "$res" != "$2" ]; then
		printf "\n"
		printf "exp: '\033[0;31m%s\033[0m'\n" "$2"
		printf "act: '\033[0;31m%s\033[0m'\n" "$res"
		ret=1
	else
		printf "'\033[0;32m%s\033[0m'\n" "$res"
	fi
}

cm_check "asm" "ASM: test"
cm_check "asmc" "ASMC: test"
cm_check "nasm" "NASM: test"
cm_check "nasmc" "NASMC: test"
cm_check "c" "C: test"
cm_check "cpp" "CPP: test"
cm_check "all" "
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

exit $ret
