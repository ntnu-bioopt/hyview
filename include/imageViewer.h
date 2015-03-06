//=======================================================================================================
// Copyright 2015 Asgeir Bjorgan, Lise Lyngsnes Randeberg, Norwegian University of Science and Technology
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
// http://opensource.org/licenses/MIT)
//=======================================================================================================


#ifndef IMAGEVIEWER_H_DEFINED
#define IMAGEVIEWER_H_DEFINED

#include <QWidget>
#include <QVector>
#include <string>
#include <vector>

class QLabel;

//used in SpectrumDisplayer for controlling whether to keep or delete previous spectra in the plot when adding a new one
enum KeepMode{KEEP_PREVIOUS_SPECTRA, DELETE_PREVIOUS_SPECTRA};

//Qt widget for displaying a hyperspectral datacube, using a QImage and a scrollbar for choosing band to display. 
class ImageViewer : public QWidget{
	Q_OBJECT
	public:
		ImageViewer(float *data, int lines, int samples, int bands, std::vector<float> wlens, QWidget *parent = NULL); //assumes BIL-interleaved hyperspectral datacube in *data
		void getSpectrum(int x, int y, float *spec); //copy spectrum at pixel (y,x) in provided float array
	public slots:	
		void updateImage(int band); //update displayed image to provided band. Uses dynamic ranges. 
		void saveImage(int band, std::string bandimagename); //save the image at provided band to file (format specified by the filename)
	private:
		//datacube information
		float *data;
		int lines;
		int samples;
		int bands;
		std::vector<float> wlens;

		QImage currImage; //currently displayed image
		uchar *currImgData; //image data of currently displayed image
		QLabel *imageLabel;
		
		//scaling factors of image when widget is physically resized
		float widthScale;
		float heightScale;
	protected:
		void paintEvent(QPaintEvent *evt);
		bool eventFilter(QObject *object, QEvent *event); //mouse button clicks on image
	signals:
		void clickedPixel(int line, int sample, QVector<double> wlens, QVector<double> spectrum, KeepMode keepMode); //emit spectrum residing in clicked pixel
		float newBand(float wavelength); //use for signalling current wavelength to e.g. SpectrumDisplayer

};


#ifdef WITH_QWT
class QwtPlot;
class QwtPlotCurve;
class QwtPlotMarker;

//receive spectra and display 'em
class SpectrumDisplayer : public QWidget{
	Q_OBJECT
	public:
		SpectrumDisplayer(QWidget *parent = NULL);
	public slots:
		void displaySpectrum(int y, int x, QVector<double> wlens, QVector<double> intensity, KeepMode keepBehavior);
		void setVerticalLine(float wavelength); //set a vertical line at the specified wavelength
	private:
		QwtPlot *plot;
		QVector<QwtPlotCurve*> curves; //current displayed data curves
		QwtPlotMarker *vertLine; //vertical line for indicating current wavelength in the imageviewer
		int colorCtr; //for choosing between colors to use in the displayed spectrum
};
#endif

#endif
