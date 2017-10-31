#!/bin/bash

# Run doxygen
doxygen doxygen.config

# Compile latex source
cd doc/latex/
lualatex refman.tex
lualatex refman.tex
mv refman.pdf ../
cd ../../


