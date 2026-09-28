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


#include "pracaoc.h"


pracAOC::pracAOC(): QWidget()
{
	mainWin=new QMainForm();
	mainWin->setupUi(this);


	ctable.resize(256);
	for(int i=0; i < 256; i++)
		ctable[i] = qRgb(i,i,i);

    IMAGEW = mainWin->frameOrig->width();
    IMAGEW = (IMAGEW/4)*4;
    IMAGEH = mainWin->frameOrig->height();
    WIDTH = IMAGEW;
    HEIGHT = IMAGEH;

    qimgOrig = new QImage(IMAGEW, IMAGEH, QImage::Format_Indexed8);
	qimgOrig->setColorTable(ctable);
    qimgOrig->setColorCount(256);
    imgO = qimgOrig->bits();

    qimgDest = new QImage(IMAGEW, IMAGEH, QImage::Format_Indexed8);
	qimgDest->setColorTable(ctable);
    qimgDest->setColorCount(256);
    imgD = qimgDest->bits();

    qimgFromFile=QImage(IMAGEW, IMAGEH, QImage::Format_Indexed8);
    qimgFromFile.setColorTable(ctable);
    qimgFromFile.setColorCount(256);
    qimgFromFile.fill(0);

    for(unsigned int i=0; i<IMAGEW*IMAGEH;i++)
    {
        imgO[i]=0;
        imgD[i]=0;
    }

    gradients = (Gradient*)malloc(sizeof(Gradient) * WIDTH * HEIGHT);
    nContours = 0;

    connect ( mainWin->pushButtonCargar, SIGNAL (clicked()), this, SLOT( cargar() ) );
    connect ( mainWin->pushButtonSalir, SIGNAL (clicked()), this, SLOT( close() ) );

    connect (&timer,SIGNAL(timeout()),this,SLOT(process()));

    this->resize(this->width(), this->height());
    timer.start(33);

}


pracAOC::~pracAOC()
{
}


void pracAOC::paintEvent(QPaintEvent *)
{
	QPainter painter(this);

    QPoint iniPointOrig = mainWin->frameOrig->mapTo(this, mainWin->frameOrig->rect().topLeft());
    QPoint iniPointDest = mainWin->frameDest->mapTo(this, mainWin->frameDest->rect().topLeft());

    painter.drawImage(iniPointOrig, *qimgOrig);
    painter.drawImage(iniPointDest, *qimgDest);

    if(mainWin->CBShowContours->isChecked())
    {
        for(int i = 0; i<nContours; i++)
        {
            Point p = contours[i][0];
            int pIndex = 1;

            painter.setPen(QPen(Qt::green,2));
            if((mainWin->ContourType->currentIndex()==0 && !closedContour[i]) or (mainWin->ContourType->currentIndex()==1 && closedContour[i]) or mainWin->ContourType->currentIndex()==2)
            {
                while(contours[i][pIndex].x>=0)
                {
                    Point np = contours[i][pIndex];
                    painter.drawLine(QPoint(p.x, p.y)+iniPointDest, QPoint(np.x, np.y)+iniPointDest);
                    p = np;
                    pIndex++;
                }
            }

        }
    }


    if(mainWin->CBShowPolygons->isChecked())
    {
        painter.setBrush(QBrush(Qt::red));
        for(int i = 0; i<nContours; i++)
        {
            if(polygons[i][0].x>=0)
            {
                Point p = polygons[i][0];
                int pIndex = 1;

                painter.setPen(QPen(Qt::green,2));
                if((mainWin->ContourType->currentIndex()==0 && !closedContour[i]) or (mainWin->ContourType->currentIndex()==1 && closedContour[i]) or mainWin->ContourType->currentIndex()==2)
                {
                    while(polygons[i][pIndex].x>=0)
                    {
                        Point np = polygons[i][pIndex];
                        painter.setPen(QPen(Qt::green,2));
                        painter.drawLine(QPoint(p.x, p.y)+iniPointDest, QPoint(np.x, np.y)+iniPointDest);
                        painter.setPen(QPen(Qt::red,2));
                        painter.drawEllipse(QPoint(p.x,p.y)+iniPointDest, 3, 3);
                        p = np;
                        pIndex++;
                    }

                }
            }

        }
    }

	painter.end();
}

void pracAOC::resizeEvent(QResizeEvent *event)
{
    std::ignore = event;

    IMAGEW = mainWin->frameOrig->width();
    IMAGEW = (IMAGEW/4)*4;
    IMAGEH = mainWin->frameOrig->height();
    WIDTH = IMAGEW;
    HEIGHT = IMAGEH;

    *qimgOrig = qimgFromFile.scaled(WIDTH, HEIGHT);
    imgO = qimgOrig->bits();
    *qimgDest = qimgDest->scaled(WIDTH, HEIGHT);
    imgD = qimgDest->bits();

    delete gradients;
    gradients = (Gradient*)malloc(sizeof(Gradient) * WIDTH * HEIGHT);

}

void pracAOC::cargar()
{
    disconnect(&timer,SIGNAL(timeout()),this,SLOT(process()));

    unsigned alignedWidth;
    QImage qimgRead(IMAGEW,IMAGEH,QImage::Format_Indexed8);
	
    QString fn = QFileDialog::getOpenFileName(this, "Seleccione un fichero",".", "Images (*.png *.xpm *.jpg)");
	if(qimgRead.load(fn))
	{
        qimgFromFile = qimgRead;
        qimgFromFile=qimgFromFile.convertToFormat(QImage::Format_Indexed8, ctable);

        qimgRead = qimgRead.scaled(WIDTH, HEIGHT);
		QImage qimgCRead(qimgRead.width(),qimgRead.height(),QImage::Format_Indexed8);

		qimgCRead=qimgRead.convertToFormat(QImage::Format_Indexed8, ctable);

        qimgFromFile = qimgCRead;

        alignedWidth = qimgCRead.width();

        for(unsigned y=0; y<IMAGEH && y<(unsigned) qimgCRead.height();y++)
            for(unsigned x=0; x<IMAGEW && x<(unsigned) qimgCRead.width(); x++)
            {
                imgO[y*IMAGEW+x]=(qimgCRead.bits())[(y*alignedWidth+x)];
            }

		update();
	}

    connect(&timer,SIGNAL(timeout()),this,SLOT(process()));
}



void pracAOC::process()
{
    canny_and_contours(imgO, imgD);

    update();
}


void pracAOC::closeEvent(QCloseEvent *event)
{
    std::ignore = event;

}


void pracAOC::canny_and_contours(unsigned char * input, unsigned char * output)
{
    unsigned char * output_binary = (unsigned char *) malloc(WIDTH*HEIGHT);
    unsigned char * output_blur   = (unsigned char *) malloc(WIDTH*HEIGHT);
    unsigned char * output_canny1 = (unsigned char *) malloc(WIDTH*HEIGHT);
    int           * output_gradNorm = (int *) malloc(WIDTH*HEIGHT*4);
    unsigned char * output_gradDir = (unsigned char *) malloc(WIDTH*HEIGHT);
    unsigned char * output_canny2 = (unsigned char *) malloc(WIDTH*HEIGHT);
    unsigned char * output_canny3 = (unsigned char *) malloc(WIDTH*HEIGHT);

    int N = mainWin->spinBoxBlurSize->value();

    int* kernel = (int*)Gaus1x1;
    if (N == 1) {
        kernel = (int*)Gaus3x3;
    }
    else if (N == 2) {
        kernel = (int*)Gaus5x5;
    }


    imageprocess::suavizado(input, output_blur, WIDTH, HEIGHT, kernel, N);
    memcpy(output_binary, output_blur, WIDTH*HEIGHT);
    if(mainWin->ApplyBinarization->isChecked())
        imageprocess::umbralizar(output_blur, output_binary, mainWin->BinaryThreshold->value(), WIDTH, HEIGHT);

    memset(gradients, 0, WIDTH*HEIGHT*sizeof(Gradient));
    imageprocess::calculoGradientes(output_binary, gradients, WIDTH, HEIGHT);

    for (unsigned int y = 0; y < HEIGHT; y++) {
        for (unsigned int x = 0; x < WIDTH; x++) {
            int index = y * WIDTH + x;
            float val = sqrt(pow(gradients[index].dx,2) + pow(gradients[index].dy,2)); // Initialize output with gradient magnitude
            output_gradNorm[index] = val;
            if(val > 255)
                output_canny1[index] = 255;
            else
                output_canny1[index] = val;

            float angle = atan2(gradients[index].dy, gradients[index].dx);

            float theta = angle * (360.0 / (2.0 * M_PI));

            if ((theta <= 22.5 && theta >= -22.5) || (theta <= -157.5) ||
                (theta >= 157.5))
            {
                output_gradDir[index] = 0;
            }
            else if ((theta > 22.5 && theta <= 67.5) ||
                       (theta > -157.5 && theta <= -112.5))
            {
                output_gradDir[index] = 1;
            }
            else if ((theta > 67.5 && theta <= 112.5) ||
                       (theta >= -112.5 && theta < -67.5))
            {
                output_gradDir[index] = 2;
            }
            else if ((theta >= -67.5 && theta < -22.5) ||
                       (theta > 112.5 && theta < 157.5))
            {
                output_gradDir[index] = 3;
            }
        }
    }

    memset(output_canny2, 0, WIDTH*HEIGHT);
    imageprocess::supresionNoMaximo(output_gradNorm, output_gradDir, output_canny2, WIDTH, HEIGHT);

    unsigned char lowThreshold = mainWin->minThresholdSlider->value();
    unsigned char highThreshold = mainWin->maxThresholdSlider->value();
    memset(output_canny3, 0, WIDTH*HEIGHT);
    imageprocess::dobleUmbralizacion(output_canny2, output_canny3, lowThreshold, highThreshold, WIDTH, HEIGHT);


    //output is visited now
    memset(output, 0, WIDTH*HEIGHT);
    findContours(output_canny3, output, contours, nContours);

    mainWin->detectedContours->display(nContours);

    switch(mainWin->selectOutputImage->currentIndex())
    {
        case 0:
            memset(output, 0, WIDTH*HEIGHT);
            break;
        case 1:
            memcpy(output, output_blur, WIDTH*HEIGHT);
            break;
        case 2:
            memcpy(output, output_binary, WIDTH*HEIGHT);
            break;
        case 3:
            memcpy(output, output_canny1, WIDTH*HEIGHT);
            break;
        case 4:
            memcpy(output, output_canny2, WIDTH*HEIGHT);
            break;
        case 5:
            memcpy(output, output_canny3, WIDTH*HEIGHT);
            break;

    }

    free(output_binary);
    free(output_blur);
    free(output_canny1);
    free(output_gradNorm);
    free(output_gradDir);
    free(output_canny2);
    free(output_canny3);

}

void pracAOC::findContours(unsigned char * image, unsigned char *visited, Point contours[1000][5000], int & nContours)
{
    nContours = 0;
    for(unsigned int y = 1; y<HEIGHT-1; y++)
    {
        for(unsigned int x = 1; x<WIDTH-1; x++)
        {
            int index = y*WIDTH + x;
            if(visited[index]==0 && image[index]==255)
            {
                Point ini_point = {(int) x, (int) y};
                int nPoints = followContour(image, visited, ini_point, contours[nContours], WIDTH, HEIGHT);
                if(nPoints>mainWin->spinBoxMinPoints->value())
                {
                    contours[nContours][nPoints] = {-1,-1};
                    float dist=sqrt(pow(contours[nContours][0].x-contours[nContours][nPoints-1].x, 2)+pow(contours[nContours][0].y-contours[nContours][nPoints-1].y, 2));
                    if(dist<20 && nPoints<5000)
                        closedContour[nContours] = true;
                    else
                        closedContour[nContours] = false;

                    int nPPolygon = createPolygon(contours[nContours], closedContour[nContours], nPoints, polygons[nContours], mainWin->doubleSpinBoxDistancePoly->value());
                    polygons[nContours][nPPolygon] = {-1,-1};
                    nContours++;
                }
            }
            if(nContours>=1000)
                break;
        }

        if(nContours>=1000)
            break;

    }

}

int pracAOC::followContour(unsigned char * image, unsigned char * visited, Point ini_point, Point contour[5000], int WIDTH, int HEIGHT)
{
    int nPoints= 0;

    Point directions[8];
    directions[0] = {0, -1};
    directions[4] = {1, -1};
    directions[2] = {1, 0};
    directions[5] = {1, 1};
    directions[1] = {0, 1};
    directions[6] = {-1, 1};
    directions[3] = {-1, 0};
    directions[7] = {-1, -1};

    Point cont_aux[2][5000];
    int nP_sides[2];

    nP_sides[0] = imageprocess::detectaContorno(image, visited, ini_point.x, ini_point.y, directions, cont_aux[0], WIDTH, HEIGHT);
    nP_sides[1] = imageprocess::detectaContorno(image, visited, ini_point.x, ini_point.y, directions, cont_aux[1], WIDTH, HEIGHT);

    int iC = 0;
    for(int i=nP_sides[1]-1; i>0; i--)
    {
        if(iC<5000)
        {
            contour[iC] = cont_aux[1][i];
            iC++;
        }

    }
    for(int i=0; i<nP_sides[0]; i++)
    {
        if(iC<5000)
        {
            contour[iC] = cont_aux[0][i];
            iC++;
        }
    }


    nPoints = iC;

    return nPoints;


}

int pracAOC::createPolygon(Point contour[5000], bool isClosed, int nPoints, Point polygon[5000], float distance)
{
    int nP;
    int index = 0;
    float dmax = 0;

    if(isClosed)
    {
        for(int i = 1; i<nPoints-1; i++)
        {
            float d = sqrt(pow(contour[i].x-contour[0].x,2) + pow(contour[i].y-contour[0].y,2));
            if(d > dmax)
            {
                index = i;
                dmax = d;
            }
        }

        if(dmax > distance)
        {
            int np1 = imageprocess::contornoAPoligono(contour, index+1, polygon, distance);
            int np2 = imageprocess::contornoAPoligono(&contour[index], nPoints-index, &polygon[np1-1], distance);
            nP = np1+np2-1;
            if(nP<0)
                nP = 0;
        }
        else
        {
            polygon[0] = contour[0];
            polygon[1] = contour[nPoints-1];
            nP = 2;
        }

    }
    else
        nP = imageprocess::contornoAPoligono(contour, nPoints, polygon, distance);

    return nP;
}
