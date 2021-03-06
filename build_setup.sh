#!/bin/bash

# This script contains the necessary installation and setup commands to prepare a new machine
# for compiling/developing the Asteria software.
#
# Development baseline:
# 1) clean installation of Lubuntu 16.04 LTS
# 2) GCC version 4.9
# 3) Qt version 5.5
#

# Build environment:
sudo apt-get install make
sudo apt-get install build-essential
sudo apt-get install git

# Ensure version 4.9 or later of the GCC/G++ compilers are present.
# Versions 4.8 and earlier lack regex support that is required by the project.
# NOTE: this may be installed by default in Lubuntu 16.04
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt-get update
sudo apt-get install g++-4.9
# Put symlinks in place:
sudo rm /usr/bin/gcc
sudo ln -s /usr/bin/gcc-4.9 /usr/bin/gcc
sudo rm /usr/bin/g++
sudo ln -s /usr/bin/g++-4.9 /usr/bin/g++

# AV packages:
sudo apt-get install libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev
sudo apt-get install libghc-gstreamer-dev
sudo apt-get install libgl1-mesa-dev

# QT packages [which QT version is the development baseline?]:
sudo apt-get install qt5-default
sudo apt-get install qtcreator
sudo apt-get install qtmultimedia5-dev
sudo apt-get install libqt5multimedia5-plugins

# Doxygen for code documentation
sudo apt-get install doxygen

# Other necessary libraries:
# GLUT for various OpenGL utilities:
sudo apt-get install freeglut3 freeglut3-dev
# JPEG libs for decompressing JPEG images from webcams
sudo apt-get install libjpeg-dev
# FTGL and Freetype for improved text rendering in OpenGL (this may no longer be used):
sudo apt-get install ftgl-dev libfreetype6-dev libfreetype6

# Gnuplot is used for plotting
sudo apt-get install gnuplot

# Eigen3 library is used for linear algebra
# Eigen headers are installed at /usr/include/eigen3/
# Include in Qt project by adding this to the .pro file:
# INCLUDEPATH += /usr/include/eigen3/
sudo apt-get install libeigen3-dev
sudo apt-get install libeigen3-doc

# Use the Boost C++ libraries for certain things (e.g. serialisation)
sudo apt-get install libboost-all-dev
# NOTE: Either install using the command above, or download the packages and unpack manually using:
# $ cd <boost root>
# $ ./bootstrap
# $ .b2 install
# Then, add the following line to the .pro file (boost unpacked in /opt/boost/):
# INCLUDEPATH += /opt/boost/boost_1_65_1/

# Other useful stuff:
sudo apt-get install v4l-utils

# In order to keep the system clock synchronized with UTC we need to turn on Network Time Protocol synchronization.
# After running the following command, the status can be checked by executing $ timedatectl status
# and checking for 'NTP enabled: yes'
sudo timedatectl set-ntp on

# To build Asteria it is simplest to use QtCreator. Once this has been configured for the developent machine,
# then it's simply a case of opening up the project and building it within the QtCreator environment. That
# produces the executable that can then be launched from the command line.

# To execute the application from the command line:
./Asteria

# Attempts at building directly from the command line (couldn't get it to work):
#git clone https://github.com/NickRowell/asteria
#cd asteria
#mkdir build
#cd build
#qmake Asteria.pro -spec linux-g++ CONFIG+=debug CONFIG+=qml_debug
#make
#make clean


