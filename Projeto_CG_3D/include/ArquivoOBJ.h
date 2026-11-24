#ifndef ARQUIVO_OBJ_H
#define ARQUIVO_OBJ_H

#include "Malha.h"
#include "Matriz4.h"

#include <string>

// Leitura de arquivos Wavefront .obj:
//  - "v" (vertices), "vt" (coordenadas de textura) e "f" (faces com qualquer
//    numero de lados, triangulados em leque; formatos v, v/vt, v//vn, v/vt/vn
//    e indices negativos);
//  - "mtllib": procura a textura difusa (map_Kd) do material;
//  - arquivos 2D do projeto original (apenas pontos de controle, sem faces)
//    sao convertidos em tubos 3D ao longo das curvas de Bezier.
// As normais "vn" do arquivo sao ignoradas: o programa calcula normais por
// face (flat) e normais medias por vertice (smooth).
//
// Em caso de erro retorna false e preenche "mensagem"; a malha nao e alterada.
bool carregarObj(const std::string& caminho, Malha& malha, std::string& mensagem);

// Salva a malha aplicando a matriz "transformacao" aos vertices.
bool salvarObj(const std::string& caminho,
               const Malha& malha,
               const Matriz4& transformacao,
               std::string& mensagem);

#endif
