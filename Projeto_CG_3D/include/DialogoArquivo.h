#ifndef DIALOGO_ARQUIVO_H
#define DIALOGO_ARQUIVO_H

#include <string>

// Selecao de arquivos pela interface grafica (sem linha de comando).

enum class TipoArquivo {
    ModeloOBJ,  // .obj
    Imagem      // .png, .bmp, .jpg, .jpeg
};

using RetornoArquivo = void (*)(const std::string& caminhoAbsoluto);

// Abre a janela "Abrir arquivo" da GLUI (GLUI_FileBrowser): o usuario navega
// pelas pastas com duplo clique e escolhe o arquivo. Funciona em Windows e Linux.
void abrirNavegadorGLUI(TipoArquivo tipo, RetornoArquivo retorno);

// Caixa de dialogo nativa "Abrir" do Windows. Retorna false se o usuario
// cancelar ou se a plataforma nao for Windows.
bool dialogoNativoDisponivel();
bool abrirDialogoNativo(TipoArquivo tipo, std::string& caminhoEscolhido);

bool extensaoAceita(TipoArquivo tipo, const std::string& caminho);

#endif
