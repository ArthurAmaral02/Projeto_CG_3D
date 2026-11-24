#ifndef COORDENADAS_TEXTURA_H
#define COORDENADAS_TEXTURA_H

#include "Malha.h"

#include <vector>

// Origem das coordenadas de textura usadas na renderizacao.
enum class GeradorUV {
    ArquivoOBJ = 0,  // usa os "vt" do arquivo (quando existem)
    Esferica = 1,
    Cilindrica = 2,
    Planar = 3
};

const char* nomeGeradorUV(GeradorUV gerador);

// Gera uma coordenada (u, v) por canto de triangulo (3 por triangulo).
// Se o gerador for ArquivoOBJ e a malha nao tiver vt, usa a projecao esferica.
// "usado" (opcional) recebe o gerador efetivamente aplicado.
std::vector<Vetor2> gerarCoordenadasTextura(const Malha& malha,
                                            GeradorUV gerador,
                                            GeradorUV* usado = nullptr);

#endif
