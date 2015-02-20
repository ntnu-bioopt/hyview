hyview
======

Simple hyperspectral image viewer. Can be used for quickly visualizing the
images contained in each band of the hyperspectral data cube (band scrollbar available).

The hyperspectral image reader currently supports only images in the ENVI
format, but the image widget (src/imageViewer.cpp) can display any data as long
as it is BIL-interleaved and contained in a continuous float array. 

Requirements:
 - GNU Regex
 - Qt 4
 - CMake

Compiled using cmake:

1. mkdir build
2. cd build 
3. cmake ..
4. make
5. (make install)

./hyview [imagefile]. See also ./hyview --help.
