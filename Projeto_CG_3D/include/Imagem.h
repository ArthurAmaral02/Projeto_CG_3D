#ifndef IMAGEM_H
#define IMAGEM_H

#include <cstdint>
#include <string>
#include <vector>

// Imagem RGBA 8 bits em memoria (sem dependencia de OpenGL).
// A linha 0 e a linha de BAIXO da imagem (convencao do OpenGL, v = 0 embaixo).
struct Imagem {
    int largura = 0;
    int altura = 0;
    std::vector<std::uint8_t> rgba;

    bool vazia() const { return largura <= 0 || altura <= 0 || rgba.empty(); }
    std::uint8_t* pixel(int x, int y) { return &rgba[(static_cast<std::size_t>(y) * largura + x) * 4]; }
    const std::uint8_t* pixel(int x, int y) const { return &rgba[(static_cast<std::size_t>(y) * largura + x) * 4]; }
};

// Carrega .png, .bmp, .jpg/.jpeg (e outros formatos suportados pela stb_image).
bool carregarImagem(const std::string& caminho, Imagem& imagem, std::string& mensagem);

// Salva em PNG (usado nas capturas do autoteste).
bool salvarPNG(const std::string& caminho, const Imagem& imagem);

// Mapa de reflexao esferico (sphere map) de um estudio: ceu em degrade,
// janelas luminosas e piso de madeira, visto em uma esfera espelhada.
Imagem gerarMapaAmbienteEstudio(int tamanho);

// Textura quadriculada do piso (padrao de mesa/ladrilho).
Imagem gerarTexturaPiso(int tamanho);

#endif
