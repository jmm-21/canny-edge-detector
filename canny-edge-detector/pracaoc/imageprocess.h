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

#ifndef IMAGEPROCESS_H
#define IMAGEPROCESS_H

#include <QtCore>
#include <math.h>

typedef struct {
    int x, y;
} Point;

typedef struct {
    int dx, dy;
} Gradient;

namespace imageprocess{
    extern void suavizado(unsigned char * imagenO, unsigned char * imagenD, int W, int H, int * kernel, int N);
    extern void umbralizar(unsigned char * imagenO, unsigned char * imagenD, unsigned char umbral, int W, int H);
    extern void calculoGradientes(unsigned char* imagen, Gradient* gradientes, int W, int H);
    extern void supresionNoMaximo(int * gradNorm, unsigned char * gradDir, unsigned char* imagenD, int W, int H);
    extern void dobleUmbralizacion(unsigned char* imagenO, unsigned char* imagenD, unsigned char umbralMin, unsigned char umbralMax, int W, int H);
    extern int detectaContorno(unsigned char * imagen, unsigned char * visitados, int ini_x, int ini_y, Point direcciones[8], Point contorno[5000], int W, int H);
    extern int contornoAPoligono(Point contorno[5000], int nPuntos, Point poligono[5000], float distancia);
}

#endif
