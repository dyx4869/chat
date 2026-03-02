#!bin/bash

cd ./build
cmake ..
make
cd ..
rm -rf ./build
mkdir build
