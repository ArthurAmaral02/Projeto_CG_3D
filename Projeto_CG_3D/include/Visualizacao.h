#ifndef VISUALIZACAO_H
#define VISUALIZACAO_H

#include "Camera.h"
#include "Configuracao.h"
#include "CoordenadasTextura.h"
#include "Malha.h"
#include "Matriz4.h"
#include "Textura.h"

#include <cstddef>
#include <vector>

// Informacoes do ultimo quadro desenhado (usadas pelo autoteste e pela
// barra de status para comprovar o que foi efetivamente renderizado).
struct EstatisticasQuadro {
    int largura = 0;
    int altura = 0;
    double projecao[16] = {0};
    bool perspectiva = false;
    bool translucido = false;
    bool escritaProfundidadeDesligada = false;
    std::size_t triangulosOrdenados = 0;
    int modeloSombreamento = 0;  // GL_FLAT ou GL_SMOOTH
    bool texturaAtiva = false;
    bool reflexaoEsferica = false;
    int luzesLigadas = 0;
    float brilho = 0.0f;
};

// Renderizacao da cena 3D com OpenGL (pipeline fixo).
class Visualizacao {
public:
    // Configura o estado OpenGL e cria as texturas internas (piso e ambiente).
    void inicializar();

    // Prepara os arrays de vertices (posicao, normais e uv por canto).
    void definirMalha(const Malha& malha);
    void definirGeradorUV(const Malha& malha, GeradorUV gerador);
    GeradorUV geradorUVEmUso() const { return geradorAtual; }

    void redimensionar(int largura, int altura);

    void desenhar(const Configuracao& config,
                  const Camera& camera,
                  const Matriz4& transformacao,
                  const Textura& texturaObjeto,
                  const Textura& mapaAmbiente);

    // Mapa de ambiente padrao (estudio gerado proceduralmente).
    const Textura& mapaAmbientePadrao() const { return ambientePadrao; }

    const EstatisticasQuadro& estatisticas() const { return ultimoQuadro; }

    // Altura do piso (no espaco do objeto normalizado).
    float alturaPiso() const { return pisoY; }

private:
    void configurarLuzes(const Configuracao& config);
    void desenharPiso();
    void desenharEixos();
    void desenharMarcadoresLuz(const Configuracao& config);
    void desenharContornoOriginal();
    void desenharObjeto(const Configuracao& config,
                        const Textura& texturaObjeto,
                        const Textura& mapaAmbiente,
                        bool orientacaoInvertida);

    int larguraJanela = 1;
    int alturaJanela = 1;

    // Um elemento por canto de triangulo (3 por triangulo).
    std::vector<float> posicoes;        // xyz
    std::vector<float> normaisSuaves;   // xyz (media por vertice)
    std::vector<float> normaisPlanas;   // xyz (normal da face repetida)
    std::vector<float> coordenadasUV;   // uv
    std::vector<unsigned int> indicesOrdenados;

    Matriz4 normalizacao;  // centraliza e escala a malha para caber no raio 1
    float pisoY = -1.0f;
    GeradorUV geradorAtual = GeradorUV::Esferica;

    Textura texturaPiso;
    Textura ambientePadrao;

    EstatisticasQuadro ultimoQuadro;
};

#endif
