#!/bin/bash
if [ $# == 0 ]; then
    if ! [ -f build.inc ]; then
        echo "Running ./configure..."
        ./configure
        if ! [ -f build.inc ]; then
            echo "Could not create build.inc, obviously."
            exit 1
        fi
    fi

    exec bash build.inc
fi

if [ "$1" == "clean" ]; then
    make -s clean
else
    echo "Building tools..."
    make -s -r -C tools || exit 1
    echo "Building GBConDS..."
    make -s -r "CFLAGS=$CFLAGS" "ASFLAGS=$ASFLAGS" "MY_CC=$MY_CC" "MY_AS=$MY_AS" "MY_LINK=$MY_LINK" "OCP=$OCP" -C src || exit 1
    echo "Creating DS image..."
    cd tools && ./create_image.sh ../src/arm7elf ../src/arm9elf ../gbconds.nds
fi
