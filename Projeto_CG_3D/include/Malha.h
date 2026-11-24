#ifndef MALHA_H
#define MALHA_H

#include "Vetor.h"

#include <string>
#include <vector>

// Triangulo indexado. vt[i] = -1 quando o canto nao tem coordenada de textura.
struct Triangulo {
    int v[3] = {0, 0, 0};
    int vt[3] = {-1, -1, -1};
};

// Malha triangular 3D carregada de um arquivo .obj.
class Malha {
public:
    std::string nome;
    std::string arquivoTextura;  // textura indicada pelo .mtl (map_Kd), se houver

    std::vector<Vetor3> vertices;
    std::vector<Vetor2> coordTextura;
    std::vector<Triangulo> triangulos;

    // Dados derivados (preenchidos por atualizarDerivados()).
    std::vector<Vetor3> normaisFace;     // uma por triangulo (flat shading)
    std::vector<Vetor3> normaisVertice;  // media das faces vizinhas (smooth shading)
    Vetor3 minimo;
    Vetor3 maximo;

    void limpar();
    bool vazia() const { return triangulos.empty(); }

    // true se todos os cantos de todos os triangulos possuem vt.
    bool possuiCoordTextura() const;

    // Calcula limites, normais por face e normais medias por vertice.
    void atualizarDerivados();

    Vetor3 centro() const;
    // Raio da esfera envolvente centrada no centro da caixa.
    float raio() const;

private:
    void calcularLimites();
    void calcularNormais();
};

#endif
