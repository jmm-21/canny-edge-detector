# Detector de Bordes Canny en Ensamblador x86-64

Aplicación de escritorio en C++ y Qt que detecta bordes y contornos en imágenes aplicando el **algoritmo de Canny**, con las etapas de procesamiento de imagen implementadas a mano en **ensamblador x86-64** mediante inline assembly.

Proyecto de la asignatura **Arquitectura y Organización de Computadores** (Grado en Ingeniería Informática del Software, Universidad de Extremadura).

**Tecnologías:** Ensamblador x86-64 (AT&T), C++, Qt, procesamiento de imagen, visión por computador

---

## Qué hace

El usuario carga una imagen y la aplicación la procesa en tiempo real (unas 30 veces por segundo), mostrando la imagen original junto al resultado de la etapa que elija. Sobre el resultado se dibujan los contornos detectados y su aproximación a polígonos.

Desde la interfaz se puede ajustar:

- Tamaño del filtro de suavizado (Gauss 1x1, 3x3 o 5x5)
- Binarización previa y su umbral
- Umbrales mínimo y máximo de la doble umbralización
- Número mínimo de puntos para considerar un contorno
- Tipo de contorno a mostrar (abiertos, cerrados o todos)
- Etapa del proceso que se visualiza

---

## Pipeline de procesamiento

```mermaid
flowchart LR
    A[Imagen original] --> B[Suavizado gaussiano]
    B --> C[Umbralización]
    C --> D[Gradientes Sobel]
    D --> E[Supresión de no máximos]
    E --> F[Doble umbralización]
    F --> G[Seguimiento de contornos]
    G --> H[Aproximación a polígonos]
```

| Etapa | Función | Implementación |
|---|---|---|
| Suavizado gaussiano | `suavizado` | Ensamblador |
| Umbralización | `umbralizar` | Ensamblador |
| Gradientes con operador de Sobel | `calculoGradientes` | Ensamblador |
| Módulo y dirección del gradiente | `canny_and_contours` | C++ |
| Supresión de no máximos | `supresionNoMaximo` | Ensamblador |
| Doble umbralización con histéresis | `dobleUmbralizacion` | Ensamblador |
| Seguimiento de contornos en 8 direcciones | `detectaContorno` | Ensamblador |
| Aproximación de contornos a polígonos | `contornoAPoligono` | Ensamblador (FPU/SSE) |

---

## Aspectos técnicos destacados

- **Inline assembly x86-64 en sintaxis AT&T** dentro de C++, con paso de parámetros por memoria y declaración explícita de registros modificados (clobbers).
- **Recorrido de imágenes a bajo nivel** con direccionamiento indexado y escalado, por ejemplo `(%rsi, %r14)` para píxeles y `(%rdi, %r13, 8)` para estructuras de gradiente.
- **Convolución con kernels** de tamaño variable (Gauss y Sobel) controlando manualmente los bordes de la imagen.
- **Aritmética entera** con extensión de signo y de cero (`movsx`, `movzbl`), multiplicación y división con signo.
- **Uso de 15 registros de propósito general** para mantener índices, acumuladores y punteros sin accesos a memoria innecesarios.
- **Interfaz gráfica en Qt** con dibujo de contornos y polígonos sobre la imagen resultante.

---

## Estructura del código

| Archivo | Responsabilidad |
|---|---|
| `main.cpp` | Punto de entrada de la aplicación Qt |
| `pracaoc.cpp` / `.h` | Ventana principal, carga de imágenes, bucle de procesamiento y dibujado |
| `imageprocess.cpp` / `.h` | Funciones de procesamiento en ensamblador x86-64 |
| `mainForm.ui` | Diseño de la interfaz |
| `pracaoc.pro` | Configuración de compilación de qmake |
| `samples/` | Imágenes de prueba |

## Requisitos

- Linux x86-64
- Qt 5 con los módulos Widgets y OpenGL
- `g++` y `qmake`

```bash
sudo apt install build-essential qtbase5-dev qt5-qmake libqt5opengl5-dev
```

## Cómo ejecutarlo

```bash
git clone https://github.com/jmm-21/canny-edge-detector-asm.git
cd canny-edge-detector-asm
qmake pracaoc.pro
make
./pracAOC
```

También se puede abrir `pracaoc.pro` directamente con Qt Creator y ejecutarlo desde ahí.

Una vez abierta, pulsa **Cargar** y elige una imagen `.png` o `.jpg`. En la carpeta `samples/` hay imágenes de prueba con figuras geométricas.

---

## Autor

**Jorge Méndez Martínez**

La estructura base de la aplicación (interfaz Qt y esqueleto de funciones) fue proporcionada por el profesorado de la asignatura bajo licencia GPL v2. La implementación en ensamblador de las funciones de procesamiento es trabajo propio.
