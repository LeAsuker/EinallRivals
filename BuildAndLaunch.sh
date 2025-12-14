if [ "$#" -ne 1 ] || [ "$1" != "linux" ] && [ "$1" != "macos" ]; then
    echo "USAGE: ./BuildAndLaunch.sh <linux/macos>"
    exit 1
fi

if [ "$1" = "linux" ]; then
    cd build
    ./premake5 gmake
    cd ..
    make
    cd ./bin/Debug
    ./EinallRivals
    exit 0
fi

if [ "$1" = "macos" ]; then
    cd build
    ./premake5.osx gmake
    cd ..
    make
    cd ./bin/Debug
    ./EinallRivals
    exit 0
fi

