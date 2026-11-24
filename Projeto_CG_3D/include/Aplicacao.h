#ifndef APLICACAO_H
#define APLICACAO_H

#include "Camera.h"
#include "Configuracao.h"
#include "Malha.h"
#include "Textura.h"
#include "Transformacoes.h"
#include "Visualizacao.h"

#include <string>

// Modelo carregado na inicializacao (malha principal com >= 1600 triangulos).
extern const char* const MODELO_PADRAO;
extern const char* const TEXTURA_PADRAO;

// Estado global do visualizador, compartilhado pelos callbacks da GLUT e da GLUI.
class Aplicacao {
public:
    int janela = 0;  // identificador da janela GLUT de visualizacao

    Configuracao config;
    Camera camera;
    PilhaTransformacoes transformacoes;
    Visualizacao visualizacao;

    Malha malha;
    std::string caminhoModelo;

    Textura texturaObjeto;
    Textura mapaAmbiente;  // vazio = usa o mapa padrao da Visualizacao

    // Inicializa OpenGL e carrega modelo/textura padrao. Deve ser chamada
    // depois de criar a janela.
    void inicializar();

    bool carregarModelo(const std::string& caminho);
    bool carregarTextura(const std::string& caminho);
    bool carregarMapaAmbiente(const std::string& caminho);
    bool salvarModelo(const std::string& caminho);

    void aplicarGeradorUV();  // chamado quando o seletor de coordenadas muda
    void resetarTransformacoes();

    void desenhar();
    void redimensionar(int largura, int altura);
    void solicitarRedesenho() const;

    // Ultima mensagem de status (exibida na interface e no terminal).
    const std::string& mensagem() const { return ultimaMensagem; }
    void informar(const std::string& texto);

    // Notificado quando a mensagem ou o modelo mudam (atualiza a GLUI).
    void (*aoAtualizarInterface)() = nullptr;

private:
    std::string ultimaMensagem;
};

Aplicacao& aplicacao();

#endif
