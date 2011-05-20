#!/bin/bash

write32()
{
    let d=0
    while [ $d -lt 32 ]; do
        let v=\($2\>\>$d\)\&255
        printf "\x$(printf %x $v)" >> $1
        let d=$d+8
    done
}

write16()
{
    let d=0
    while [ $d -lt 16 ]; do
        let v=\($2\>\>$d\)\&255
        printf "\x$(printf %x $v)" >> $1
        let d=$d+8
    done
}

fill()
{
    let i=$2
    let upper=$3
    while [ $i -lt $upper ]; do
        printf "\x0" >> $1
        let i=$i+1
    done
}

append()
{
    cat $1 $2 > cache
    mv cache $1
}

if [ $# != 3 ]; then
    echo "Benutzung: create_image.sh <arm7elf> <arm9elf> <rom>"
    exit 1
fi

let arm7entry=$(readelf -h $1 | grep "Entry point address" | tail -c 10 - | tr -d ' ')
let arm9entry=$(readelf -h $2 | grep "Entry point address" | tail -c 10 - | tr -d ' ')
let arm7addr=$(readelf -l $1 | grep LOAD | head -c 36 - | tail -c 10 - | tr -d ' ')
let arm9addr=$(readelf -l $2 | grep LOAD | head -c 36 - | tail -c 10 - | tr -d ' ')

arm-linux-objcopy -O binary $1 arm7.bin || exit 1
arm-linux-objcopy -O binary $2 arm9.bin || exit 1

let arm7size=$(stat -c %s arm7.bin)
let arm9size=$(stat -c %s arm9.bin)

printf "%-12s" GBCONDS > $3
fill $3 0x0C 0x20

write32 $3 0x8000
write32 $3 $arm9entry
write32 $3 $arm9addr
write32 $3 $arm9size

let arm7offs=0x8000+\(\($arm9size+4095\)/4096\)\*4096
write32 $3 $arm7offs
write32 $3 $arm7entry
write32 $3 $arm7addr
write32 $3 $arm7size

fill $3 0x40 0x80
let rom_size=$arm7offs+$arm7size
write32 $3 $rom_size
write32 $3 0x8000
fill $3 0x88 0xC0

append $3 ninlog.bin
write16 $3 $(./crc16 $3)

fill $3 0x160 0x8000

append $3 arm9.bin
fill $3 0x8000+$arm9size $arm7offs
append $3 arm7.bin

rm -f arm7.bin arm9.bin

