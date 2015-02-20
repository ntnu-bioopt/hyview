//=======================================================================================================
// Copyright 2015 Asgeir Bjorgan, Lise Lyngsnes Randeberg, Norwegian University of Science and Technology
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
// http://opensource.org/licenses/MIT)
//=======================================================================================================


#ifndef IMAGEVIEWER_H_DEFINED
#define IMAGEVIEWER_H_DEFINED

#include <QWidget>

#include <string>


class QLabel;

//Qt widget for displaying a hyperspectral datacube, using a QImage and a scrollbar for choosing band to display. 
class ImageViewer : public QWidget{
	Q_OBJECT
	public:
		ImageViewer(float *data, int lines, int samples, int bands, QWidget *parent = NULL); //assumes BIL-interleaved hyperspectral datacube in *data
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

		QImage currImage; //currently displayed image
		uchar *currImgData; //image data of currently displayed image
		QLabel *imageLabel;
		
		//scaling factors of image when widget is physically resized
		float widthScale;
		float heightScale;
	protected:
		void paintEvent(QPaintEvent *evt);
};

#endif
