#!/bin/bash

# Run doxygen
doxygen doxygen.config

# Compile latex source
cd doc/latex/
pdflatex refman.tex
mv refman.pdf ../
cd ../../


