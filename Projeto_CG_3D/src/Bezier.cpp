#include "Bezier.h"

#include <algorithm>
#include <cmath>

namespace {
const float PI = 3.14159265358979f;
}

Ponto bezierCubica(
    const Ponto& p0,
    const Ponto& p1,
    const Ponto& p2,
    const Ponto& p3,
    double t
) {
    const double u = 1.0 - t;

    const double b0 = u * u * u;
    const double b1 = 3.0 * u * u * t;
    const double b2 = 3.0 * u * t * t;
    const double b3 = t * t * t;

    Ponto resultado;

    resultado.x = b0 * p0.x + b1 * p1.x + b2 * p2.x + b3 * p3.x;
    resultado.y = b0 * p0.y + b1 * p1.y + b2 * p2.y + b3 * p3.y;

    return resultado;
}

std::vector<std::vector<Ponto>> organizarCurvas(const std::vector<std::vector<Ponto>>& grupos)
{
    std::vector<std::vector<Ponto>> naoVazios;

    for (const auto& g : grupos) {
        if (!g.empty()) {
            naoVazios.push_back(g);
        }
    }

    const bool gruposIndependentes =
        !naoVazios.empty() &&
        std::all_of(naoVazios.begin(), naoVazios.end(), [](const std::vector<Ponto>& g) {
            return g.size() >= 4 && g.size() % 3 == 1;
        });

    if (gruposIndependentes) {
        return naoVazios;
    }

    // Uma unica curva encadeada, como no visualizador 2D original
    // (pontos que sobram depois do ultimo trecho completo sao ignorados).
    std::vector<Ponto> todos;
    for (const auto& g : naoVazios) {
        todos.insert(todos.end(), g.begin(), g.end());
    }

    if (todos.size() < 4) {
        return {};
    }

    todos.resize(((todos.size() - 1) / 3) * 3 + 1);

    return {todos};
}

std::vector<Ponto> amostrarCurva(const std::vector<Ponto>& controle, int amostrasPorTrecho)
{
    std::vector<Ponto> pontos;
    amostrasPorTrecho = std::max(amostrasPorTrecho, 1);

    for (std::size_t i = 0; i + 3 < controle.size(); i += 3) {
        // O primeiro ponto de cada trecho e o ultimo do anterior: so entra uma vez.
        for (int k = (i == 0 ? 0 : 1); k <= amostrasPorTrecho; ++k) {
            const double t = static_cast<double>(k) / amostrasPorTrecho;
            const Ponto p = bezierCubica(controle[i], controle[i + 1], controle[i + 2], controle[i + 3], t);

            // Descarta pontos repetidos (trechos degenerados).
            if (!pontos.empty() &&
                std::fabs(pontos.back().x - p.x) < 1e-9 &&
                std::fabs(pontos.back().y - p.y) < 1e-9) {
                continue;
            }

            pontos.push_back(p);
        }
    }

    return pontos;
}

void gerarMalhaTubular(
    const std::vector<std::vector<Ponto>>& curvas,
    Malha& saida,
    int amostrasPorTrecho,
    int lados,
    float raioRelativo
) {
    saida.vertices.clear();
    saida.coordTextura.clear();
    saida.triangulos.clear();

    lados = std::max(lados, 3);

    // Raio do tubo proporcional ao tamanho do desenho.
    double minX = 1e300, minY = 1e300, maxX = -1e300, maxY = -1e300;
    for (const auto& c : curvas) {
        for (const Ponto& p : c) {
            minX = std::min(minX, p.x); maxX = std::max(maxX, p.x);
            minY = std::min(minY, p.y); maxY = std::max(maxY, p.y);
        }
    }
    const float diagonal = static_cast<float>(std::hypot(maxX - minX, maxY - minY));
    const float raio = std::max(diagonal * raioRelativo, 1e-4f);

    // vt de indice 0: usado pelos cantos das tampas.
    saida.coordTextura.push_back({0.0f, 0.0f});

    for (const auto& controle : curvas) {
        const std::vector<Ponto> pts = amostrarCurva(controle, amostrasPorTrecho);

        if (pts.size() < 2) {
            continue;
        }

        const int primeiroAnel = static_cast<int>(saida.vertices.size());
        float comprimento = 0.0f;
        std::vector<int> indiceU;  // indice do vt de cada anel/lado

        for (std::size_t i = 0; i < pts.size(); ++i) {
            const Ponto& anterior = pts[i == 0 ? 0 : i - 1];
            const Ponto& proximo = pts[std::min(i + 1, pts.size() - 1)];

            if (i > 0) {
                comprimento += static_cast<float>(std::hypot(pts[i].x - pts[i - 1].x, pts[i].y - pts[i - 1].y));
            }

            // Referencial do anel: T (tangente), N (no plano XY) e B = +Z.
            Vetor3 tangente(static_cast<float>(proximo.x - anterior.x),
                            static_cast<float>(proximo.y - anterior.y), 0.0f);
            tangente = tangente.normalizado();
            const Vetor3 normal(-tangente.y, tangente.x, 0.0f);
            const Vetor3 binormal(0.0f, 0.0f, 1.0f);
            const Vetor3 centro(static_cast<float>(pts[i].x), static_cast<float>(pts[i].y), 0.0f);

            for (int j = 0; j < lados; ++j) {
                const float ang = 2.0f * PI * j / lados;
                saida.vertices.push_back(centro + normal * (raio * std::cos(ang)) + binormal * (raio * std::sin(ang)));
            }

            // u acompanha o comprimento do tubo (textura repetida, sem esticar).
            const float u = comprimento / (2.0f * PI * raio);
            const int base = static_cast<int>(saida.coordTextura.size());
            for (int j = 0; j <= lados; ++j) {
                saida.coordTextura.push_back({u, static_cast<float>(j) / lados});
            }
            indiceU.push_back(base);
        }

        const int aneis = static_cast<int>(pts.size());

        for (int i = 0; i + 1 < aneis; ++i) {
            for (int j = 0; j < lados; ++j) {
                const int jn = (j + 1) % lados;
                const int a = primeiroAnel + i * lados + j;
                const int b = primeiroAnel + (i + 1) * lados + j;
                const int c = primeiroAnel + (i + 1) * lados + jn;
                const int d = primeiroAnel + i * lados + jn;
                const int ta = indiceU[i] + j, tb = indiceU[i + 1] + j;
                const int tc = indiceU[i + 1] + j + 1, td = indiceU[i] + j + 1;

                // Ordem anti-horaria vista de fora do tubo (normal para fora).
                Triangulo t1;
                t1.v[0] = a; t1.v[1] = c; t1.v[2] = b;
                t1.vt[0] = ta; t1.vt[1] = tc; t1.vt[2] = tb;
                Triangulo t2;
                t2.v[0] = a; t2.v[1] = d; t2.v[2] = c;
                t2.vt[0] = ta; t2.vt[1] = td; t2.vt[2] = tc;
                saida.triangulos.push_back(t1);
                saida.triangulos.push_back(t2);
            }
        }

        // Tampas nas extremidades do tubo.
        for (int extremidade = 0; extremidade < 2; ++extremidade) {
            const int anel = (extremidade == 0) ? 0 : aneis - 1;
            const Ponto& p = pts[anel];
            const int centro = static_cast<int>(saida.vertices.size());
            saida.vertices.push_back(Vetor3(static_cast<float>(p.x), static_cast<float>(p.y), 0.0f));

            for (int j = 0; j < lados; ++j) {
                const int jn = (j + 1) % lados;
                Triangulo t;
                t.v[0] = centro;
                t.v[1] = primeiroAnel + anel * lados + (extremidade == 0 ? jn : j);
                t.v[2] = primeiroAnel + anel * lados + (extremidade == 0 ? j : jn);
                t.vt[0] = t.vt[1] = t.vt[2] = 0;
                saida.triangulos.push_back(t);
            }
        }
    }
}
