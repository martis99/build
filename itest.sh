#!/bin/sh

if [ "$#" -ne 3 ]; then
	echo "Usage: $0 <build_path> <config> <arch>"
	exit 1
fi

build="$1"
config="$2"
arch="$3"

ret=0

$build -S itests/all -D 1 -C 1 -I 1

check()
{
	printf "%s: " "$1"
	res="$(make -s -C itests/all "$1"/run CONFIG="$config" ARCH="$arch")"
	if [ "$res" != "$2" ]; then
		printf "\n"
		printf "exp: '\033[0;31m%s\033[0m'\n" "$2"
		printf "act: '\033[0;31m%s\033[0m'\n" "$res"
		ret=1
	else
		printf "'\033[0;32m%s\033[0m'\n" "$res"
	fi
}

check "asm_exe" "ASM: test"
check "asmc_exe" "ASMC: test"
check "nasm_exe" "NASM: test"
check "nasmc_exe" "NASMC: test"
check "c_exe" "C: test"
check "cpp_exe" "CPP: test"
check "all_exe" "
ASM: test
C: test
CPP: test"

exit $ret
