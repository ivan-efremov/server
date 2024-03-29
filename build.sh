#!/bin/sh

case $1 in
        "usage" )
                echo "Usage:"
                echo "        ./build                - Build by default mode."
                echo "        ./build debug          - Build Debug mode."
                echo "        ./build release        - Build Release mode."
                echo "        ./build clean          - Clean all objects files."
                ;;
        "debug" )
                mkdir -p bin
                cd bin
                cmake -DCMAKE_BUILD_TYPE=Debug ../ && make
                cd ..
                ;;
        "release" )
                mkdir -p bin
                cd obj
                cmake -DCMAKE_BUILD_TYPE=Release ../ && make
                cd ..
                ;;
        "clean" )
                rm -Rf bin
                ;;
        * )
                if [ ! -d "bin" ]; then
                    $0 debug
                else
                    cd bin
                    make
                    cd ..
                fi
                ;;
esac

echo "Build end"
exit 0

