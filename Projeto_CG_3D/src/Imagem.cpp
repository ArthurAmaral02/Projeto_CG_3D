#include "Imagem.h"

#include <algorithm>
#include <cmath>
#include <cstring>

#if defined(_MSC_VER)
#pragma warning(push, 0)
#endif
#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

namespace {

struct Cor {
    float r, g, b;
};

Cor misturar(const Cor& a, const Cor& b, float t)
{
    t = std::clamp(t, 0.0f, 1.0f);
    return {a.r + (b.r - a.r) * t, a.g + (b.g - a.g) * t, a.b + (b.b - a.b) * t};
}

float suavizar(float borda0, float borda1, float x)
{
    const float t = std::clamp((x - borda0) / (borda1 - borda0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

// Ruido pseudoaleatorio deterministico em [0, 1).
float ruido(int a, int b)
{
    unsigned int h = static_cast<unsigned int>(a) * 374761393u + static_cast<unsigned int>(b) * 668265263u;
    h = (h ^ (h >> 13)) * 1274126177u;
    return static_cast<float>((h ^ (h >> 16)) & 0xFFFFu) / 65536.0f;
}

// Cor de madeira em coordenadas do plano (tabuas ao longo de x).
Cor madeira(float x, float z)
{
    const float larguraTabua = 0.5f;
    const int tabua = static_cast<int>(std::floor(z / larguraTabua));
    const float variacao = ruido(tabua, 7);
    const float veio = 0.5f + 0.5f * std::sin(x * 9.0f + std::sin(z * 13.0f + tabua) * 2.0f + variacao * 20.0f);
    const float fresta = suavizar(0.0f, 0.03f, std::fabs(z - (tabua + 0.5f) * larguraTabua) < larguraTabua * 0.47f ? 1.0f : 0.0f);

    Cor base = misturar({0.42f, 0.25f, 0.13f}, {0.62f, 0.40f, 0.22f}, variacao);
    base = misturar(base, {base.r * 0.75f, base.g * 0.72f, base.b * 0.70f}, veio * 0.6f);
    return misturar({0.18f, 0.10f, 0.05f}, base, fresta);
}

// Cor do ambiente vista na direcao "d" (normalizada).
Cor ambienteEstudio(float dx, float dy, float dz)
{
    const Cor horizonte = {0.86f, 0.86f, 0.84f};
    const Cor zenite = {0.32f, 0.40f, 0.55f};

    Cor cor;

    if (dy >= 0.0f) {
        cor = misturar(horizonte, zenite, std::pow(dy, 0.6f));

        // Duas "janelas" (softboxes) que geram os reflexos brilhantes.
        const float janela1 = suavizar(0.90f, 0.96f, (dx * 0.55f + dy * 0.45f - dz * 0.70f) / 1.0f);
        const float janela2 = suavizar(0.93f, 0.97f, (-dx * 0.75f + dy * 0.60f + dz * 0.28f) / 1.0f);
        cor = misturar(cor, {1.0f, 1.0f, 0.97f}, std::max(janela1, janela2));
    } else {
        // Piso de madeira: intersecao do raio com o plano y = -1.
        const float t = -1.0f / dy;
        const Cor piso = madeira(dx * t, dz * t);
        // Perto do horizonte o piso some na penumbra.
        cor = misturar(piso, {0.30f, 0.25f, 0.22f}, std::exp(dy * 6.0f));
    }

    return cor;
}

std::uint8_t paraByte(float v)
{
    return static_cast<std::uint8_t>(std::clamp(v, 0.0f, 1.0f) * 255.0f + 0.5f);
}

}

bool carregarImagem(const std::string& caminho, Imagem& imagem, std::string& mensagem)
{
    int largura = 0, altura = 0, canais = 0;

    // OpenGL espera a primeira linha embaixo.
    stbi_set_flip_vertically_on_load(1);
    unsigned char* dados = stbi_load(caminho.c_str(), &largura, &altura, &canais, 4);

    if (dados == nullptr) {
        mensagem = "Falha ao carregar imagem " + caminho + ": " + stbi_failure_reason();
        return false;
    }

    imagem.largura = largura;
    imagem.altura = altura;
    imagem.rgba.assign(dados, dados + static_cast<std::size_t>(largura) * altura * 4);
    stbi_image_free(dados);

    mensagem = "Imagem carregada: " + caminho + " (" + std::to_string(largura) + "x" +
               std::to_string(altura) + ")";
    return true;
}

bool salvarPNG(const std::string& caminho, const Imagem& imagem)
{
    if (imagem.vazia()) {
        return false;
    }

    stbi_flip_vertically_on_write(1);
    return stbi_write_png(caminho.c_str(), imagem.largura, imagem.altura, 4,
                          imagem.rgba.data(), imagem.largura * 4) != 0;
}

Imagem gerarMapaAmbienteEstudio(int tamanho)
{
    Imagem img;
    img.largura = img.altura = tamanho;
    img.rgba.resize(static_cast<std::size_t>(tamanho) * tamanho * 4);

    for (int y = 0; y < tamanho; ++y) {
        for (int x = 0; x < tamanho; ++x) {
            // Coordenadas do sphere map em [-1, 1].
            float s = (x + 0.5f) / tamanho * 2.0f - 1.0f;
            float t = (y + 0.5f) / tamanho * 2.0f - 1.0f;
            const float r2 = s * s + t * t;

            if (r2 > 1.0f) {
                const float r = std::sqrt(r2);
                s /= r;
                t /= r;
            }

            // Normal da esfera espelhada e reflexao do raio de visao (0, 0, -1).
            const float nz = std::sqrt(std::max(0.0f, 1.0f - s * s - t * t));
            const float rx = 2.0f * nz * s;
            const float ry = 2.0f * nz * t;
            const float rz = 2.0f * nz * nz - 1.0f;

            const Cor c = ambienteEstudio(rx, ry, rz);
            std::uint8_t* p = img.pixel(x, y);
            p[0] = paraByte(c.r);
            p[1] = paraByte(c.g);
            p[2] = paraByte(c.b);
            p[3] = 255;
        }
    }

    return img;
}

Imagem gerarTexturaPiso(int tamanho)
{
    Imagem img;
    img.largura = img.altura = tamanho;
    img.rgba.resize(static_cast<std::size_t>(tamanho) * tamanho * 4);

    for (int y = 0; y < tamanho; ++y) {
        for (int x = 0; x < tamanho; ++x) {
            // 4 tabuas por repeticao da textura.
            const Cor c = madeira(x * 2.0f / tamanho * 3.14159f, y * 2.0f / tamanho);
            std::uint8_t* p = img.pixel(x, y);
            p[0] = paraByte(c.r);
            p[1] = paraByte(c.g);
            p[2] = paraByte(c.b);
            p[3] = 255;
        }
    }

    return img;
}
