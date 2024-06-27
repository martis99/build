#!/bin/sh

if [ "$#" -ne 2 ]; then
	echo "Usage: $0 <Debug/Release> <x86_64/i386>"
	exit 1
fi

config="$1"
arch="$2"

ret=0

#make -s build/compile CONFIG="$config" PLATFORM=x64
#./bin/"$config"-x64/build/build -S itests -D 1 -C 1 -I 1

check()
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

check "asm" "ASM: test"
check "asmc" "ASMC: test"
check "nasm" "NASM: test"
check "nasmc" "NASMC: test"
check "c" "C: test"
check "cpp" "CPP: test"

check "all" "
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

make -s -C itests binutils-2.40/compile ARCH="$arch"

make -s -C itests fat12/run CONFIG="$config" ARCH="$arch" > qemu.log
make -s -C itests binf/bin CONFIG="$config" ARCH="$arch"
make -s -C itests artifact CONFIG="$config" ARCH="$arch"

exit $ret
