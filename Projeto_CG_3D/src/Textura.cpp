#include "Textura.h"

#include "Caminhos.h"

bool Textura::carregarArquivo(const std::string& caminho, std::string& mensagem)
{
    Imagem imagem;

    if (!carregarImagem(caminho, imagem, mensagem)) {
        return false;
    }

    // A opacidade do objeto e definida pelo slider (alfa do material). O alfa
    // de imagens de cor costuma ser mascara/lixo de exportacao (ex.: spot.png),
    // entao a textura e tratada como opaca para nao "furar" a malha.
    for (std::size_t i = 3; i < imagem.rgba.size(); i += 4) {
        imagem.rgba[i] = 255;
    }

    if (!criar(imagem, nomeArquivo(caminho))) {
        mensagem = "Falha ao criar textura OpenGL para " + caminho;
        return false;
    }

    return true;
}

bool Textura::criar(const Imagem& imagem, const std::string& descricao)
{
    if (imagem.vazia()) {
        return false;
    }

    GLuint novo = 0;
    glGenTextures(1, &novo);

    if (novo == 0) {
        return false;
    }

    glBindTexture(GL_TEXTURE_2D, novo);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    // gluBuild2DMipmaps tambem redimensiona imagens que nao sao potencia de 2.
    const int erro = gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, imagem.largura, imagem.altura,
                                       GL_RGBA, GL_UNSIGNED_BYTE, imagem.rgba.data());
    glBindTexture(GL_TEXTURE_2D, 0);

    if (erro != 0) {
        glDeleteTextures(1, &novo);
        return false;
    }

    liberar();
    id = novo;
    origem = descricao;
    return true;
}

void Textura::liberar()
{
    if (id != 0) {
        glDeleteTextures(1, &id);
        id = 0;
    }
    origem.clear();
}
