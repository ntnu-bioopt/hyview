//=======================================================================================================
// Copyright 2015 Asgeir Bjorgan, Lise Lyngsnes Randeberg, Norwegian University of Science and Technology
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
// http://opensource.org/licenses/MIT)
//=======================================================================================================


#include <QApplication>
#include <string>
#include "getopt.h"
#include <sstream>
#include "imageViewer.h"
#include "readimage.h"
#include <vector>
#include <iostream>
using namespace std;

void showHelp(){
	cerr << "Usage: hyview [OPTION]... [FILE]" << endl
		<< "Hyperspectral image viewer (BIL-interleaved ENVI image assumed)." << endl << endl
		<< "--help\t\t\t Show help" << endl << endl
		<< "Image subset arguments:" << endl
		<< "--startpix=START_PIXEL \t Start pixel (chosen pixel for showpixel)" << endl
		<< "--endpix=END_PIXEL \t End pixel" << endl
		<< "--startline=START_LINE \t Start line (chosen line for showpixel)" << endl
		<< "--endline=END_LINE \t End line" << endl;
}
void createOptions(option **options){
	int numOptions = 6;
	*options = new option[numOptions];

	(*options)[0].name = "startpix";
	(*options)[0].has_arg = required_argument;
	(*options)[0].flag = NULL;
	(*options)[0].val = 0;

	(*options)[1].name = "endpix";
	(*options)[1].has_arg = required_argument;
	(*options)[1].flag = NULL;
	(*options)[1].val = 1;
	
	(*options)[2].name = "help";
	(*options)[2].has_arg = no_argument;
	(*options)[2].flag = NULL;
	(*options)[2].val = 2;
	
	(*options)[3].name = "startline";
	(*options)[3].has_arg = required_argument;
	(*options)[3].flag = NULL;
	(*options)[3].val = 3;
	
	(*options)[4].name = "endline";
	(*options)[4].has_arg = required_argument;
	(*options)[4].flag = NULL;
	(*options)[4].val = 4;
	
	(*options)[5].name = 0;
	(*options)[5].has_arg = 0;
	(*options)[5].flag = 0;
	(*options)[5].val = 0;

}

int main(int argc, char *argv[]){
	//process options
	char shortopts[] = "";
	option *longopts;
	createOptions(&longopts);

	int startline = 0;
	int endline = 0;
	int startpix = 0;
	int endpix = 0;
	int band = 0;

	int index;
	
	while (true){
		int flag = getopt_long(argc, argv, shortopts, longopts, &index);	
		switch (flag){
			case 0: 
				startpix = strtod(optarg, NULL);
			break;

			case 1: 
				endpix = strtod(optarg, NULL);
			break;
			
			case 2: //help
				showHelp();
				exit(0);
			break;

			case 3: 
				startline = strtod(optarg, NULL);
			break;

			case 4: 
				endline = strtod(optarg, NULL);
			break;
		}
		if (flag == -1){
			break;
		}
	}
	char *filename = argv[optind];
	if (filename == NULL){
		cerr << "Filename missing." << endl;
		exit(1);
	}
	
	//read hyperspectral image header
	size_t offset;
	vector<float> wlens;
	HyspexHeader header;
	hyperspectral_read_header(filename, &header);

	//configure image subsets
	if (!startline){
		startline = 0;
	}
	if (!endline){
		endline = header.lines;
	}
	if (!startpix){
		startpix = 0;
	}
	if (!endpix){
		endpix = header.samples;
	}
	
	int newLines = endline - startline;
	int newSamples = endpix - startpix;

	ImageSubset subset;
	subset.startSamp = startpix;
	subset.endSamp = endpix;
	subset.startLine = startline;
	subset.endLine = endline;
	
	//read hyperspectral image
	float *data = new float[newLines*newSamples*header.bands];
	hyperspectral_read_image(filename, &header, subset, data);
	wlens = header.wlens;

	//start Qt app, display imageViewer widget
	QApplication app(argc, argv);
	ImageViewer viewer(data, newLines, newSamples, header.bands, wlens);
	viewer.show();
	
	return app.exec();
}	
