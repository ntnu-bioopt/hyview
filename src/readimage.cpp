//=======================================================================================================
// Copyright 2015 Asgeir Bjorgan, Lise Lyngsnes Randeberg, Norwegian University of Science and Technology
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
// http://opensource.org/licenses/MIT)
//=======================================================================================================

#include <readimage.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
using namespace std;

const int MAX_CHAR = 512;
const int MAX_FILE_SIZE = 4000;

int getMatch(char *string, regmatch_t *matchArray, int matchNum, char **match);

//extract specified property value from header text as a char array
char* getValue(char *hdrText, const char *property);

//return list of wavelengths. Input: char array containing characters {wlen1, wlen2, wlen3, ...}
vector<float> getWavelengths(int bands, char* wavelengthStr);

char *getBasename(char *filename);

void hyperspectral_read_header(char *filename, HyspexHeader *header){
	//find base filename
	char *baseName = getBasename(filename);

	//open and read header file
	//append .hdr to filename
	char *hdrName = (char*)malloc(sizeof(char)*MAX_CHAR);
	strcpy(hdrName, baseName);
	strcat(hdrName, ".hdr");

	FILE *fp = fopen(hdrName, "rt");
	if (fp == NULL){
		fprintf(stderr, "Could not find header file: %s, exiting\n", hdrName);
		exit(1);
	}
	char hdrText[MAX_FILE_SIZE] = "";
	int sizeRead = 1;
	int offset = 0;
	while (sizeRead){
		sizeRead = fread(hdrText + offset, sizeof(char), MAX_CHAR, fp);
		offset += sizeRead/sizeof(char);

	}
	fclose(fp);

	//extract properties from header file text
	char *samples = getValue(hdrText, "samples");
	char *bands = getValue(hdrText, "bands");
	char *lines = getValue(hdrText, "lines");
	char *wavelengths = getValue(hdrText, "wavelength");
	char *hdrOffset = getValue(hdrText, "header offset");
	char *interleave = getValue(hdrText, "interleave");
	char *datatype = getValue(hdrText, "data type");

	//convert strings to values
	header->bands = strtod(bands, NULL);
	header->lines = strtod(lines, NULL);
	header->samples = strtod(samples, NULL);
	header->offset = strtod(hdrOffset, NULL);
	header->wlens = getWavelengths(header->bands, wavelengths);
	header->datatype = strtod(datatype, NULL);


	if (strcmp(interleave, "bil")){
		fprintf(stderr, "Interleave not supported by this file reader: %s, exiting\n", interleave);
		exit(1);
	}

	//cleanup
	free(baseName);
	free(hdrName);

	free(wavelengths);
	free(samples);
	free(bands);
	free(lines);
	free(hdrOffset);
	free(interleave);
	free(datatype);

	//recap
	fprintf(stderr, "Extracted: lines=%d, samples=%d, bands=%d, offset=%d\n", header->lines, header->samples, header->bands, header->offset);
	fprintf(stderr, "Wavelengths: ");
	for (int i=0; i < header->wlens.size(); i++){
		fprintf(stderr, "%f ", header->wlens[i]);
	}
	fprintf(stderr, "\n");
}

void hyperspectral_read_image(char *filename, HyspexHeader *header, ImageSubset subset, float *data){
	//find number of bytes for contained element
	size_t elementBytes = 0;
	if (header->datatype == 4){
		elementBytes = sizeof(float);
	} else if (header->datatype == 12){
		elementBytes = sizeof(uint16_t);
	} else {
		fprintf(stderr, "Datatype not supported.\n");
		exit(1);
	}

	FILE *fp = fopen(filename, "rb");
	if (fp == NULL){
		fprintf(stderr, "Could not open file.\n");
		exit(1);
	}
	
	//skip header and lines we do not want
	size_t skipBytes = subset.startLine*header->samples*header->bands*elementBytes + header->offset;
	fseek(fp, skipBytes, SEEK_SET);

	int numLinesToRead = subset.endLine - subset.startLine;


	//read in line by line
	for (int i=0; i < numLinesToRead; i++){
		char *line = (char*)malloc(elementBytes*header->bands*header->samples);
		int sizeRead = fread(line, elementBytes, header->bands*header->samples, fp);
		if (sizeRead == 0){
			fprintf(stderr, "Something went extremely wrong in the file reading: %d, %d\n", ferror(fp), feof(fp));
			exit(1);
		}

		//convert to float, copy to total array
		for (int j=subset.startSamp; j < subset.endSamp; j++){
			for (int k=0; k < header->bands; k++){
				float val;
				int position = k*header->samples + j;
				if (header->datatype == 4){
					val = ((float*)line)[position];
				} else if (header->datatype == 12){
					val = (((uint16_t*)line)[position])*1.0f;
				}
				data[i*header->bands*(subset.endSamp - subset.startSamp) + k*(subset.endSamp - subset.startSamp) + j-subset.startSamp] = val;
			}
		}
		free(line);



	}
	fclose(fp);
	
}

int getMatch(char *string, regmatch_t *matchArray, int matchNum, char **match){
	int start = matchArray[matchNum].rm_so;
	int end = matchArray[matchNum].rm_eo;
	*match = (char*)malloc(sizeof(char)*(end-start+1));
	
	strncpy(*match, string+start, end-start);
	(*match)[end-start] = '\0';
}

char* getValue(char *hdrText, const char *property){
	//find the input property using regex
	regex_t propertyMatch;
	int numMatch = 2;
	regmatch_t *matchArray = (regmatch_t*)malloc(sizeof(regmatch_t)*numMatch);

	char regexExpr[MAX_CHAR] = "";
	strcat(regexExpr, property);
	strcat(regexExpr, "\\s*=\\s*([{|}|0-9|,| |.|a-z]+)"); //property followed by = and a set of number, commas, spaces or {}s

	int retcode = regcomp(&propertyMatch, regexExpr, REG_EXTENDED | REG_NEWLINE);
	int match = regexec(&propertyMatch, hdrText, numMatch, matchArray, 0);
	if (match != 0){
		fprintf(stderr, "Could not find property in header file: %s\nExiting\n", property);
		exit(1);
	}
	char *retVal;
	getMatch(hdrText, matchArray, 1, &retVal);


	//cleanup
	regfree(&propertyMatch);
	free(matchArray);

	return retVal;
	
}

char *getBasename(char *filename){
	regex_t filenameMatch;
	int numMatch = 2;
	regmatch_t *matchArray = (regmatch_t*)malloc(sizeof(regmatch_t)*numMatch);
	int retcode = regcomp(&filenameMatch, "(.*)[.].*$", REG_EXTENDED);
	int match = regexec(&filenameMatch, filename, numMatch, matchArray, 0);
	char *baseName;
	getMatch(filename, matchArray, 1, &baseName);
	regfree(&filenameMatch);
	free(matchArray);
	return baseName;
}

vector<float> getWavelengths(int bands, char* wavelengthStr){
	vector<float> retWlens;

	//prepare regex
	regex_t numberMatch;
	int numMatch = 2;
	regmatch_t *matchArray = (regmatch_t*)malloc(sizeof(regmatch_t)*numMatch);
	char regexExpr[MAX_CHAR] = "([0-9|.]+)[,| |}]*";
	int retcode = regcomp(&numberMatch, regexExpr, REG_EXTENDED);
	
	//find start of number sequence
	char *currStart = strchr(wavelengthStr, '{') + 1;
	
	//go through all bands
	bool useStandardValues = false;
	for (int i=0; i < bands; i++){
		//extract wavelength
		if (regexec(&numberMatch, currStart, numMatch, matchArray, 0)){
			fprintf(stderr, "Could not extract wavelengths. Assuming standard values 1, 2, 3, ... .\n");
			useStandardValues = true;
			break;
		}
		
		char *match;
		getMatch(currStart, matchArray, 1, &match);
		retWlens.push_back(strtod(match, NULL));
		free(match);

		//move to next
		currStart = currStart + matchArray[0].rm_eo;
	}

	if (useStandardValues){
		retWlens.clear();
		for (int i=0; i < bands; i++){
			retWlens.push_back(i);
		}
	}


	regfree(&numberMatch);
	free(matchArray);
	return retWlens;
}



#include <fstream>
#include <iostream>
#include <math.h>
#include <cstring>
#include <sstream>
using namespace std;

void hyperspectral_write_header(const char *filename, int numBands, int numPixels, int numLines, std::vector<float> wlens){
	//write image header
	ostringstream hdrFname;
	hdrFname << filename << ".hdr";
	ofstream hdrOut(hdrFname.str().c_str());
	hdrOut << "ENVI" << endl;
	hdrOut << "samples = " << numPixels << endl;
	hdrOut << "lines = " << numLines << endl;
	hdrOut << "bands = " << numBands << endl;
	hdrOut << "header offset = 0" << endl;
	hdrOut << "file type = ENVI Standard" << endl;
	hdrOut << "data type = 4" << endl;
	hdrOut << "interleave = bil" << endl;
	hdrOut << "default bands = {55,41,12}" << endl;
	hdrOut << "byte order = 0" << endl;
	hdrOut << "wavelength = {";
	for (int i=0; i < wlens.size(); i++){
		hdrOut << wlens[i] << " ";
	}
	hdrOut << "}" << endl;
	hdrOut.close();
}

void hyperspectral_write_image(const char *filename, int numBands, int numPixels, int numLines, float *data){
	//prepare image file
	ostringstream imgFname;
	imgFname << filename << ".img";
	ofstream *hyspexOut = new ofstream(imgFname.str().c_str(),ios::out | ios::binary);
	
	//write image
	for (int i=0; i < numLines; i++){
		float *write_data = data + i*numBands*numPixels;
		hyspexOut->write((char*)(write_data), sizeof(float)*numBands*numPixels);
	}

	hyspexOut->close();
	delete hyspexOut;
}


