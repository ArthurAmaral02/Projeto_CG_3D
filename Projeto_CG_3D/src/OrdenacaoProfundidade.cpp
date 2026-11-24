#include "OrdenacaoProfundidade.h"

#include <algorithm>
#include <numeric>

void ordenarTriangulosPorProfundidade(const std::vector<float>& posicoesCantos,
                                      const double modelView[16],
                                      std::vector<unsigned int>& indices)
{
    const std::size_t quantidade = posicoesCantos.size() / 9;

    // Profundidade = z do centroide no espaco da camera. A camera olha para
    // -z, entao quanto mais negativo, mais distante.
    std::vector<float> profundidade(quantidade);

    for (std::size_t t = 0; t < quantidade; ++t) {
        const float* p = &posicoesCantos[t * 9];
        const double cx = (p[0] + p[3] + p[6]) / 3.0;
        const double cy = (p[1] + p[4] + p[7]) / 3.0;
        const double cz = (p[2] + p[5] + p[8]) / 3.0;

        profundidade[t] = static_cast<float>(
            modelView[2] * cx + modelView[6] * cy + modelView[10] * cz + modelView[14]);
    }

    std::vector<unsigned int> ordem(quantidade);
    std::iota(ordem.begin(), ordem.end(), 0u);

    std::sort(ordem.begin(), ordem.end(), [&](unsigned int a, unsigned int b) {
        return profundidade[a] < profundidade[b];  // mais distante primeiro
    });

    indices.resize(quantidade * 3);

    for (std::size_t i = 0; i < quantidade; ++i) {
        indices[i * 3 + 0] = ordem[i] * 3 + 0;
        indices[i * 3 + 1] = ordem[i] * 3 + 1;
        indices[i * 3 + 2] = ordem[i] * 3 + 2;
    }
}
