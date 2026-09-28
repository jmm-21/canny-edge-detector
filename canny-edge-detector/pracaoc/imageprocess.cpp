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

#include "imageprocess.h"


void imageprocess::suavizado(unsigned char * imagenO, unsigned char * imagenD, int W, int H, int * kernel, int N)
{
    // %0 = imagenO; %1 = imagenD; %2 = W; %3 = H; %4 = kernel; %5 = N

    asm volatile(

        "mov %0, %%rsi;" // imagenO
        "mov %1, %%rdi;" // imagenD
        "mov $0, %%r8d;" //y
    "bSuavizadoY:"
        "mov $0, %%r9d;" //x
        "bSuavizadoX:"
            "mov %4, %%rbx;" //kernel
            "mov $0, %%eax;" //acum
            "mov $0, %%ecx;" //totalK
            "mov %5, %%r10d;"
            "neg %%r10d;" //j
            "bSuavizadoKernelJ:"
                "mov %5, %%r11d;"
                "neg %%r11d;" //i
                "bSuavizadoKernelI:"
                    "mov %%r8d, %%r12d;"
                    "add %%r10d, %%r12d;" //y+j
                    "mov %%r9d, %%r13d;"
                    "add %%r11d, %%r13d;" //x+i
                    "cmp $0, %%r12d;"
                    "jl sigKernelI;"
                    "cmp %3, %%r12d;"
                    "jge sigKernelI;"
                    "cmp $0, %%r13d;"
                    "jl sigKernelI;"
                    "cmp %2, %%r13d;"
                    "jge sigKernelI;"

                    "mov %2, %%r14d;" //W
                    "imul %%r12d, %%r14d;" //(y + j) * W
                    "add %%r13d, %%r14d;" //(x + i)
                    "movsx %%r14d, %%r14;"
                    "movzbl (%%rsi, %%r14), %%edx;" //movzbl para pasar de byte a entero

                    "imull (%%rbx), %%edx;"
                    "add %%edx, %%eax;"
                    "add (%%rbx), %%ecx;"

                "sigKernelI:"
                    "add $4, %%rbx;"
                    "inc %%r11d;"
                    "cmp %5, %%r11d;"
                    "jle bSuavizadoKernelI;"
            "sigKernelJ:"
                "inc %%r10d;"
                "cmp %5, %%r10d;"
                "jle bSuavizadoKernelJ;"
            "mov $0, %%edx;"
            "div %%ecx;" // acum/kernel_t
            "mov %%r8d, %%r14d;"
            "imul %2, %%r14d;"
            "add %%r9d, %%r14d;"
            "movsx %%r14d, %%r14;"
            "mov %%al, (%%rdi, %%r14);"
            "inc %%r9d;"
            "cmp %2, %%r9d;"
            "jl bSuavizadoX;"
        "inc %%r8d;"
        "cmp %3, %%r8d;"
        "jl bSuavizadoY;"


        :
        : "m" (imagenO), "m" (imagenD), "m" (W), "m" (H), "m" (kernel), "m" (N)
        : "%rax", "%rbx", "%rcx", "%rdx", "%rsi", "%rdi", "%r8", "%r9", "%r10", "%r11", "%r12",
          "%r13", "%r14", "memory"

    );


}

void imageprocess::umbralizar(unsigned char * imagenO, unsigned char * imagenD, unsigned char umbral, int W, int H)
{
    // %0 = imagenO; %1 = imagenD; %2 = umbral; %3 = W; %4 = H

    asm volatile(

        "mov %0, %%rsi;" // imagenO
        "mov %1, %%rdi;" // imagenD
        "mov $0, %%r8d;" //y
        "bImageprocessY:"
              "mov $0, %%r9d;" //x
              "bImageprocessX:"
                    "mov %3, %%r10d;" //W
                    "imul %%r8d, %%r10d;" //y * W
                    "add %%r9d, %%r10d;" //y * W + x
                    "movsx %%r10d, %%r10;"
                    "mov (%%rsi, %%r10), %%al;" //[dirImagenO + offsetPixel]
                    "cmp %2, %%al;"
                    "ja sigUmbral;"
                    "movb $0, (%%rdi, %%r10);"
                    "jmp sigpunto;"
               "sigUmbral:"
                    "movb $255, (%%rdi, %%r10);"
               "sigpunto:"
               "inc %%r9d;"
               "cmp %3, %%r9d;"
               "jl bImageprocessX;"
         "inc %%r8d;"
         "cmp %4, %%r8d;"
         "jl bImageprocessY;"

        :
        : "m" (imagenO), "m" (imagenD), "m" (umbral), "m" (W), "m" (H)
        : "%rsi", "%rdi", "%r8", "%r9","%r10", "%rax" , "memory"

    );

}

void imageprocess::calculoGradientes(unsigned char* imagen, Gradient* gradientes, int W, int H)
{
    int kernel_x[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
    int * sobel_x = (int *) &kernel_x;
    int kernel_y[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};
    int *  sobel_y = (int *) &kernel_y;

    // %0 = imagen; %1 = gradientes; %2 = W; %3 = H; %4 = sobel_x; %5 = sobel_y

    asm volatile(
                  "mov %0, %%rsi;" // imagenO
                        "mov %1, %%rdi;" // gradientes
                         "mov $0, %%r8d;" //y
                            "bCalculoGradientesY:"
                                "mov $0, %%r9d;" //x
                                "bCalculoGradientesX:"
                                    "mov %4, %%rax;" // sobel_x
                                    "mov %5, %%rbx;" // sobel_y
                                    "mov $0, %%r10d;" // gradX = 0;
                                    "mov $0, %%r11d;" // gradY = 0;

                                    "mov $-1, %%r12;" // j= -1
                                    "bCalculoGradientesJ:"
                                        "mov $-1, %%r13;"  //i= -1
                                        "bCalculoGradientesI:"
                                             "mov %%r9d, %%r15d;"
                                             "add %%r13d, %%r15d;" //x+1
                                             "mov %%r8d, %%r14d;"
                                             "add %%r12d, %%r14d;" // y+j
                                             "cmp $0, %%r15d;"
                                             "jl CalculoGradientesElse;"
                                             "cmp %2, %%r15d;"
                                             "jge CalculoGradientesElse;"
                                             "cmp $0, %%r14d;"
                                             "jl CalculoGradientesElse;"
                                             "cmp %3, %%r14d;"
                                             "jge CalculoGradientesElse;"

                                             "imul %2, %%r14d;" //(y + j) * W
                                             "add %%r15d, %%r14d;" //(y + j) * W + (x + i)
                                             "movsx %%r14d, %%r14;"
                                             "movzbl (%%rsi, %%r14), %%edx;" //movzbl para pasar de byte a entero, %%edx es offsetPixel
                                             "imul (%%rax), %%edx;"
                                             "add %%edx, %%r10d;"
                                             "movzbl (%%rsi, %%r14), %%edx;"
                                             "imul (%%rbx), %%edx;"
                                             "add %%edx, %%r11d;"

                                             "CalculoGradientesElse:"
                                             "add $4, %%rax;" //dirSobelX += 4
                                             "add $4, %%rbx;" //dirSobelY += 4

                                        "inc %%r13d;"
                                        "cmp $1, %%r13d;"
                                        "jle bCalculoGradientesI;"
                                    "inc %%r12d;"
                                    "cmp $1, %%r12d;"
                                    "jle bCalculoGradientesJ;"

                                   "mov %%r8d, %%r13d;"
                                   "imul %2, %%r13d;" //y * W
                                   "add %%r9d, %%r13d;" //y * W + x
                                   "movsx %%r13d, %%r13;" // offsetPixel = y*W + x

                                  "mov %%r10, (%%rdi, %%r13, 8);" //[dirGradientes + offsetPixel*8].dx = gradX;
                                  "mov %%r11, 4(%%rdi, %%r13, 8);" //[dirGradientes + offsetPixel*8].dy = gradY;

                                "inc %%r9d;"
                                "cmp %2, %%r9d;"
                                "jl bCalculoGradientesX;"
                          "inc %%r8d;"
                          "cmp %3, %%r8d;"
                          "jl bCalculoGradientesY;"


                        :
                        : "m" (imagen), "m" (gradientes), "m" (W), "m" (H), "m" (sobel_x), "m" (sobel_y)
                        :  "%rsi", "%rdi", "%r8", "%r9","%r10", "%r11",  "%r12","%r13","%r14", "%r15", "%rax", "%rbx", "%rdx" ,"memory"
           );
}





void imageprocess::supresionNoMaximo(int * gradNorm, unsigned char * gradDir, unsigned char* imagenD, int W, int H)

{

    // %0 = gradNorm; %1 = gradDir; %2 = imagenD; %3 = W; %4 = H

    asm volatile(

                "mov %0, %%rsi;" // dirGradNorm = gradNorm
                "mov %1, %%rdi;" // dirGradDir = gradDir
                "mov %2, %%r8;" // dirImagenD = imagenD
                   "mov $1, %%r9;" //y
                   "bSupreY:"
                        "mov $1, %%r10d;" //x
                        "bSupreX:"
                        "mov %3, %%r11d;" //W
                        "imul %%r9d, %%r11d;" //y * W
                        "add %%r10d, %%r11d;" //offsetPixel= y * W + x
                        "movsx %%r11d, %%r11;"
                        "mov (%%rsi, %%r11,4), %%eax;"//[dirGradNorm+offsetPixel*4]
                                "cmp $255, %%eax;"
                                "jge finCond1;"
                                "mov %%al, (%%r8, %%r11);"
                                "jmp caseSupre;"
                                "finCond1:"
                                "movb $255, (%%r8, %%r11);" // [dirImagenD+offsetPixel]=255
                                        "caseSupre:"
                                        "cmpb $0, (%%rdi, %%r11);" //[dirGradDir+offsetPixel]==0
                                        "je L0;"
                                        "cmpb $1, (%%rdi, %%r11);" //[dirGradDir+offsetPixel]==1
                                        "je L1;"
                                        "cmpb $2, (%%rdi, %%r11);" //[dirGradDir+offsetPixel]==2
                                        "je L2;"
                                        "cmpb $3, (%%rdi, %%r11);" //[dirGradDir+offsetPixel]==3
                                        "je L3;"
                                        "jmp finCaseSupre;"
                                        "L4:"
                                        "movb $0, (%%r8,%%r11);" // [dirImagenD+offsetPixel]=0
                                        "jmp sigBucleX_supNoMax;"// salta al último if
                                        "L3:"
                                        "mov %3, %%r15d;" //W
                                        "dec %%r15d;" //W-1
                                        "add %%r11d, %%r15d;" // vOffset2 = offsetPixel+W-1
                                        "mov %3, %%r14d;"
                                        "neg %%r14d;"
                                        "inc %%r14d;"// -W+1
                                        "add %%r11d, %%r14d;"// vOffset1 = offsetPixel-W+1
                                        "jmp finCaseSupre;" // salta al último if
                                        "L2:"
                                        "mov %3, %%r15d;" //W
                                        "add %%r11d, %%r15d;" // vOffset2 = offsetPixel+W
                                        "mov %3, %%r14d;"
                                        "neg %%r14d;"// -W
                                        "add %%r11d, %%r14d;"// vOffset1 = offsetPixel-W
                                        "jmp finCaseSupre;" // salta al último if
                                        "L1:"
                                        "mov %3, %%r15d;" //W
                                        "inc %%r15d;" //W+1
                                        "add %%r11d, %%r15d;" // vOffset2 = offsetPixel+W+1
                                        "mov %3, %%r14d;"
                                        "neg %%r14d;"
                                        "dec %%r14d;"// -W-1
                                        "add %%r11d, %%r14d;"// vOffset1 = offsetPixel-W-1
                                        "jmp finCaseSupre;" // salta al último if
                                        "L0:"
                                        "mov %%r11d, %%r14d;"
                                        "dec %%r14d;"
                                        "mov %%r11d, %%r15d;"
                                        "inc %%r15d;"
                                        "finCaseSupre:"
                                        "movsx %%r15d, %%r15;"
                                        "mov (%%rsi, %%r15,4), %%ebx;" //[dirGradNorm+vOffset1*4]
                                        "movsx %%r14d, %%r14;"
                                        "mov (%%rsi, %%r14,4), %%ecx;" //[dirGradNorm+vOffset2*4]
                                        "cmp %%ebx, %%eax;" //[dirGradNorm+offsetPixel*4]<=[dirGradNorm+vOffset1*4]
                                        "jle L4;"
                                        "cmp %%ecx, %%eax;" //[dirGradNorm+offsetPixel*4]<[dirGradNorm+vOffset2*4]
                                        "jl L4;"
                        "sigBucleX_supNoMax:"
                        "inc %%r10d;"
                        "mov %3, %%r13d;"
                        "dec %%r13d;" //W-1
                        "cmp %%r13d, %%r10d;"
                        "jle bSupreX;"
                   "inc %%r9d;"
                   "mov %4, %%r13d;"
                   "dec %%r13d;" //H-1
                   "cmp %%r13d, %%r9d;"
                   "jle bSupreY;"
        :
        : "m" (gradNorm), "m" (gradDir), "m" (imagenD), "m" (W), "m" (H)
        :  "%rsi", "%rdi", "%r8", "%r9","%r10", "%r11",  "%r12","%r13","%r14", "%r15", "%rax", "%rbx","%rcx" ,"memory"
    );

}

void imageprocess::dobleUmbralizacion(unsigned char* imagenO, unsigned char* imagenD, unsigned char umbralMin, unsigned char umbralMax, int W, int H)
{

  //   %0 = imagenO; %1 = imagenD; %2 = umbralMin; %3 = umbralMax; %4 = W; %5 = H

    asm volatile(
                "mov %0, %%rsi;" // imagenO
                "mov %1, %%rdi;" // imagenD
                "mov $1, %%r8d;" //y
                "dUmbrY:"
                      "mov $1, %%r9d;" //x
                      "dUmbrX:"
                            "mov %4, %%r10d;" //W
                            "imul %%r8d, %%r10d;" //y * W
                            "add %%r9d, %%r10d;" //y * W + x
                            "movsx %%r10d, %%r10;"
                            "mov (%%rsi, %%r10), %%al;" //[dirImagenO + offsetPixel]
                            "cmp %3, %%al;" //Condicion Si 1
                            "jb jConditionElse;"
                            "movb $255, (%%rdi, %%r10);" // [dirImagenD+offsetPixel] = 255
                            "jmp sigCondition;"

                            "jConditionElse:"
                            "movb $0, %%r11b;" // esBorde = Falso
                            "cmp %2, %%al;" // Condición Si 2
                            "jb contUmbral;"
                                "mov $-1, %%r12d;" //j
                                "loopNoEsBordeJ:" // while j
                                "cmp $1, %%r12d;"
                                "jg finLoopJ;"
                                "cmp $0, %%r11b;"
                                "jne finLoopJ;"
                                    "mov $-1, %%r13d;" //i
                                    "loopNoEsBordeI:" // while i
                                    "cmp $1, %%r13d;"
                                    "jg finLoopI;"
                                    "cmp $0, %%r11b;"
                                    "jne finLoopI;"
                                        "mov %%r8d, %%r15d;" //y
                                        "add %%r12d, %%r15d;" //y+j
                                        "imul %4, %%r15d;" //(y + j) * W
                                        "add %%r9d, %%r15d;" // + x
                                        "add %%r13d, %%r15d;" //+ i
                                        "movsx %%r15d, %%r15;"
                                        "mov (%%rsi, %%r15), %%dl;" //[dirImagenO+vOffset]
                                        "mov (%%rdi, %%r15), %%cl;" //[dirImagenD+vOffset]
                                        "cmp %3, %%dl;" //Condición Si 3
                                        "jae compEsBorde;"
                                        "cmp $255, %%cl;"
                                        "je compEsBorde;"
                                        "jmp pasa;"

                                        "compEsBorde:"
                                        "movb $1, %%r11b;" //esBorde = Cierto
                                        "pasa:"

                                        "inc %%r13d;"
                                        "jmp loopNoEsBordeI;"
                                        "finLoopI:"
                                    "inc %%r12d;"
                                    "jmp loopNoEsBordeJ;"
                                    "finLoopJ:"
                            "contUmbral:"

                        "cmp $1, %%r11b;"
                        "jne saltCond;"
                        "movb $255, (%%rdi, %%r10);"
                        "jmp sigCondition;"
                        "saltCond:"
                        "movb $0, (%%rdi, %%r10);"
                    "sigCondition:"
                      "inc %%r9d;"
                      "mov %4, %%r13d;"
                      "dec %%r13d;"
                      "cmp %%r13d, %%r9d;"
                      "jl dUmbrX;"

                "inc %%r8d;"
                "mov %5, %%r12d;"
                "dec %%r12d;"
                "cmp %%r12d, %%r8d;"
                "jl dUmbrY;"


        :
        : "m" (imagenO), "m" (imagenD), "m" (umbralMin), "m" (umbralMax), "m" (W), "m" (H)
        : "%rsi", "%rdi", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "%rax","%rdx", "memory"

    );


}


int imageprocess::detectaContorno(unsigned char * imagen, unsigned char * visitados, int ini_x, int ini_y, Point direcciones[8], Point contorno[5000], int W, int H)
{
    int nPuntos = 0;

    // %0 = nPuntos; %1 = imagen; %2 = visitados; %3 = ini_x; %4 = ini_y; %5 = direcciones; %6 = contorno; %7 = W; %8 = H

    asm volatile(

       "mov %1, %%rsi;" // dirImagen = imagen
       "mov %2, %%rdi;" // dirVisitados = visitados
       "mov %6, %%rax;" // dirContorno = contorno
       "mov %5, %%rbx;"  // dirDirecciones = direcciones
            "mov %3, %%r8d ;" // sig_x = ini_x
            "mov %4, %%r9d ;" // sig_y = ini_y
            "mov $0, %%r10;" // Contorno = 0
            "mov $1, %%r11b;" //hayPunto = cierto
            "b1DetectCont:"
            "cmp $1,%%r11b;"
            "jne terminarB1;"
            "cmp $4999, %%r10d;"
            "jge terminarB1;"
                    "mov %%r8d, (%%rax, %%r10, 8);" // [dirContorno + iContorno*8].x = sig_x
                    "mov %%r9d, 4(%%rax, %%r10, 8);" // [dirContorno + iContorno*8].y = sig_y

                    "mov %7, %%r12d;" //W
                    "imul %%r9d, %%r12d;" //sig_y * W
                    "add %%r8d, %%r12d;" //offsetPixel= sig_y * W + sig_x
                    "movsx %%r12d, %%r12;"
                    "movb $255, (%%rdi, %%r12);" // [dirVisitados + offsetPixel] = 255
                    "inc %%r10;" // iContorno++

                    "mov $0, %%r13;" // iDir = 0
                    "mov $0, %%r11b;" //hayPunto = falso

                    "b2DetectCont:"
                    "cmp $8, %%r13;"
                    "jge terminarB2;"
                    "cmp $0, %%r11b;"
                    "jne terminarB2;"
                          "mov (%%rbx, %%r13, 8), %%r14d;" // [dirDirecciones+iDir*8].x
                          "mov 4(%%rbx, %%r13, 8), %%r15d;" //[dirDirecciones+iDir*8].y


                          "add %%r8d, %%r14d;" //vec_x = sig_x + [dirDirecciones+iDir*8].x
                          "add %%r9d, %%r15d;" //vec_y = sig_y + [dirDirecciones+iDir*8].y

                          "cmp $0, %%r14d;"
                          "jl terminarCond1;"
                          "cmp %7, %%r14d;"
                          "jge terminarCond1;"
                          "cmp $0, %%r15d;"
                          "jl terminarCond1;"
                          "cmp %8, %%r15d;"
                          "jge terminarCond1;"
                                "mov %7, %%r12d;" //W
                                "imul %%r15d, %%r12d;" //vec_y * W
                                "add %%r14d, %%r12d;" //offset_vec= vec_y * W + vec_x
                                "movsx %%r12d, %%r12;"
                                "mov (%%rdi, %%r12), %%cl;"
                                "cmp $0, %%cl;"
                                "jne terminarCond2;"
                                "mov (%%rsi, %%r12), %%cl;"
                                "cmp $255, %%cl;"
                                "jne terminarCond2;"
                                     "mov $1, %%r11b;" //hayPunto = cierto
                                     "mov %%r14d, %%r8d;" // sig_x = vec_x
                                     "mov %%r15d, %%r9d;" // sig_y = vec_y

                                "terminarCond2:"
                          "terminarCond1:"
                          "inc %%r13;" // iDir++
                    "jmp b2DetectCont;"
                    "terminarB2:"
            "jmp b1DetectCont;"
            "terminarB1:"
            "mov %%r10d, %0;" // nPuntos = iContorno
        : "=m" (nPuntos)
        : "m" (imagen), "m" (visitados), "m" (ini_x), "m" (ini_y), "m" (direcciones), "m" (contorno), "m" (W), "m" (H)
        : "%rsi", "%rdi", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "%rax","%rbx", "memory"


    );

    return nPuntos;
}


int imageprocess::contornoAPoligono(Point contorno[5000], int nPuntos, Point poligono[5000], float distancia)
{
    int nP = 0;

    // %rsi = contour; %eax = nPoints; %rdi = polygon; %edx = nP

    asm volatile(
        "mov $0, %%edx;" //nP = 0
        "flds %4;" //distancia en %st(0) para implementación con FPU
        "movss %4, %%xmm0;" //distancia en %xmm0 para implementación con SSE
        "call contornoAPoligonoRecursivo;"
        "fstp %%st(0);"
        "jmp finContornoAPoligono;"

    "contornoAPoligonoRecursivo:"
        // IMPLEMENTACIÓN DEL PROCEDIMIENTO contornoAPoligonoRecursivo

        "ret;"

        "finContornoAPoligono:"

        : "=d" (nP)
        : "S" (contorno), "a" (nPuntos), "D" (poligono), "m" (distancia)
        : "%xmm0", "memory"
    );


    return nP;

}







