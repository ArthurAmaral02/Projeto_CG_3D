#ifndef BEZIER_H
#define BEZIER_H

#include "Malha.h"
#include "Ponto.h"

#include <vector>

// Curvas de Bezier cubicas do projeto 2D original. Na versao 3D os desenhos
// antigos (arquivos .obj so com pontos de controle "v x y") sao convertidos
// em tubos 3D que seguem as curvas, preservando os desenhos do projeto.

Ponto bezierCubica(
    const Ponto& p0,
    const Ponto& p1,
    const Ponto& p2,
    const Ponto& p3,
    double t
);

// Recebe os pontos de controle agrupados como no arquivo (por "o", "g" ou
// comentario) e devolve as curvas compostas a desenhar. Se todos os grupos
// tiverem 3n+1 pontos, cada grupo e uma curva independente; senao todos os
// pontos formam uma unica curva encadeada (comportamento do projeto 2D).
std::vector<std::vector<Ponto>> organizarCurvas(
    const std::vector<std::vector<Ponto>>& grupos
);

// Amostra uma curva composta (trechos cubicos que compartilham extremidades).
std::vector<Ponto> amostrarCurva(const std::vector<Ponto>& controle, int amostrasPorTrecho);

// Gera uma malha 3D de tubos (com tampas e coordenadas de textura) ao longo
// das curvas. O raio e relativo a diagonal do desenho.
void gerarMalhaTubular(
    const std::vector<std::vector<Ponto>>& curvas,
    Malha& saida,
    int amostrasPorTrecho = 12,
    int lados = 10,
    float raioRelativo = 0.009f
);

#endif
