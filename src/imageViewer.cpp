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

/////////////////
// ImageViewer //
/////////////////

ImageViewer::ImageViewer(float *data, int lines, int samples, int bands, vector<float> wlens, QWidget *parent) : data(data), lines(lines), samples(samples), bands(bands), wlens(wlens), QWidget(parent){
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
	imageLabel->setMouseTracking(true);
	imageLabel->installEventFilter(this);

	bandChooser->setValue(0);

	//plot displayer, if compiled with support for this
	#ifdef WITH_QWT
	SpectrumDisplayer *spectrumDisplayer = new SpectrumDisplayer;
	connect(this, SIGNAL(clickedPixel(int, int, QVector<double>, QVector<double>, KeepMode)), spectrumDisplayer, SLOT(displaySpectrum(int, int, QVector<double>, QVector<double>, KeepMode)));
	connect(this, SIGNAL(newBand(float)), spectrumDisplayer, SLOT(setVerticalLine(float)));
	spectrumDisplayer->show();
	#endif
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

	//signal that the wavelength has changed
	if (band < wlens.size()){
		emit newBand(wlens[band]);
	}
}
		
void ImageViewer::saveImage(int band, string bandimagename){
	updateImage(band);
	currImage.save(QString::fromStdString(bandimagename));
}

bool ImageViewer::eventFilter(QObject *object, QEvent *event){
	Q_UNUSED(object);

	//update displayed spectrum on mouse button press
	if ((event->type() == QEvent::MouseButtonPress)){
		QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
		KeepMode keepMode = DELETE_PREVIOUS_SPECTRA;
		
		if (mouseEvent->modifiers() == Qt::ControlModifier){
			//ctrl was pressed. Keep previous spectra
			keepMode = KEEP_PREVIOUS_SPECTRA;
		}

		//get spectrum in current position
		int pixel = widthScale*mouseEvent->x();
		int line = heightScale*mouseEvent->y();
		float *spectrum = new float[bands];
		this->getSpectrum(pixel, line, spectrum);

		//convert data to QVector and emit signal (to plot displayer, if compiled with support for this...)
		QVector<double> wlens_vec;
		QVector<double> spectrum_vec;
		for (int i=0; i < bands; i++){
			wlens_vec.push_back(wlens[i]);
			spectrum_vec.push_back(spectrum[i]);
		}

		emit clickedPixel(line, pixel, wlens_vec, spectrum_vec, keepMode);
		delete [] spectrum;
	} 
	return false;
}



////////////////////////
// Spectrum displayer //
////////////////////////



#ifdef WITH_QWT
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_legend.h>
#include <qwt_symbol.h>
#include <qwt_plot_marker.h>


SpectrumDisplayer::SpectrumDisplayer(QWidget *parent) : QWidget(parent){
	this->setWindowTitle("Spectra");
	
	plot = new QwtPlot(this);
	plot->setAxisTitle(QwtPlot::yLeft, "Pixel intensity");
	plot->setAxisTitle(QwtPlot::xBottom, "Wavelength (nm)");
	QwtLegend *legend = new QwtLegend;
	plot->insertLegend(legend, QwtPlot::RightLegend);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(plot, 0, 0);
	colorCtr = 0;

	//vertical line indicating current wavelength
	vertLine = new QwtPlotMarker;
	vertLine->setLineStyle(QwtPlotMarker::VLine);
	vertLine->attach(plot);
}
		
void SpectrumDisplayer::displaySpectrum(int y, int x, QVector<double> wlens, QVector<double> intensity, KeepMode keepBehavior){
	QwtPlotCurve *curve = new QwtPlotCurve("Line " + QString::number(y) + ", sample " + QString::number(x));
	curve->setData(wlens, intensity);
	curve->setPen(QColor::fromHsv(colorCtr, 255, 255));
	curve->attach(plot);
	colorCtr += 10;
	if (colorCtr > 255){
		colorCtr = 0;
	}

	//remove previous spectra
	if (keepBehavior == DELETE_PREVIOUS_SPECTRA){
		for (int i=0; i < curves.size(); i++){
			curves[i]->detach();
		}
		curves.clear();
		colorCtr = 0;
	}
	curves.push_back(curve);
	plot->replot();

}
		
void SpectrumDisplayer::setVerticalLine(float wavelength){
	vertLine->setXValue(wavelength);
	plot->replot();
}
#endif

