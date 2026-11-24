#ifndef ORDENACAO_PROFUNDIDADE_H
#define ORDENACAO_PROFUNDIDADE_H

#include <vector>

// Ordenacao de triangulos de tras para frente (algoritmo do pintor), usada
// no desenho translucido.
//
// "posicoesCantos": 9 floats por triangulo (x, y, z dos 3 cantos).
// "modelView": matriz 4x4 column-major (glGetDoublev(GL_MODELVIEW_MATRIX)).
// "indices": recebe os indices dos cantos (3 por triangulo) ordenados do
// triangulo mais distante da camera para o mais proximo.
void ordenarTriangulosPorProfundidade(const std::vector<float>& posicoesCantos,
                                      const double modelView[16],
                                      std::vector<unsigned int>& indices);

#endif
