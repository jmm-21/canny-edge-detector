/***************************************************************************
 *   Copyright (C) 2023 by pilar                                           *
 *   pilarb@unex.es                                                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#ifndef PRACAOC_H
#define PRACAOC_H

#include <ui_mainForm.h>
#include <QtCore>
#include <QPainter>
#include <QtWidgets/QFileDialog>
#include <QMouseEvent>
#include <iostream>
#include "imageprocess.h"


const int Gaus1x1[] = {1};

const int Gaus3x3[] = { 1, 2, 1,
                           2, 4, 2,   // * 1/16
                           1, 2, 1};

const int Gaus5x5[] = { 	2,  4,  5,  4, 2,
                            4,  9, 12,  9, 4,
                            5, 12, 15, 12, 5, // * 1/159
                            4,  9, 12,  9, 4,
                            2,  4,  5,  4, 2 };


using namespace Ui;


class pracAOC:public QWidget
{
    Q_OBJECT

public:
    pracAOC();
    ~pracAOC();
		
		

private:

		QMainForm * mainWin;
        QImage * qimgOrig, * qimgDest, qimgFromFile;
        uchar * imgO, * imgD;
        uchar tLUT[256];

		QVector<QRgb> ctable;

        QTimer timer;

        unsigned int IMAGEW, IMAGEH, WIDTH, HEIGHT;

        // CANNY
        int directions[8][2] = {
            {-1, 0}, {-1, 1}, {0, 1}, {1, 1},
            {1, 0}, {1, -1}, {0, -1}, {-1, -1}
        };

        Point contours[1000][5000];
        bool closedContour[1000];
        int nContours;

        Point polygons[1000][5000];

        Gradient* gradients;
        void blur(unsigned char * input, unsigned char * output);
        void computeGradient(unsigned char* input, Gradient* gradients);
        void nonMaxSuppression(Gradient* gradients, unsigned char* output);
        void doubleThreshold(unsigned char* input, unsigned char* output, unsigned char lowThreshold, unsigned char highThreshold);
        void canny_and_contours(unsigned char * input, unsigned char * output);
        void findContours(unsigned char * image, unsigned char *visited, Point contours[1000][5000], int & nContours);
        int followContour(unsigned char * image, unsigned char * visited, Point ini_point, Point contour[5000], int WIDTH, int HEIGHT);
        int createPolygon(Point contour[5000], bool isClosed, int nPoints, Point polygon[5000], float distance);

protected:
		void closeEvent(QCloseEvent *event);
        void resizeEvent(QResizeEvent *event);

public slots:

		void paintEvent(QPaintEvent *);
		void cargar();
        void process();
};




#endif
