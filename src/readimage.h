//=======================================================================================================
// Copyright 2015 Asgeir Bjorgan, Lise Lyngsnes Randeberg, Norwegian University of Science and Technology
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
// http://opensource.org/licenses/MIT)
//=======================================================================================================

#ifndef READIMAGE_H_DEFINED
#define READIMAGE_H_DEFINED
#include <vector>

typedef struct {
	int samples;
	int bands;
	int lines;
	int offset;
	std::vector<float> wlens;
	int datatype;
} HyspexHeader;

typedef struct {
	int startSamp;
	int endSamp;
	int startLine;
	int endLine;
} ImageSubset;
	
void hyperspectral_read_header(char *filename, HyspexHeader *header);
void hyperspectral_read_image(char *filename, HyspexHeader *header, ImageSubset subset, float *data);


void hyperspectral_write_header(const char *filename, int bands, int samples, int lines, std::vector<float> wlens);
void hyperspectral_write_image(const char *filename, int bands, int samples, int lines, float *data);


#endif
