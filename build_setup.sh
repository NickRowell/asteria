#!/bin/bash

# This script contains the necessary installation and setup commands to prepare a new machine
# for compiling/developing the Asteria software.
#



# Install GLUT for basic text rendering in OpenGL
sudo apt-get install freeglut3 freeglut3-dev

# Install FTGL and Freetype for improved text rendering in OpenGL
sudo apt-get install ftgl-dev libfreetype6-dev libfreetype6
