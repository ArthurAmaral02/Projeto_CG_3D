#ifndef CAMINHOS_H
#define CAMINHOS_H

#include <string>

// Utilitarios de caminhos de arquivo (independentes de plataforma).

bool arquivoExiste(const std::string& caminho);
bool diretorioExiste(const std::string& caminho);

std::string diretorioDe(const std::string& caminho);
std::string nomeArquivo(const std::string& caminho);
std::string juntarCaminho(const std::string& diretorio, const std::string& nome);
std::string caminhoAbsoluto(const std::string& caminho);
std::string diretorioAtual();

// Extensao em minusculas, com ponto (".obj"); vazio se nao houver.
std::string extensaoMinuscula(const std::string& caminho);
std::string trocarExtensao(const std::string& caminho, const std::string& novaExtensao);

// Filtros de tipo de arquivo usados pelos dialogos de abertura.
bool ehArquivoOBJ(const std::string& caminho);
bool ehImagemSuportada(const std::string& caminho);  // .png, .bmp, .jpg, .jpeg

// Pasta raiz do projeto (onde ficam modelos/ e texturas/). Procura a partir do
// diretorio atual e da pasta do executavel, subindo alguns niveis.
void definirExecutavel(const char* argv0);
std::string pastaDoProjeto();

// Resolve um caminho relativo contra a pasta do projeto.
std::string caminhoDoProjeto(const std::string& relativo);

#endif
