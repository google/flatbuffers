#!/bin/bash

sudo apt-get update -qq
sudo apt-get install libglu1-mesa-dev xorg-dev
wget http://www.biicode.com/downloads/latest/ubuntu64
mv ubuntu64 bii-ubuntu64.deb
(sudo dpkg -i bii-ubuntu64.deb) && sudo apt-get -f install
rm bii-ubuntu64.deb
wget https://s3.amazonaws.com/biibinaries/thirdparty/cmake-3.0.2-Linux-64.tar.gz
tar -xzf cmake-3.0.2-Linux-64.tar.gz
sudo cp -fR cmake-3.0.2-Linux-64/* /usr
rm -rf cmake-3.0.2-Linux-64
rm cmake-3.0.2-Linux-64.tar.gz

cmake --version
bii init -l && bii configure -DCMAKE_BUILD_TYPE=$1 && bii test