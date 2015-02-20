//=======================================================================================================
// Copyright 2015 Asgeir Bjorgan, Lise Lyngsnes Randeberg, Norwegian University of Science and Technology
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
// http://opensource.org/licenses/MIT)
//=======================================================================================================

#include "imageViewer.h"
#include <cmath>
#include <QGridLayout>
#include <QLabel>
#include <QScrollBar>
#include <QScrollArea>
#include <QEvent>
#include <QPainter>
#include <QMouseEvent>
#include <iostream>
using namespace std;

ImageViewer::ImageViewer(float *data, int lines, int samples, int bands, QWidget *parent) : data(data), lines(lines), samples(samples), bands(bands), QWidget(parent){
	currImgData = new uchar[samples*lines*3];
	imageLabel = new QLabel;

	//scrollbar for choosing band
	QScrollBar *bandChooser = new QScrollBar;
	bandChooser->setMaximum(bands-1);
	connect(bandChooser, SIGNAL(valueChanged(int)), SLOT(updateImage(int)));
	
	//layout
	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(bandChooser, 0, 1);
	updateImage(0);
	QScrollArea *area = new QScrollArea;
	layout->addWidget(area, 0, 0);
	
	imageLabel->setBackgroundRole(QPalette::Base);
	imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	area->setWidget(imageLabel);

	imageLabel->setScaledContents(true);

	bandChooser->setValue(0);
}
		
		

void ImageViewer::paintEvent(QPaintEvent *evt){
	Q_UNUSED(evt);
	
	//scale QImage to current widget size
	QImage resImage;
	int imHeight = currImage.height();
	int imWidth = currImage.width();
	resImage = currImage.scaledToWidth(size().width()-30);
	
	//update scaling factors
	int newHeight = resImage.height();
	int newWidth = resImage.width();
	widthScale = imWidth*1.0f/(newWidth*1.0f);
	heightScale = imHeight*1.0f/(newHeight*1.0f);
	
	//update label with current pixmap
	this->imageLabel->setPixmap(QPixmap::fromImage(resImage));
	this->imageLabel->adjustSize();
	
}

void ImageViewer::getSpectrum(int x, int y, float *spec){
	for (int i=0; i < bands; i++){
		spec[i] = data[y*samples*bands + i*samples + x];
	}
}

void ImageViewer::updateImage(int band){
	int c = 0;
	float maxval = -10000;
	float *imgDataTmp = new float[lines*samples*3];
	float std = 0, mean = 0;
	long n = 0;
	
	//convert to greyscale array
	for (int i=0; i < lines; i++){
		for (int j=0; j < samples; j++){
			float val = data[i*samples*bands + band*samples + j];
			n++;
			float delta = val - mean;
			mean = mean + delta/(1.0f*n);
			std = std + delta*(val - mean);

			imgDataTmp[c++] = val;
			imgDataTmp[c++] = val;
			imgDataTmp[c++] = val;
		}
	}
	std = sqrt(std/(n-1));
	
	float max = mean + 2*std;
	float min = mean - 2*std;
	
	//convert to positive values, divide by largest value
	for (int i=0; i < lines*samples*3; i++){
		if (imgDataTmp[i] > (mean + 2*std)){
			imgDataTmp[i] = max;
		}
		if (imgDataTmp[i] < (mean - 2*std)){
			imgDataTmp[i] = min;
		}
		currImgData[i] = (uchar)((imgDataTmp[i] - min)/(max - min)*255);
	}


	currImage = QImage(currImgData, samples, lines, 3*samples, QImage::Format_RGB888);
	update();

	delete [] imgDataTmp;
}
		
void ImageViewer::saveImage(int band, string bandimagename){
	updateImage(band);
	currImage.save(QString::fromStdString(bandimagename));
}
