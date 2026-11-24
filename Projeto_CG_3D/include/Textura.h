#ifndef TEXTURA_H
#define TEXTURA_H

#include "Imagem.h"
#include "OpenGL.h"

#include <string>

// Textura 2D do OpenGL (com mipmaps e repeticao).
class Textura {
public:
    // A textura so e liberada explicitamente (liberar() ou ao ser substituida),
    // pois no encerramento do programa o contexto OpenGL ja pode nao existir.
    Textura() = default;
    Textura(const Textura&) = delete;
    Textura& operator=(const Textura&) = delete;

    // Carrega .png/.bmp/.jpg do disco. Mantem a textura anterior em caso de erro.
    bool carregarArquivo(const std::string& caminho, std::string& mensagem);
    bool criar(const Imagem& imagem, const std::string& descricao);
    void liberar();

    bool valida() const { return id != 0; }
    GLuint identificador() const { return id; }
    const std::string& descricao() const { return origem; }

private:
    GLuint id = 0;
    std::string origem;
};

#endif
