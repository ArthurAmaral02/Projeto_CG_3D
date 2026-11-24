#include "ArquivoOBJ.h"

#include "Bezier.h"
#include "Caminhos.h"
#include "Ponto.h"

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <vector>

namespace {

// Converte um indice do .obj (1-based, ou negativo = relativo ao fim) para 0-based.
bool resolverIndice(long indiceObj, std::size_t quantidade, int& saida)
{
    long resolvido = 0;

    if (indiceObj > 0) {
        resolvido = indiceObj - 1;
    } else if (indiceObj < 0) {
        resolvido = static_cast<long>(quantidade) + indiceObj;
    } else {
        return false;
    }

    if (resolvido < 0 || resolvido >= static_cast<long>(quantidade)) {
        return false;
    }

    saida = static_cast<int>(resolvido);
    return true;
}

// Le um canto de face no formato v, v/vt, v//vn ou v/vt/vn.
bool lerCanto(const std::string& token, std::size_t qtdVertices, std::size_t qtdTextura,
              int& indiceV, int& indiceVt)
{
    indiceVt = -1;

    const std::size_t barra = token.find('/');
    const std::string parteV = token.substr(0, barra);

    char* fim = nullptr;
    const long v = std::strtol(parteV.c_str(), &fim, 10);
    if (parteV.empty() || *fim != '\0' || !resolverIndice(v, qtdVertices, indiceV)) {
        return false;
    }

    if (barra != std::string::npos) {
        const std::size_t barra2 = token.find('/', barra + 1);
        const std::string parteVt = token.substr(barra + 1, barra2 == std::string::npos
                                                                ? std::string::npos
                                                                : barra2 - barra - 1);
        if (!parteVt.empty()) {
            const long vt = std::strtol(parteVt.c_str(), &fim, 10);
            if (*fim != '\0' || !resolverIndice(vt, qtdTextura, indiceVt)) {
                return false;
            }
        }
    }

    return true;
}

std::string removerEspacos(const std::string& s)
{
    const std::size_t ini = s.find_first_not_of(" \t\r\n");
    if (ini == std::string::npos) {
        return "";
    }
    const std::size_t fim = s.find_last_not_of(" \t\r\n");
    return s.substr(ini, fim - ini + 1);
}

// Procura "map_Kd" no arquivo .mtl. Retorna o caminho absoluto ou vazio.
std::string texturaDoMaterial(const std::string& caminhoMtl)
{
    std::ifstream arquivo(caminhoMtl);
    std::string linha;

    while (std::getline(arquivo, linha)) {
        std::istringstream leitor(linha);
        std::string tipo;
        leitor >> tipo;

        if (tipo == "map_Kd") {
            // Opcoes (-s, -o, ...) podem vir antes; o nome do arquivo e o ultimo token.
            std::string token, ultimo;
            while (leitor >> token) {
                ultimo = token;
            }
            if (!ultimo.empty()) {
                const std::string caminho = juntarCaminho(diretorioDe(caminhoMtl), ultimo);
                if (arquivoExiste(caminho)) {
                    return caminho;
                }
            }
        }
    }

    return "";
}

// Se nao houver .mtl, aceita uma imagem com o mesmo nome do .obj (modelo.png).
std::string texturaComMesmoNome(const std::string& caminhoObj)
{
    for (const char* ext : {".png", ".jpg", ".jpeg", ".bmp"}) {
        const std::string candidato = trocarExtensao(caminhoObj, ext);
        if (arquivoExiste(candidato)) {
            return candidato;
        }
    }
    return "";
}

}

bool carregarObj(const std::string& caminho, Malha& malha, std::string& mensagem)
{
    std::ifstream arquivo(caminho);

    if (!arquivo) {
        mensagem = "Nao foi possivel abrir: " + caminho;
        return false;
    }

    Malha nova;
    nova.nome = nomeArquivo(caminho);

    std::vector<std::vector<Ponto>> gruposControle(1);  // para arquivos 2D antigos
    std::string arquivoMtl;
    std::string linha;
    int numeroLinha = 0;

    while (std::getline(arquivo, linha)) {
        ++numeroLinha;

        std::istringstream leitor(linha);
        std::string tipo;
        leitor >> tipo;

        if (tipo == "v") {
            Vetor3 p;
            if (!(leitor >> p.x >> p.y)) {
                mensagem = "Vertice invalido na linha " + std::to_string(numeroLinha);
                return false;
            }
            if (!(leitor >> p.z)) {
                p.z = 0.0f;  // arquivos 2D do projeto original
            }
            nova.vertices.push_back(p);
            gruposControle.back().push_back(Ponto{p.x, p.y});
        }
        else if (tipo == "vt") {
            Vetor2 t;
            if (!(leitor >> t.u >> t.v)) {
                mensagem = "Coordenada de textura invalida na linha " + std::to_string(numeroLinha);
                return false;
            }
            nova.coordTextura.push_back(t);
        }
        else if (tipo == "f") {
            std::vector<int> v, vt;
            std::string token;

            while (leitor >> token) {
                int iv = 0, ivt = -1;
                if (!lerCanto(token, nova.vertices.size(), nova.coordTextura.size(), iv, ivt)) {
                    mensagem = "Face com indice invalido na linha " + std::to_string(numeroLinha) +
                               ": \"" + token + "\"";
                    return false;
                }
                v.push_back(iv);
                vt.push_back(ivt);
            }

            if (v.size() < 3) {
                mensagem = "Face com menos de 3 vertices na linha " + std::to_string(numeroLinha);
                return false;
            }

            // Triangulacao em leque (poligonos convexos).
            for (std::size_t k = 1; k + 1 < v.size(); ++k) {
                Triangulo t;
                t.v[0] = v[0];  t.v[1] = v[k];  t.v[2] = v[k + 1];
                t.vt[0] = vt[0]; t.vt[1] = vt[k]; t.vt[2] = vt[k + 1];
                nova.triangulos.push_back(t);
            }
        }
        else if (tipo == "mtllib") {
            arquivoMtl = removerEspacos(linha.substr(linha.find("mtllib") + 6));
        }
        else if (tipo == "o" || tipo == "g" || (!tipo.empty() && tipo[0] == '#')) {
            // Novo grupo de pontos de controle (so importa para arquivos 2D).
            if (!gruposControle.back().empty()) {
                gruposControle.emplace_back();
            }
        }
    }

    if (nova.triangulos.empty()) {
        // Arquivo do projeto 2D: pontos de controle de curvas de Bezier.
        const auto curvas = organizarCurvas(gruposControle);

        if (curvas.empty()) {
            mensagem = "O arquivo nao possui faces nem curvas de Bezier: " + caminho;
            return false;
        }

        gerarMalhaTubular(curvas, nova);
        nova.nome += " (curvas Bezier -> tubos 3D)";
    }

    if (!arquivoMtl.empty()) {
        nova.arquivoTextura = texturaDoMaterial(juntarCaminho(diretorioDe(caminho), arquivoMtl));
    }
    if (nova.arquivoTextura.empty()) {
        nova.arquivoTextura = texturaComMesmoNome(caminho);
    }

    nova.atualizarDerivados();

    malha = std::move(nova);

    std::ostringstream resumo;
    resumo << "Carregado " << malha.nome << ": " << malha.triangulos.size()
           << " triangulos, " << malha.vertices.size() << " vertices"
           << (malha.possuiCoordTextura() ? ", com vt" : ", sem vt");
    mensagem = resumo.str();

    return true;
}

bool salvarObj(const std::string& caminho,
               const Malha& malha,
               const Matriz4& transformacao,
               std::string& mensagem)
{
    if (malha.vazia()) {
        mensagem = "Nenhuma malha carregada para salvar.";
        return false;
    }

    std::ofstream arquivo(caminho);

    if (!arquivo) {
        mensagem = "Erro ao criar o arquivo: " + caminho;
        return false;
    }

    // Reflexoes invertem a orientacao: a ordem dos cantos e trocada para
    // manter as normais apontando para fora.
    const bool inverterOrdem = transformacao.determinante3x3() < 0.0;
    const bool comTextura = malha.possuiCoordTextura();

    arquivo << "# Arquivo gerado pelo Visualizador 3D (Projeto_CG_3D)\n";
    arquivo << "# " << malha.vertices.size() << " vertices, "
            << malha.triangulos.size() << " triangulos\n\n";

    for (const Vetor3& v : malha.vertices) {
        const Vetor3 p = transformacao.transformarPonto(v);
        arquivo << "v " << p.x << ' ' << p.y << ' ' << p.z << '\n';
    }

    if (comTextura) {
        for (const Vetor2& t : malha.coordTextura) {
            arquivo << "vt " << t.u << ' ' << t.v << '\n';
        }
    }

    for (const Triangulo& t : malha.triangulos) {
        const int ordem[3] = {0, inverterOrdem ? 2 : 1, inverterOrdem ? 1 : 2};
        arquivo << 'f';
        for (int k : ordem) {
            arquivo << ' ' << (t.v[k] + 1);
            if (comTextura) {
                arquivo << '/' << (t.vt[k] + 1);
            }
        }
        arquivo << '\n';
    }

    if (!arquivo) {
        mensagem = "Erro ao escrever o arquivo: " + caminho;
        return false;
    }

    mensagem = "Malha salva em: " + caminho;
    return true;
}
