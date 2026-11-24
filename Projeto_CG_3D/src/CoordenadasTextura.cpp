#include "CoordenadasTextura.h"

#include <algorithm>
#include <cmath>

namespace {

const float PI = 3.14159265358979f;

// Angulo em torno do eixo Y, mapeado para [0, 1). A frente (+Z) fica em u = 0.5.
float anguloAzimutal(const Vetor3& p)
{
    return 0.5f + std::atan2(p.x, p.z) / (2.0f * PI);
}

bool sobreEixoY(const Vetor3& p, float tolerancia)
{
    return std::fabs(p.x) < tolerancia && std::fabs(p.z) < tolerancia;
}

// Corrige a costura do mapeamento angular: se um triangulo cruza a linha
// u = 0 / u = 1, os cantos do lado esquerdo sao deslocados em +1 (com
// GL_REPEAT a textura continua sem "rasgar" o triangulo inteiro).
// Cantos exatamente sobre o eixo (polos) recebem a media dos outros cantos.
void corrigirCosturaAngular(Vetor2 uv[3], const bool noEixo[3])
{
    float minU = 2.0f;
    float maxU = -1.0f;

    for (int i = 0; i < 3; ++i) {
        if (!noEixo[i]) {
            minU = std::min(minU, uv[i].u);
            maxU = std::max(maxU, uv[i].u);
        }
    }

    if (maxU - minU > 0.5f) {
        for (int i = 0; i < 3; ++i) {
            if (!noEixo[i] && uv[i].u < 0.5f) {
                uv[i].u += 1.0f;
            }
        }
    }

    float soma = 0.0f;
    int quantidade = 0;

    for (int i = 0; i < 3; ++i) {
        if (!noEixo[i]) {
            soma += uv[i].u;
            ++quantidade;
        }
    }

    for (int i = 0; i < 3; ++i) {
        if (noEixo[i]) {
            uv[i].u = quantidade ? soma / quantidade : 0.5f;
        }
    }
}

}

const char* nomeGeradorUV(GeradorUV gerador)
{
    switch (gerador) {
        case GeradorUV::ArquivoOBJ: return "OBJ (vt)";
        case GeradorUV::Esferica:   return "esferica";
        case GeradorUV::Cilindrica: return "cilindrica";
        case GeradorUV::Planar:     return "planar";
    }
    return "?";
}

std::vector<Vetor2> gerarCoordenadasTextura(const Malha& malha,
                                            GeradorUV gerador,
                                            GeradorUV* usado)
{
    if (gerador == GeradorUV::ArquivoOBJ && !malha.possuiCoordTextura()) {
        gerador = GeradorUV::Esferica;
    }

    if (usado != nullptr) {
        *usado = gerador;
    }

    std::vector<Vetor2> resultado(malha.triangulos.size() * 3);

    const Vetor3 centro = malha.centro();
    const Vetor3 tamanho = malha.maximo - malha.minimo;
    const float dx = std::max(tamanho.x, 1e-6f);
    const float dy = std::max(tamanho.y, 1e-6f);
    const float tolerancia = std::max(malha.raio(), 1e-6f) * 1e-5f;

    for (std::size_t f = 0; f < malha.triangulos.size(); ++f) {
        const Triangulo& t = malha.triangulos[f];
        Vetor2 uv[3];
        bool noEixo[3] = {false, false, false};

        for (int k = 0; k < 3; ++k) {
            const Vetor3& original = malha.vertices[t.v[k]];
            const Vetor3 p = original - centro;

            switch (gerador) {
                case GeradorUV::ArquivoOBJ:
                    uv[k] = malha.coordTextura[t.vt[k]];
                    break;

                case GeradorUV::Esferica: {
                    const float r = std::max(p.comprimento(), 1e-12f);
                    const float latitude = std::asin(std::clamp(p.y / r, -1.0f, 1.0f));
                    uv[k].u = anguloAzimutal(p);
                    uv[k].v = 0.5f + latitude / PI;
                    noEixo[k] = sobreEixoY(p, tolerancia);
                    break;
                }

                case GeradorUV::Cilindrica:
                    uv[k].u = anguloAzimutal(p);
                    uv[k].v = (original.y - malha.minimo.y) / dy;
                    noEixo[k] = sobreEixoY(p, tolerancia);
                    break;

                case GeradorUV::Planar:
                    // Projecao ortogonal no plano XY (vista frontal).
                    uv[k].u = (original.x - malha.minimo.x) / dx;
                    uv[k].v = (original.y - malha.minimo.y) / dy;
                    break;
            }
        }

        if (gerador == GeradorUV::Esferica || gerador == GeradorUV::Cilindrica) {
            corrigirCosturaAngular(uv, noEixo);
        }

        for (int k = 0; k < 3; ++k) {
            resultado[f * 3 + k] = uv[k];
        }
    }

    return resultado;
}
