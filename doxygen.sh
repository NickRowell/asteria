#!/bin/bash

# Run doxygen
doxygen doxygen.config

# Compile latex source
cd doc/doxy/latex/
lualatex refman.tex
lualatex refman.tex
mv refman.pdf ../
cd ../../../


