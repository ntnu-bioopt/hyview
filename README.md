hyview
======

Simple hyperspectral image viewer. Can be used for quickly visualizing the
images contained in each band of the hyperspectral data cube (band scrollbar available).

The hyperspectral image reader currently supports only images in the ENVI
format, but the image widget (src/imageViewer.cpp) can display any data as long
as it is BIL-interleaved and contained in a continuous float array. 

./hyview [imagefile]. See also ./hyview --help.

Compiled using cmake:

mkdir build
cd build 
cmake ..
make
(make install)


Requirements:
 - GNU Regex (for file reading)
 - Qt 4
 - CMake

Optional requirements:
 - libqwt 5

Compiling with qwt will make it possible to display individual pixel spectra in a separate widget. Hold CTRL while clicking on the image
to compare multiple pixel spectra. 

The default option is to compile /with/ qwt. Disable this by editing CMakeLists.txt manually and comment out the lines
between "QWT start" and "Qwt end" (quickfix).
