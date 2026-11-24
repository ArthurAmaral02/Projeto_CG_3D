#include "Caminhos.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <system_error>
#include <vector>

namespace fs = std::filesystem;

namespace {
std::string pastaExecutavel;
std::string pastaProjetoCache;

bool pareceRaizDoProjeto(const fs::path& p)
{
    std::error_code ec;
    return fs::is_directory(p / "modelos", ec) && fs::is_directory(p / "texturas", ec);
}
}

bool arquivoExiste(const std::string& caminho)
{
    std::error_code ec;
    return !caminho.empty() && fs::is_regular_file(fs::path(caminho), ec);
}

bool diretorioExiste(const std::string& caminho)
{
    std::error_code ec;
    return !caminho.empty() && fs::is_directory(fs::path(caminho), ec);
}

std::string diretorioDe(const std::string& caminho)
{
    return fs::path(caminho).parent_path().string();
}

std::string nomeArquivo(const std::string& caminho)
{
    return fs::path(caminho).filename().string();
}

std::string juntarCaminho(const std::string& diretorio, const std::string& nome)
{
    if (diretorio.empty()) {
        return nome;
    }
    return (fs::path(diretorio) / fs::path(nome)).string();
}

std::string caminhoAbsoluto(const std::string& caminho)
{
    std::error_code ec;
    const fs::path p = fs::absolute(fs::path(caminho), ec);
    return ec ? caminho : p.lexically_normal().string();
}

std::string diretorioAtual()
{
    std::error_code ec;
    const fs::path p = fs::current_path(ec);
    return ec ? std::string(".") : p.string();
}

std::string extensaoMinuscula(const std::string& caminho)
{
    std::string ext = fs::path(caminho).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return ext;
}

std::string trocarExtensao(const std::string& caminho, const std::string& novaExtensao)
{
    fs::path p = fs::path(caminho);
    p.replace_extension(novaExtensao);
    return p.string();
}

bool ehArquivoOBJ(const std::string& caminho)
{
    return extensaoMinuscula(caminho) == ".obj";
}

bool ehImagemSuportada(const std::string& caminho)
{
    const std::string ext = extensaoMinuscula(caminho);
    return ext == ".png" || ext == ".bmp" || ext == ".jpg" || ext == ".jpeg";
}

void definirExecutavel(const char* argv0)
{
    if (argv0 == nullptr) {
        return;
    }
    pastaExecutavel = diretorioDe(caminhoAbsoluto(argv0));
    pastaProjetoCache.clear();
}

std::string pastaDoProjeto()
{
    if (!pastaProjetoCache.empty()) {
        return pastaProjetoCache;
    }

    std::error_code ec;
    std::vector<fs::path> inicios = {fs::current_path(ec)};
    if (!pastaExecutavel.empty()) {
        inicios.push_back(fs::path(pastaExecutavel));
    }

    for (fs::path p : inicios) {
        // Sobe ate 4 niveis (ex.: vs/x64/Release/ -> raiz do projeto).
        for (int nivel = 0; nivel < 5 && !p.empty(); ++nivel) {
            if (pareceRaizDoProjeto(p)) {
                pastaProjetoCache = p.lexically_normal().string();
                return pastaProjetoCache;
            }
            if (p == p.parent_path()) {
                break;
            }
            p = p.parent_path();
        }
    }

    pastaProjetoCache = diretorioAtual();
    return pastaProjetoCache;
}

std::string caminhoDoProjeto(const std::string& relativo)
{
    const fs::path p = fs::path(relativo);
    if (p.is_absolute()) {
        return relativo;
    }
    return (fs::path(pastaDoProjeto()) / p).lexically_normal().string();
}
