#include "Malha.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <unordered_map>

namespace {

// Chave de posicao quantizada: vertices duplicados na mesma posicao (comuns
// em costuras de UV) compartilham a mesma normal media.
struct ChavePosicao {
    std::int64_t x, y, z;
    bool operator==(const ChavePosicao& o) const { return x == o.x && y == o.y && z == o.z; }
};

struct HashPosicao {
    std::size_t operator()(const ChavePosicao& k) const {
        std::size_t h = static_cast<std::size_t>(k.x * 73856093LL);
        h ^= static_cast<std::size_t>(k.y * 19349663LL) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= static_cast<std::size_t>(k.z * 83492791LL) + 0x9e3779b9 + (h << 6) + (h >> 2);
        return h;
    }
};

}

void Malha::limpar()
{
    nome.clear();
    arquivoTextura.clear();
    vertices.clear();
    coordTextura.clear();
    triangulos.clear();
    normaisFace.clear();
    normaisVertice.clear();
    minimo = maximo = Vetor3();
}

bool Malha::possuiCoordTextura() const
{
    if (coordTextura.empty() || triangulos.empty()) {
        return false;
    }

    for (const Triangulo& t : triangulos) {
        for (int i = 0; i < 3; ++i) {
            if (t.vt[i] < 0) {
                return false;
            }
        }
    }

    return true;
}

void Malha::atualizarDerivados()
{
    calcularLimites();
    calcularNormais();
}

void Malha::calcularLimites()
{
    if (vertices.empty()) {
        minimo = maximo = Vetor3();
        return;
    }

    minimo = maximo = vertices[0];

    for (const Vetor3& p : vertices) {
        minimo.x = std::min(minimo.x, p.x);
        minimo.y = std::min(minimo.y, p.y);
        minimo.z = std::min(minimo.z, p.z);
        maximo.x = std::max(maximo.x, p.x);
        maximo.y = std::max(maximo.y, p.y);
        maximo.z = std::max(maximo.z, p.z);
    }
}

void Malha::calcularNormais()
{
    normaisFace.assign(triangulos.size(), Vetor3());
    normaisVertice.assign(vertices.size(), Vetor3());

    // Agrupa vertices pela posicao para a media nao "rachar" nas costuras.
    const float tamanho = std::max(raio(), 1e-6f);
    const double quantum = tamanho * 1e-5;

    std::unordered_map<ChavePosicao, int, HashPosicao> grupos;
    std::vector<int> grupoDoVertice(vertices.size());

    for (std::size_t i = 0; i < vertices.size(); ++i) {
        const ChavePosicao chave{
            static_cast<std::int64_t>(std::llround(vertices[i].x / quantum)),
            static_cast<std::int64_t>(std::llround(vertices[i].y / quantum)),
            static_cast<std::int64_t>(std::llround(vertices[i].z / quantum))
        };
        auto it = grupos.emplace(chave, static_cast<int>(grupos.size())).first;
        grupoDoVertice[i] = it->second;
    }

    std::vector<Vetor3> somaPorGrupo(grupos.size());

    for (std::size_t f = 0; f < triangulos.size(); ++f) {
        const Triangulo& t = triangulos[f];
        const Vetor3 p[3] = {vertices[t.v[0]], vertices[t.v[1]], vertices[t.v[2]]};

        const Vetor3 n = produtoVetorial(p[1] - p[0], p[2] - p[0]).normalizado();
        normaisFace[f] = n;

        // Media ponderada pelo angulo do triangulo em cada canto: o resultado
        // nao depende de como os poligonos foram triangulados.
        for (int k = 0; k < 3; ++k) {
            const Vetor3 e1 = (p[(k + 1) % 3] - p[k]).normalizado();
            const Vetor3 e2 = (p[(k + 2) % 3] - p[k]).normalizado();
            const float angulo = std::acos(std::clamp(produtoEscalar(e1, e2), -1.0f, 1.0f));
            somaPorGrupo[grupoDoVertice[t.v[k]]] += n * angulo;
        }
    }

    for (std::size_t i = 0; i < vertices.size(); ++i) {
        normaisVertice[i] = somaPorGrupo[grupoDoVertice[i]].normalizado();
    }
}

Vetor3 Malha::centro() const
{
    return (minimo + maximo) * 0.5f;
}

float Malha::raio() const
{
    return (maximo - minimo).comprimento() * 0.5f;
}
