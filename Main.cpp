#include <iostream>          // Permite utilizar cout y otras funciones de entrada/salida
#include <vector>            // Permite utilizar std::vector para guardar datos dinámicos
#include <cstdlib>           // Permite utilizar abs()
#include <utility>           // Permite utilizar std::pair
#include <cmath>             // Permite utilizar sqrt(), sin(), cos() y lround()
#include <algorithm>         // Permite utilizar min() y max()

#include <glad/glad.h>       // Carga las funciones modernas de OpenGL
#include <GLFW/glfw3.h>      // Permite crear y manejar la ventana de OpenGL

// ImGui: interfaz grafica (selector de color, botones, etc.)
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// Clases utilizadas para manejar OpenGL
#include "shaderClass.h"
#include "VAO.h"
#include "EBO.h"
#include "VBO.h"

using namespace std;


// ============================================================================
// FUNCIÓN PARA AJUSTAR EL ÁREA DE DIBUJO
// ============================================================================

// Esta función se ejecuta automáticamente cuando cambia el tamaño de la ventana.
// width  = nuevo ancho de la ventana
// height = nuevo alto de la ventana
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // Le indicamos a OpenGL que el área donde se va a dibujar
    // debe ocupar todo el nuevo tamaño de la ventana.
    glViewport(0, 0, width, height);
}


// ============================================================================
// ALGORITMO DE BRESENHAM PARA EL CÍRCULO
// ============================================================================

// Esta función calcula las celdas que forman un círculo.
//
// xc    = coordenada X del centro
// yc    = coordenada Y del centro
// radio = radio del círculo
//
// La función devuelve una lista de pares:
// (columna, fila)
//
// Se utiliza un parámetro de decisión llamado "d".
vector<pair<int, int>> bresenhamCirculo(int xc, int yc, int radio)
{
    // Vector donde se guardarán todas las celdas que pertenecen al círculo.
    vector<pair<int, int>> puntos;


    // ------------------------------------------------------------------------
    // PUNTO INICIAL
    // ------------------------------------------------------------------------

    // Comenzamos en la parte superior del círculo.
    int x = 0;
    int y = radio;


    // ------------------------------------------------------------------------
    // PARÁMETRO DE DECISIÓN
    // ------------------------------------------------------------------------

    // El parámetro de decisión determina cuál será el siguiente punto:
    //
    // Si d < 0:
    //      se mantiene el mismo valor de Y.
    //
    // Si d >= 0:
    //      se disminuye Y.
    //
    // Esta es la parte principal del algoritmo de Bresenham.
    int d = 1 - radio;


    // ------------------------------------------------------------------------
    // FUNCIÓN PARA GENERAR LOS 8 PUNTOS SIMÉTRICOS
    // ------------------------------------------------------------------------

    // Un círculo tiene simetría de 8 partes.
    //
    // Por cada punto calculado podemos obtener otros 7 puntos
    // utilizando la simetría del círculo.
    auto agregarOctantes = [&](int x, int y)
        {
            // Primer octante
            puntos.push_back({ xc + x, yc + y });

            // Segundo octante
            puntos.push_back({ xc - x, yc + y });

            // Tercer octante
            puntos.push_back({ xc + x, yc - y });

            // Cuarto octante
            puntos.push_back({ xc - x, yc - y });

            // Quinto octante
            puntos.push_back({ xc + y, yc + x });

            // Sexto octante
            puntos.push_back({ xc - y, yc + x });

            // Séptimo octante
            puntos.push_back({ xc + y, yc - x });

            // Octavo octante
            puntos.push_back({ xc - y, yc - x });
        };


    // Agregamos el primer punto del círculo.
    agregarOctantes(x, y);


    // ------------------------------------------------------------------------
    // RECORRIDO DEL CÍRCULO
    // ------------------------------------------------------------------------

    // Continuamos mientras X sea menor que Y.
    //
    // Cuando X alcanza o supera Y ya hemos recorrido el octante
    // suficiente para reconstruir todo el círculo mediante simetría.
    while (x < y)
    {
        // Siempre avanzamos una posición en X.
        x++;


        // --------------------------------------------------------------------
        // DECISIÓN
        // --------------------------------------------------------------------

        if (d < 0)
        {
            // El parámetro de decisión indica que el siguiente punto
            // debe mantenerse en la misma fila.
            //
            // Por lo tanto, Y no cambia.
            d = d + 2 * x + 1;
        }
        else
        {
            // El parámetro de decisión indica que debemos acercarnos
            // hacia el centro del círculo.
            //
            // Por lo tanto, disminuimos Y.
            y--;

            // Actualizamos el parámetro de decisión.
            d = d + 2 * (x - y) + 1;
        }


        // Agregamos los 8 puntos simétricos del nuevo punto calculado.
        agregarOctantes(x, y);
    }


    // Devolvemos todas las celdas que forman el círculo.
    return puntos;
}


// ============================================================================
// ALGORITMO DE BRESENHAM PARA UNA LÍNEA
// ============================================================================

// Esta función calcula las celdas que forman una línea.
//
// x0, y0 = punto inicial
// x1, y1 = punto final
//
// Devuelve una lista de pares:
// (columna, fila)
//
// IMPORTANTE:
// Esta versión utiliza un "parámetro de decisión" llamado p.
// No utiliza ninguna variable llamada error.
vector<pair<int, int>> bresenham(int x0, int y0, int x1, int y1)
{
    // Vector donde se almacenan las celdas que forman la línea.
    vector<pair<int, int>> puntos;


    // ------------------------------------------------------------------------
    // CALCULAR LAS DISTANCIAS
    // ------------------------------------------------------------------------

    // Distancia horizontal entre los dos puntos.
    int dx = abs(x1 - x0);

    // Distancia vertical entre los dos puntos.
    int dy = abs(y1 - y0);


    // ------------------------------------------------------------------------
    // DETERMINAR LA DIRECCIÓN
    // ------------------------------------------------------------------------

    // sx indica hacia dónde se mueve X.
    //
    // Si x0 < x1:
    //      avanzamos hacia la derecha (+1)
    //
    // Si x0 > x1:
    //      avanzamos hacia la izquierda (-1)
    int sx = (x0 < x1) ? 1 : -1;


    // sy indica hacia dónde se mueve Y.
    //
    // Si y0 < y1:
    //      avanzamos hacia arriba (+1)
    //
    // Si y0 > y1:
    //      avanzamos hacia abajo (-1)
    int sy = (y0 < y1) ? 1 : -1;


    // ------------------------------------------------------------------------
    // DETERMINAR EL EJE PRINCIPAL
    // ------------------------------------------------------------------------

    // Si dx es mayor que dy, la línea avanza principalmente en X.
    //
    // Si dy es mayor que dx, la línea avanza principalmente en Y.
    if (dx >= dy)
    {
        // ================================================================
        // CASO 1: X ES EL EJE PRINCIPAL
        // ================================================================

        // Parámetro de decisión inicial.
        //
        // Fórmula de Bresenham:
        //
        // p = 2dy - dx
        int p = 2 * dy - dx;


        // Continuamos hasta llegar al punto final.
        while (true)
        {
            // Guardamos la celda actual.
            puntos.push_back({ x0, y0 });


            // Si ya llegamos al punto final, terminamos.
            if (x0 == x1 && y0 == y1)
                break;


            // Avanzamos una celda en X.
            x0 += sx;


            // ------------------------------------------------------------
            // TOMAR LA DECISIÓN
            // ------------------------------------------------------------

            if (p < 0)
            {
                // El siguiente punto permanece en la misma fila.

                // Actualizamos el parámetro de decisión.
                p = p + 2 * dy;
            }
            else
            {
                // El siguiente punto debe cambiar de fila.

                // Avanzamos en Y.
                y0 += sy;

                // Actualizamos el parámetro de decisión.
                p = p + 2 * (dy - dx);
            }
        }
    }
    else
    {
        // ================================================================
        // CASO 2: Y ES EL EJE PRINCIPAL
        // ================================================================

        // Parámetro de decisión inicial.
        //
        // En este caso se intercambian los papeles de X y Y.
        //
        // p = 2dx - dy
        int p = 2 * dx - dy;


        // Continuamos hasta llegar al punto final.
        while (true)
        {
            // Guardamos la celda actual.
            puntos.push_back({ x0, y0 });


            // Si ya llegamos al punto final, terminamos.
            if (x0 == x1 && y0 == y1)
                break;


            // Avanzamos una celda en Y.
            y0 += sy;


            // ------------------------------------------------------------
            // TOMAR LA DECISIÓN
            // ------------------------------------------------------------

            if (p < 0)
            {
                // El siguiente punto permanece en la misma columna.

                // Actualizamos el parámetro de decisión.
                p = p + 2 * dx;
            }
            else
            {
                // El siguiente punto debe cambiar de columna.

                // Avanzamos en X.
                x0 += sx;

                // Actualizamos el parámetro de decisión.
                p = p + 2 * (dx - dy);
            }
        }
    }


    // Devolvemos todas las celdas calculadas.
    return puntos;
}


// ============================================================================
// ESTRUCTURAS PARA LAS FIGURAS
// ============================================================================

const float PI = 3.14159265358979f;

// Un punto con decimales. x = columna, y = fila.
// (Con decimales para poder rotar sin perder precisión; solo se
//  redondea a celda enteras al momento de pintar.)
struct Punto
{
    float x;
    float y;
};

// Cómo se dibuja una figura:
//   T_LINEA   -> una línea entre 2 puntos
//   T_CIRCULO -> centro (pts[0]) y un punto del borde (pts[1])
//   T_ABIERTA -> varias líneas seguidas SIN cerrar (lápiz libre)
//   T_CERRADA -> varias líneas seguidas y la última vuelve a la primera (polígonos)
enum TipoForma { T_LINEA, T_CIRCULO, T_ABIERTA, T_CERRADA };

// Una figura es solo: su tipo + la lista de sus vértices.
struct Forma
{
    TipoForma tipo;
    vector<Punto> pts;
};

// Herramientas del menú (el orden debe coincidir con "nombresHerramientas").
enum Herramienta
{
    H_LINEA,
    H_CIRCULO,
    H_LIBRE,
    H_TRIANGULO,
    H_CUADRADO,
    H_POLIGONO_REG,
    H_POLIGONO_CLICS,
    H_RELLENO
};

const char* nombresHerramientas[] =
{
    "Linea",
    "Circulo",
    "Lapiz libre",
    "Triangulo",
    "Cuadrado",
    "Poligono regular",
    "Poligono por clics",
    "Relleno (semilla)"
};


// ============================================================================
// FUNCIÓN PARA PINTAR UNA CELDA
// ============================================================================

void pintarCelda(
    vector<GLfloat>& vertices,
    int fila,
    int col,
    int filas,
    int columnas,
    float r,
    float g,
    float b)
{
    // Si la celda está fuera de la grilla no hacemos nada.
    if (fila < 0 || fila >= filas || col < 0 || col >= columnas)
        return;

    int indiceCelda = fila * columnas + col;

    // 4 vértices por celda y 6 valores por vértice (X,Y,Z,R,G,B).
    int base = indiceCelda * 4 * 6;

    for (int v = 0; v < 4; v++)
    {
        int offset = base + v * 6;
        vertices[offset + 3] = r;
        vertices[offset + 4] = g;
        vertices[offset + 5] = b;
    }
}


// ============================================================================
// CONSTRUIR LOS VÉRTICES DE CADA FIGURA A PARTIR DEL ARRASTRE DEL MOUSE
// ============================================================================

// (c0,f0) = celda donde se presionó el mouse
// (c1,f1) = celda donde está el cursor ahora
// lados   = cantidad de lados (solo para el polígono regular)
Forma formaDesdeArrastre(int herr, int c0, int f0, int c1, int f1, int lados)
{
    Forma forma;
    forma.tipo = T_LINEA;

    Punto a = { (float)c0, (float)f0 };
    Punto b = { (float)c1, (float)f1 };

    switch (herr)
    {
    case H_LINEA:
        forma.tipo = T_LINEA;
        forma.pts = { a, b };
        break;

    case H_CIRCULO:
        // a = centro, b = un punto del borde
        forma.tipo = T_CIRCULO;
        forma.pts = { a, b };
        break;

    case H_TRIANGULO:
    {
        // El ANCHO del arrastre es la base y el ALTO es la altura.
        //   A = donde empezaste (esquina izquierda de la base)
        //   B = misma fila, columna del cursor (esquina derecha de la base)
        //   C = punta: a la mitad de la base y a la fila del cursor
        forma.tipo = T_CERRADA;
        Punto A = a;
        Punto B = { b.x, a.y };
        Punto C = { (a.x + b.x) / 2.0f, b.y };
        forma.pts = { A, B, C };
        break;
    }

    case H_CUADRADO:
    {
        // El lado es la mayor de las dos distancias (horizontal o vertical),
        // así siempre sale cuadrado aunque el arrastre sea rectangular.
        float dx = b.x - a.x;
        float dy = b.y - a.y;
        float lado = max(fabs(dx), fabs(dy));
        float sx = (dx >= 0) ? 1.0f : -1.0f;   // hacia dónde crece en X
        float sy = (dy >= 0) ? 1.0f : -1.0f;   // hacia dónde crece en Y

        forma.tipo = T_CERRADA;
        forma.pts =
        {
            { a.x,            a.y },
            { a.x + sx * lado, a.y },
            { a.x + sx * lado, a.y + sy * lado },
            { a.x,            a.y + sy * lado }
        };
        break;
    }

    case H_POLIGONO_REG:
    {
        // a = centro, b = uno de los vértices.
        // Los demás vértices se reparten cada 360/lados grados.
        // (cuadrado = 4 lados = 90 grados entre vértices, triángulo = 120, ...)
        float dx = b.x - a.x;
        float dy = b.y - a.y;
        float radio = sqrt(dx * dx + dy * dy);
        float anguloInicial = atan2(dy, dx);

        forma.tipo = T_CERRADA;
        for (int i = 0; i < lados; i++)
        {
            float ang = anguloInicial + i * 2.0f * PI / lados;
            Punto p = { a.x + radio * cos(ang), a.y + radio * sin(ang) };
            forma.pts.push_back(p);
        }
        break;
    }
    }

    return forma;
}


// ============================================================================
// ROTACIÓN
// ============================================================================

// Matriz de 3x3 para transformaciones en 2D con coordenadas homogéneas.
// Un punto (x, y) se escribe como la columna (x, y, 1).
struct Mat3
{
    float m[3][3];
};

// Matriz identidad (no cambia nada):  1 0 0 / 0 1 0 / 0 0 1
Mat3 identidad()
{
    Mat3 I;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            I.m[i][j] = (i == j) ? 1.0f : 0.0f;
    return I;
}

// Producto de matrices:  C = A * B
Mat3 multiplicar(const Mat3& A, const Mat3& B)
{
    Mat3 C;
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            C.m[i][j] = 0.0f;
            for (int k = 0; k < 3; k++)
                C.m[i][j] += A.m[i][k] * B.m[k][j];
        }
    }
    return C;
}

// Matriz de traslación:
//   | 1  0  tx |
//   | 0  1  ty |
//   | 0  0  1  |
Mat3 matTraslacion(float tx, float ty)
{
    Mat3 T = identidad();
    T.m[0][2] = tx;
    T.m[1][2] = ty;
    return T;
}

// Matriz de rotación (ángulo en radianes, antihorario):
//   | cos  -sin  0 |
//   | sin   cos  0 |
//   |  0     0   1 |
Mat3 matRotacion(float ang)
{
    Mat3 R = identidad();
    R.m[0][0] = cos(ang);
    R.m[0][1] = -sin(ang);
    R.m[1][0] = sin(ang);
    R.m[1][1] = cos(ang);
    return R;
}

// Aplica una matriz a un punto:  (x', y', 1) = M * (x, y, 1)
Punto aplicarMatriz(const Mat3& M, Punto p)
{
    Punto r;
    r.x = M.m[0][0] * p.x + M.m[0][1] * p.y + M.m[0][2];
    r.y = M.m[1][0] * p.x + M.m[1][1] * p.y + M.m[1][2];
    return r;
}

// La rotación de las matrices siempre gira alrededor del ORIGEN (0,0).
// Para girar alrededor de otro punto (el pivote) se combinan 3 matrices:
//   1) T(-pivote): mueve el pivote al origen
//   2) R(ang):     gira
//   3) T(pivote):  devuelve todo a su lugar
//   M = T(pivote) * R(ang) * T(-pivote)      (se aplica de derecha a izquierda)
Mat3 matRotacionAlrededor(Punto pivote, float ang)
{
    Mat3 aOrigen = matTraslacion(-pivote.x, -pivote.y);
    Mat3 giro = matRotacion(ang);
    Mat3 devolver = matTraslacion(pivote.x, pivote.y);

    return multiplicar(devolver, multiplicar(giro, aOrigen));
}

// Centro de la figura: el centro de la caja que la contiene.
// (Para el círculo es directamente su centro.)
Punto centroForma(const Forma& forma)
{
    if (forma.tipo == T_CIRCULO)
        return forma.pts[0];

    float minX = forma.pts[0].x, maxX = forma.pts[0].x;
    float minY = forma.pts[0].y, maxY = forma.pts[0].y;

    for (const Punto& p : forma.pts)
    {
        minX = min(minX, p.x);
        maxX = max(maxX, p.x);
        minY = min(minY, p.y);
        maxY = max(maxY, p.y);
    }

    Punto c = { (minX + maxX) / 2.0f, (minY + maxY) / 2.0f };
    return c;
}


// ============================================================================
// DE FIGURA A CELDAS (usando Bresenham)
// ============================================================================

vector<pair<int, int>> celdasDeForma(const Forma& forma)
{
    vector<pair<int, int>> celdas;

    if (forma.pts.empty())
        return celdas;

    // Redondea un punto con decimales a una celda entera (columna, fila).
    auto redondear = [](Punto p)
        {
            return make_pair((int)lround(p.x), (int)lround(p.y));
        };

    auto agregar = [&](const vector<pair<int, int>>& nuevas)
        {
            celdas.insert(celdas.end(), nuevas.begin(), nuevas.end());
        };

    if (forma.tipo == T_LINEA)
    {
        pair<int, int> a = redondear(forma.pts[0]);
        pair<int, int> b = redondear(forma.pts[1]);
        agregar(bresenham(a.first, a.second, b.first, b.second));
    }
    else if (forma.tipo == T_CIRCULO)
    {
        float dx = forma.pts[1].x - forma.pts[0].x;
        float dy = forma.pts[1].y - forma.pts[0].y;
        int radio = (int)lround(sqrt(dx * dx + dy * dy));

        pair<int, int> c = redondear(forma.pts[0]);
        agregar(bresenhamCirculo(c.first, c.second, radio));
    }
    else
    {
        // T_ABIERTA y T_CERRADA: se unen los vértices uno tras otro con líneas.
        size_t n = forma.pts.size();

        if (n == 1)
            celdas.push_back(redondear(forma.pts[0]));

        for (size_t i = 0; i + 1 < n; i++)
        {
            pair<int, int> a = redondear(forma.pts[i]);
            pair<int, int> b = redondear(forma.pts[i + 1]);
            agregar(bresenham(a.first, a.second, b.first, b.second));
        }

        // Si es cerrada, la última línea vuelve al primer vértice.
        if (forma.tipo == T_CERRADA && n > 2)
        {
            pair<int, int> a = redondear(forma.pts[n - 1]);
            pair<int, int> b = redondear(forma.pts[0]);
            agregar(bresenham(a.first, a.second, b.first, b.second));
        }
    }

    return celdas;
}


// ============================================================================
// DIBUJAR UNA FIGURA (con rotación opcional)
// ============================================================================

// anguloGrados = cuánto se gira la figura
// pivoteModo   = alrededor de qué punto:
//                0 = centro de la figura
//                1 = origen (celda 0,0: esquina inferior izquierda)
//                2 = centro del lienzo
void dibujarForma(
    vector<GLfloat>& vertices,
    const Forma& forma,
    float anguloGrados,
    int pivoteModo,
    int filas,
    int columnas,
    const float color[3])
{
    Forma girada = forma;

    if (anguloGrados != 0.0f)
    {
        Punto pivote;

        if (pivoteModo == 0)
            pivote = centroForma(forma);
        else if (pivoteModo == 1)
            pivote = { 0.0f, 0.0f };
        else
            pivote = { (columnas - 1) / 2.0f, (filas - 1) / 2.0f };

        float ang = anguloGrados * PI / 180.0f;

        // Se arma UNA matriz con el giro alrededor del pivote...
        Mat3 M = matRotacionAlrededor(pivote, ang);

        // ...y se aplica a cada VÉRTICE (no a los píxeles ya pintados);
        // después se trazan las líneas. Así la figura rotada queda sin huecos.
        for (Punto& p : girada.pts)
            p = aplicarMatriz(M, p);
    }

    for (auto& celda : celdasDeForma(girada))
    {
        pintarCelda(vertices, celda.second, celda.first,
            filas, columnas, color[0], color[1], color[2]);
    }
}


// ============================================================================
// RELLENO POR SEMILLA (flood fill, 4 vecinos)
// ============================================================================

// Parte de la celda (col, fila) -la "semilla"- y pinta esa celda y todas las
// vecinas conectadas que tengan su MISMO color original, hasta chocar con
// celdas de otro color (el borde de la figura).
void rellenoSemilla(
    vector<GLfloat>& vertices,
    int col,
    int fila,
    int filas,
    int columnas,
    const float color[3])
{
    if (fila < 0 || fila >= filas || col < 0 || col >= columnas)
        return;

    // Lee el color de una celda (usa su primer vértice).
    auto colorDe = [&](int f, int c, float& r, float& g, float& b)
        {
            int base = (f * columnas + c) * 4 * 6;
            r = vertices[base + 3];
            g = vertices[base + 4];
            b = vertices[base + 5];
        };

    auto iguales = [](float a, float b) { return fabs(a - b) < 0.0001f; };

    // Color original de la semilla: es el color que se va a reemplazar.
    float rO, gO, bO;
    colorDe(fila, col, rO, gO, bO);

    // Si ya tiene el color nuevo no hay nada que hacer
    // (y se evita repetir el relleno sin fin).
    if (iguales(rO, color[0]) && iguales(gO, color[1]) && iguales(bO, color[2]))
        return;

    // Pila de celdas pendientes por revisar (columna, fila).
    // Se usa una pila en vez de recursión para no desbordar la memoria.
    vector<pair<int, int>> pila;
    pila.push_back(make_pair(col, fila));

    while (!pila.empty())
    {
        pair<int, int> celda = pila.back();
        pila.pop_back();

        int c = celda.first;
        int f = celda.second;

        if (f < 0 || f >= filas || c < 0 || c >= columnas)
            continue;

        float r, g, b;
        colorDe(f, c, r, g, b);

        // Si no es del color original, es borde (o ya se pintó): no se toca.
        if (!iguales(r, rO) || !iguales(g, gO) || !iguales(b, bO))
            continue;

        pintarCelda(vertices, f, c, filas, columnas, color[0], color[1], color[2]);

        // Se agregan los 4 vecinos: derecha, izquierda, arriba y abajo.
        pila.push_back(make_pair(c + 1, f));
        pila.push_back(make_pair(c - 1, f));
        pila.push_back(make_pair(c, f + 1));
        pila.push_back(make_pair(c, f - 1));
    }
}


// ============================================================================
// FUNCIÓN PARA SABER QUÉ CELDA ESTÁ DEBAJO DEL CURSOR
// ============================================================================

void celdaBajoCursor(GLFWwindow* window, int filas, int columnas, int& col, int& fila)
{
    // Posición del cursor en píxeles (el origen está ARRIBA a la izquierda).
    double mx, my;
    glfwGetCursorPos(window, &mx, &my);

    int ancho, alto;
    glfwGetWindowSize(window, &ancho, &alto);

    // En Y invertimos (alto - my) porque la fila 0 de la grilla está ABAJO.
    col = (int)(mx / ((double)ancho / columnas));
    fila = (int)((alto - my) / ((double)alto / filas));

    if (col < 0) col = 0;
    if (col >= columnas) col = columnas - 1;
    if (fila < 0) fila = 0;
    if (fila >= filas) fila = filas - 1;
}


// ============================================================================
// FUNCIÓN PRINCIPAL
// ============================================================================

int main()
{
    // ------------------------------------------------------------------------
    // INICIALIZAR GLFW
    // ------------------------------------------------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    // ------------------------------------------------------------------------
    // CONFIGURACIÓN DE LA GRILLA
    // ------------------------------------------------------------------------
    int filas = 100;
    int columnas = 100;

    float anchoCelda = 2.0f / columnas;
    float altoCelda = 2.0f / filas;

    // "vertices" es el lienzo guardado (lo que ya está dibujado).
    vector<GLfloat> vertices;
    vector<GLuint> indices;

    for (int f = 0; f < filas; f++)
    {
        for (int c = 0; c < columnas; c++)
        {
            float x0 = -1.0f + c * anchoCelda;
            float x1 = x0 + anchoCelda;
            float y0 = -1.0f + f * altoCelda;
            float y1 = y0 + altoCelda;

            unsigned int base = vertices.size() / 6;

            vertices.insert(vertices.end(),
                {
                    x0, y0, 0.0f, 0.5f, 0.5f, 0.5f,
                    x1, y0, 0.0f, 0.5f, 0.5f, 0.5f,
                    x1, y1, 0.0f, 0.5f, 0.5f, 0.5f,
                    x0, y1, 0.0f, 0.5f, 0.5f, 0.5f
                });

            indices.insert(indices.end(),
                {
                    base + 0, base + 1, base + 3,
                    base + 1, base + 2, base + 3
                });
        }
    }


    // ------------------------------------------------------------------------
    // CREAR LA VENTANA
    // ------------------------------------------------------------------------
    GLFWwindow* window = glfwCreateWindow(800, 800, "Int Computacion Grafica", NULL, NULL);

    if (window == NULL)
    {
        cout << "Falla al crear la ventana de GLFW" << endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGL())
    {
        cout << "Falla al inicializar GLAD" << endl;
        glfwTerminate();
        return -1;
    }

    glViewport(0, 0, 800, 800);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


    // ------------------------------------------------------------------------
    // INICIALIZAR IMGUI
    // ------------------------------------------------------------------------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");


    // ------------------------------------------------------------------------
    // SHADER, VAO, VBO, EBO
    // ------------------------------------------------------------------------
    Shader shaderProgram("default.vert", "default.frag");

    VAO VAO1;
    VAO1.Bind();

    VBO VBO1(vertices.data(), vertices.size() * sizeof(GLfloat));
    EBO EBO1(indices.data(), indices.size() * sizeof(GLuint));

    VAO1.LinkAttrib(VBO1, 0, 3, GL_FLOAT, 6 * sizeof(GLfloat), (void*)0);
    VAO1.LinkAttrib(VBO1, 1, 3, GL_FLOAT, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

    VAO1.Unbind();
    VBO1.Unbind();
    EBO1.Unbind();


    // ------------------------------------------------------------------------
    // VARIABLES DE LAS HERRAMIENTAS
    // ------------------------------------------------------------------------

    float colorActual[3] = { 0.0f, 0.0f, 0.0f };   // color elegido (R,G,B)
    int herramienta = H_LINEA;                     // figura elegida en el menú
    int lados = 5;                                 // lados del polígono regular

    // Rotación
    float angulo = 0.0f;                           // en grados
    int pivoteModo = 0;                            // 0 centro figura, 1 origen, 2 centro lienzo

    // Estado del arrastre con el mouse
    bool arrastrando = false;
    int colInicio = 0, filaInicio = 0;             // celda donde se presionó
    int colCursor = 0, filaCursor = 0;             // celda actual del cursor
    bool presionadoAntes = false;                  // para detectar el "momento del clic"
    bool derechoAntes = false;

    vector<Punto> libre;                           // puntos del lápiz libre
    vector<Punto> poliPuntos;                      // vértices del polígono por clics

    Forma formaActual;                             // figura que se está arrastrando
    formaActual.tipo = T_LINEA;

    // Figura "pendiente": ya soltaste el mouse, pero todavía puedes girarla
    // o cambiarle el color. Se guarda en el lienzo al pulsar "Aplicar figura"
    // o al empezar otra figura.
    bool pendiente = false;
    Forma formaPend;
    formaPend.tipo = T_LINEA;

    vector<GLfloat> vista;                         // lienzo + figura en pantalla


    // Pasa la figura pendiente al lienzo guardado.
    auto aplicarPendiente = [&]()
        {
            if (!pendiente)
                return;

            dibujarForma(vertices, formaPend, angulo, pivoteModo, filas, columnas, colorActual);
            pendiente = false;
            angulo = 0.0f;   // la siguiente figura empieza sin giro
        };

    // Convierte los clics acumulados en un polígono cerrado (pendiente).
    auto cerrarPoligono = [&]()
        {
            if (poliPuntos.size() < 3)
                return;

            if (pendiente)
                aplicarPendiente();

            formaPend.tipo = T_CERRADA;
            formaPend.pts = poliPuntos;
            pendiente = true;
            poliPuntos.clear();
        };


    // ------------------------------------------------------------------------
    // BUCLE PRINCIPAL
    // ------------------------------------------------------------------------
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Celda sobre la que está el cursor en este frame.
        celdaBajoCursor(window, filas, columnas, colCursor, filaCursor);


        // --------------------------------------------------------------------
        // PANEL DE HERRAMIENTAS
        // --------------------------------------------------------------------
        ImGui::SetNextWindowSize(ImVec2(300, 0), ImGuiCond_FirstUseEver);
        ImGui::Begin("Herramientas");

        ImGui::ColorEdit3("Color", colorActual);

        ImGui::Separator();

        // Lista desplegable con todas las figuras.
        ImGui::Combo("Figura", &herramienta, nombresHerramientas, IM_ARRAYSIZE(nombresHerramientas));

        if (herramienta == H_POLIGONO_REG)
        {
            ImGui::SliderInt("Lados", &lados, 3, 12);
        }

        if (herramienta == H_POLIGONO_CLICS)
        {
            ImGui::TextWrapped("Clic izquierdo: agregar vertice. Clic derecho (o el boton): cerrar.");

            if (ImGui::Button("Cerrar poligono"))
                cerrarPoligono();
        }

        if (herramienta == H_RELLENO)
        {
            ImGui::TextWrapped("Clic dentro de una figura cerrada para rellenarla con el color elegido.");
        }

        ImGui::Separator();

        ImGui::Text("Rotacion");
        ImGui::SliderFloat("Angulo", &angulo, -180.0f, 180.0f, "%.0f grados");
        ImGui::RadioButton("Centro de la figura", &pivoteModo, 0);
        ImGui::RadioButton("Origen (esquina 0,0)", &pivoteModo, 1);
        ImGui::RadioButton("Centro del lienzo", &pivoteModo, 2);

        ImGui::Separator();

        // Estos botones solo funcionan si hay una figura pendiente.
        ImGui::BeginDisabled(!pendiente);
        if (ImGui::Button("Aplicar figura"))
            aplicarPendiente();
        ImGui::SameLine();
        if (ImGui::Button("Cancelar figura"))
        {
            pendiente = false;
            angulo = 0.0f;
        }
        ImGui::EndDisabled();

        if (ImGui::Button("Limpiar lienzo"))
        {
            for (int f = 0; f < filas; f++)
                for (int c = 0; c < columnas; c++)
                    pintarCelda(vertices, f, c, filas, columnas, 0.5f, 0.5f, 0.5f);

            pendiente = false;
            poliPuntos.clear();
            angulo = 0.0f;
        }

        ImGui::Text("Celda: col %d, fila %d", colCursor, filaCursor);

        ImGui::End();


        // --------------------------------------------------------------------
        // LÓGICA DEL MOUSE
        // --------------------------------------------------------------------
        bool presionado = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
        bool derecho = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);
        bool sobreLienzo = !io.WantCaptureMouse;   // el mouse NO está sobre el panel

        if (herramienta == H_POLIGONO_CLICS)
        {
            // Polígono por clics: cada clic agrega un vértice (no se arrastra).
            arrastrando = false;

            if (presionado && !presionadoAntes && sobreLienzo)
            {
                if (pendiente)
                    aplicarPendiente();

                Punto v = { (float)colCursor, (float)filaCursor };
                poliPuntos.push_back(v);
            }

            if (derecho && !derechoAntes && sobreLienzo)
                cerrarPoligono();
        }
        else if (herramienta == H_RELLENO)
        {
            // Relleno: un clic = una semilla. No se arrastra nada.
            arrastrando = false;
            poliPuntos.clear();

            if (presionado && !presionadoAntes && sobreLienzo)
            {
                // Primero se guarda la figura pendiente (con su giro), para que
                // el relleno vea sus bordes definitivos.
                if (pendiente)
                    aplicarPendiente();

                rellenoSemilla(vertices, colCursor, filaCursor, filas, columnas, colorActual);
            }
        }
        else
        {
            poliPuntos.clear();   // si cambias de herramienta se descarta el polígono a medias

            // Inicio del arrastre.
            if (presionado && !arrastrando && sobreLienzo)
            {
                if (pendiente)
                    aplicarPendiente();

                arrastrando = true;
                colInicio = colCursor;
                filaInicio = filaCursor;

                libre.clear();
                Punto p = { (float)colCursor, (float)filaCursor };
                libre.push_back(p);
            }

            if (arrastrando)
            {
                if (herramienta == H_LIBRE)
                {
                    // Lápiz libre: se agrega un punto cada vez que el cursor cambia de celda.
                    // Entre puntos consecutivos se traza una línea de Bresenham,
                    // así no quedan huecos aunque muevas el mouse rápido.
                    Punto ultimo = libre.back();
                    if ((int)ultimo.x != colCursor || (int)ultimo.y != filaCursor)
                    {
                        Punto p = { (float)colCursor, (float)filaCursor };
                        libre.push_back(p);
                    }

                    formaActual.tipo = T_ABIERTA;
                    formaActual.pts = libre;
                }
                else
                {
                    formaActual = formaDesdeArrastre(herramienta,
                        colInicio, filaInicio, colCursor, filaCursor, lados);
                }

                // Al soltar el botón, la figura queda "pendiente" (todavía se puede girar).
                if (!presionado)
                {
                    formaPend = formaActual;
                    pendiente = true;
                    arrastrando = false;
                }
            }
        }

        presionadoAntes = presionado;
        derechoAntes = derecho;


        // --------------------------------------------------------------------
        // ARMAR LO QUE SE VA A MOSTRAR: lienzo + figura en curso
        // --------------------------------------------------------------------
        vista = vertices;

        if (arrastrando)
            dibujarForma(vista, formaActual, angulo, pivoteModo, filas, columnas, colorActual);
        else if (pendiente)
            dibujarForma(vista, formaPend, angulo, pivoteModo, filas, columnas, colorActual);

        // Vista previa del polígono por clics: sus lados + una línea hasta el cursor.
        if (herramienta == H_POLIGONO_CLICS && !poliPuntos.empty())
        {
            Forma previa;
            previa.tipo = T_ABIERTA;
            previa.pts = poliPuntos;
            Punto cursor = { (float)colCursor, (float)filaCursor };
            previa.pts.push_back(cursor);

            dibujarForma(vista, previa, 0.0f, 0, filas, columnas, colorActual);
        }


        // --------------------------------------------------------------------
        // SUBIR A LA GPU Y DIBUJAR
        // --------------------------------------------------------------------
        VBO1.Bind();
        glBufferSubData(GL_ARRAY_BUFFER, 0, vista.size() * sizeof(GLfloat), vista.data());
        VBO1.Unbind();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shaderProgram.Activate();
        VAO1.Bind();
        glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);

        // ImGui se dibuja al final para quedar encima de la grilla.
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }


    // ------------------------------------------------------------------------
    // LIBERAR RECURSOS
    // ------------------------------------------------------------------------
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    VAO1.Delete();
    VBO1.Delete();
    EBO1.Delete();
    shaderProgram.Delete();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}