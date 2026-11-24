#ifndef MATRIZ4_H
#define MATRIZ4_H

#include "Vetor.h"

// Matriz 4x4 em ordem de colunas (column-major), o mesmo layout esperado por
// glMultMatrixd/glLoadMatrixd. O elemento (linha l, coluna c) fica em m[c*4 + l].
struct Matriz4 {
    double m[16];

    Matriz4();  // identidade

    static Matriz4 identidade();
    static Matriz4 translacao(double x, double y, double z);
    static Matriz4 escala(double x, double y, double z);
    // eixo: 0 = X, 1 = Y, 2 = Z. Angulo em graus (regra da mao direita).
    static Matriz4 rotacao(int eixo, double graus);
    // plano: 0 = YZ (inverte X), 1 = XZ (inverte Y), 2 = XY (inverte Z).
    static Matriz4 reflexao(int plano);
    // Cisalhamento: coordenada[eixoAlvo] += fator * coordenada[eixoReferencia].
    static Matriz4 cisalhamento(int eixoAlvo, int eixoReferencia, double fator);

    double& operator()(int linha, int coluna) { return m[coluna * 4 + linha]; }
    double operator()(int linha, int coluna) const { return m[coluna * 4 + linha]; }

    Matriz4 operator*(const Matriz4& o) const;

    Vetor3 transformarPonto(const Vetor3& p) const;
    Vetor3 transformarDirecao(const Vetor3& d) const;

    // Determinante da parte linear 3x3 (sinal negativo = inverte orientacao).
    double determinante3x3() const;

    // Inversa-transposta da parte 3x3, usada para transformar normais.
    Matriz4 inversaTranspostaNormal() const;
};

#endif
