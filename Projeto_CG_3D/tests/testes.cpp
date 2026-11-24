// Testes unitarios do nucleo do visualizador (sem OpenGL / janela).
// Executar: "make testes" (Linux/Docker) ou scripts\windows.ps1 -Acao testes.

#include "ArquivoOBJ.h"
#include "Bezier.h"
#include "Caminhos.h"
#include "CoordenadasTextura.h"
#include "Imagem.h"
#include "Malha.h"
#include "Matriz4.h"
#include "OrdenacaoProfundidade.h"
#include "Transformacoes.h"

#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace {

int verificacoes = 0;
int falhas = 0;
std::string testeAtual;

void checar(bool condicao, const std::string& descricao)
{
    ++verificacoes;
    if (!condicao) {
        ++falhas;
        std::cout << "  [FALHOU] " << testeAtual << ": " << descricao << '\n';
    }
}

bool perto(double a, double b, double eps = 1e-5)
{
    return std::fabs(a - b) <= eps;
}

bool perto(const Vetor3& a, const Vetor3& b, double eps = 1e-5)
{
    return perto(a.x, b.x, eps) && perto(a.y, b.y, eps) && perto(a.z, b.z, eps);
}

std::string arquivoTemporario(const std::string& nome, const std::string& conteudo)
{
    const std::string caminho = juntarCaminho(caminhoDoProjeto("build"), nome);
    std::ofstream(caminho) << conteudo;
    return caminho;
}

// Cubo unitario [-1, 1]^3 com faces quadradas (testa triangulacao e normais).
const char* CUBO_OBJ =
    "# cubo\n"
    "v -1 -1 -1\nv 1 -1 -1\nv 1 1 -1\nv -1 1 -1\n"
    "v -1 -1 1\nv 1 -1 1\nv 1 1 1\nv -1 1 1\n"
    "vt 0 0\nvt 1 0\nvt 1 1\nvt 0 1\n"
    "vn 0 0 1\n"
    "f 5/1/1 6/2/1 7/3/1 8/4/1\n"   // +Z
    "f 2/1 1/2 4/3 3/4\n"           // -Z
    "f 6/1 2/2 3/3 7/4\n"           // +X
    "f 1/1 5/2 8/3 4/4\n"           // -X
    "f 8//1 7//1 3//1 4//1\n"       // +Y (formato v//vn)
    "f -8/-4 -7/-3 -3/-2 -4/-1\n";  // -Y (indices negativos: 1 2 6 5)

void testeMatrizes()
{
    const Vetor3 x(1, 0, 0);
    checar(perto(Matriz4::rotacao(2, 90).transformarPonto(x), Vetor3(0, 1, 0)), "rotacao Z 90: X -> Y");
    checar(perto(Matriz4::rotacao(1, 90).transformarPonto(x), Vetor3(0, 0, -1)), "rotacao Y 90: X -> -Z");
    checar(perto(Matriz4::rotacao(0, 90).transformarPonto(Vetor3(0, 1, 0)), Vetor3(0, 0, 1)), "rotacao X 90: Y -> Z");
    checar(perto(Matriz4::translacao(1, 2, 3).transformarPonto(Vetor3()), Vetor3(1, 2, 3)), "translacao");
    checar(perto(Matriz4::translacao(1, 2, 3).transformarDirecao(x), x), "direcao ignora translacao");
    checar(perto(Matriz4::reflexao(0).determinante3x3(), -1.0), "reflexao tem determinante -1");
    checar(perto(Matriz4::cisalhamento(0, 1, 0.5).transformarPonto(Vetor3(0, 2, 0)), Vetor3(1, 2, 0)),
           "cisalhamento X += 0.5*Y");

    // Normal de um plano inclinado apos escala nao uniforme continua perpendicular.
    const Matriz4 e = Matriz4::escala(2, 1, 1);
    const Vetor3 tangente = e.transformarDirecao(Vetor3(1, -1, 0));
    const Vetor3 normal = e.inversaTranspostaNormal().transformarDirecao(Vetor3(1, 1, 0));
    checar(perto(produtoEscalar(tangente, normal), 0.0), "inversa-transposta preserva perpendicularidade");
}

void testeTransformacoesAcumuladas()
{
    PilhaTransformacoes pilha;
    checar(pilha.vazia() && perto(pilha.matrizAcumulada().determinante3x3(), 1.0), "pilha comeca na identidade");

    // Gira 90 graus em Z e depois translada (1, 0, 0) no mundo.
    pilha.rotacionar(2, 90);
    pilha.transladar(1, 0, 0);
    checar(perto(pilha.matrizAcumulada().transformarPonto(Vetor3(1, 0, 0)), Vetor3(1, 1, 0)),
           "ordem: rotacao seguida de translacao no espaco do mundo");

    pilha.escalar(2, 2, 2);
    checar(perto(pilha.matrizAcumulada().transformarPonto(Vetor3(1, 0, 0)), Vetor3(2, 2, 0)), "escala aplicada por ultimo");
    checar(pilha.quantidade() == 3 && pilha.itens()[0].descricao().find("Rotacao") == 0, "descricao das transformacoes");

    pilha.limpar();
    checar(pilha.vazia() && perto(pilha.matrizAcumulada().transformarPonto(Vetor3(3, 4, 5)), Vetor3(3, 4, 5)), "limpar");
}

void testeLeituraOBJ()
{
    Malha m;
    std::string msg;
    checar(carregarObj(arquivoTemporario("teste_cubo.obj", CUBO_OBJ), m, msg), "carrega cubo: " + msg);
    checar(m.vertices.size() == 8 && m.triangulos.size() == 12, "quadrilateros triangulados (12 triangulos)");
    checar(m.coordTextura.size() == 4, "vt lidos");
    checar(!m.possuiCoordTextura(), "faces v//vn sem vt => malha sem vt completo");
    checar(m.triangulos[10].v[0] == 0 && m.triangulos[10].v[1] == 1 && m.triangulos[10].v[2] == 5 &&
               m.triangulos[10].vt[0] == 0,
           "indices negativos resolvidos");
    checar(perto(m.minimo, Vetor3(-1, -1, -1)) && perto(m.maximo, Vetor3(1, 1, 1)), "caixa envolvente");
    checar(perto(m.raio(), std::sqrt(3.0)), "raio da esfera envolvente");

    // Normais por face apontam para fora do cubo.
    bool todasParaFora = true;
    for (std::size_t f = 0; f < m.triangulos.size(); ++f) {
        const Triangulo& t = m.triangulos[f];
        const Vetor3 c = (m.vertices[t.v[0]] + m.vertices[t.v[1]] + m.vertices[t.v[2]]) * (1.0f / 3);
        todasParaFora = todasParaFora && produtoEscalar(m.normaisFace[f], c) > 0.0f &&
                        perto(m.normaisFace[f].comprimento(), 1.0);
    }
    checar(todasParaFora, "normais de face unitarias e para fora (flat)");

    // Normal media no canto (1,1,1) = media das 3 faces = (1,1,1)/sqrt(3).
    const float k = 1.0f / std::sqrt(3.0f);
    checar(perto(m.normaisVertice[6], Vetor3(k, k, k), 1e-4), "normal media por vertice (smooth)");

    Malha erro;
    checar(!carregarObj(arquivoTemporario("teste_erro.obj", "v 0 0 0\nv 1 0 0\nf 1 2 9\n"), erro, msg) &&
               msg.find("linha 3") != std::string::npos,
           "indice invalido gera erro com numero da linha");
    checar(!carregarObj(caminhoDoProjeto("build/nao_existe.obj"), erro, msg), "arquivo inexistente gera erro");
}

void testeCosturaUV()
{
    // Duas faces com vertices duplicados (mesma posicao, vt diferentes):
    // a normal suave deve ser compartilhada.
    const char* obj =
        "v 0 0 0\nv 1 0 0\nv 0 1 0\nv 1 0 0\nv 0 1 0\nv 1 1 0.5\n"
        "vt 0 0\nvt 1 0\nvt 0 1\n"
        "f 1/1 2/2 3/3\nf 4/1 6/2 5/3\n";
    Malha m;
    std::string msg;
    checar(carregarObj(arquivoTemporario("teste_costura.obj", obj), m, msg), "carrega malha com costura");
    checar(m.possuiCoordTextura(), "malha com vt em todos os cantos");
    checar(perto(m.normaisVertice[1], m.normaisVertice[3]), "vertices duplicados compartilham a normal media");
}

void testeCoordenadasTextura()
{
    Malha m;
    std::string msg;
    carregarObj(caminhoDoProjeto("modelos/bule.obj"), m, msg);

    GeradorUV usado = GeradorUV::ArquivoOBJ;
    const auto esfericas = gerarCoordenadasTextura(m, GeradorUV::ArquivoOBJ, &usado);
    checar(usado == GeradorUV::Esferica, "sem vt: ArquivoOBJ recai na projecao esferica");
    checar(esfericas.size() == m.triangulos.size() * 3, "uma coordenada por canto");

    bool semRasgo = true;
    for (std::size_t t = 0; t < m.triangulos.size(); ++t) {
        float mn = 10, mx = -10;
        for (int k = 0; k < 3; ++k) {
            mn = std::min(mn, esfericas[t * 3 + k].u);
            mx = std::max(mx, esfericas[t * 3 + k].u);
            semRasgo = semRasgo && esfericas[t * 3 + k].v >= -1e-4f && esfericas[t * 3 + k].v <= 1.0001f;
        }
        semRasgo = semRasgo && (mx - mn) <= 0.5f;
    }
    checar(semRasgo, "esferica: v em [0,1] e costura corrigida (nenhum triangulo atravessa a textura)");

    const auto planares = gerarCoordenadasTextura(m, GeradorUV::Planar, &usado);
    bool noIntervalo = usado == GeradorUV::Planar;
    for (const Vetor2& uv : planares) {
        noIntervalo = noIntervalo && uv.u >= -1e-4f && uv.u <= 1.0001f && uv.v >= -1e-4f && uv.v <= 1.0001f;
    }
    checar(noIntervalo, "planar: coordenadas em [0,1]");

    const auto cilindricas = gerarCoordenadasTextura(m, GeradorUV::Cilindrica, &usado);
    checar(usado == GeradorUV::Cilindrica && cilindricas.size() == planares.size(), "cilindrica gerada");

    Malha spot;
    carregarObj(caminhoDoProjeto("modelos/spot.obj"), spot, msg);
    const auto doArquivo = gerarCoordenadasTextura(spot, GeradorUV::ArquivoOBJ, &usado);
    const Triangulo& t0 = spot.triangulos[0];
    checar(usado == GeradorUV::ArquivoOBJ &&
               perto(doArquivo[0].u, spot.coordTextura[t0.vt[0]].u) &&
               perto(doArquivo[0].v, spot.coordTextura[t0.vt[0]].v),
           "com vt: coordenadas do arquivo sao usadas");
}

void testeModelos()
{
    Malha bule, spot;
    std::string msg;
    checar(carregarObj(caminhoDoProjeto("modelos/bule.obj"), bule, msg), "carrega modelos/bule.obj");
    checar(bule.triangulos.size() >= 1600, "malha principal com >= 1600 triangulos (" +
                                               std::to_string(bule.triangulos.size()) + ")");
    checar(carregarObj(caminhoDoProjeto("modelos/spot.obj"), spot, msg), "carrega modelos/spot.obj");
    checar(spot.possuiCoordTextura() && spot.triangulos.size() >= 1600, "spot.obj possui vt");
    checar(nomeArquivo(spot.arquivoTextura) == "spot.png", "textura associada (spot.png) encontrada");
}

void testeBezierLegado()
{
    checar(perto(bezierCubica({0, 0}, {0, 1}, {1, 1}, {1, 0}, 0.5).x, 0.5) &&
               perto(bezierCubica({0, 0}, {0, 1}, {1, 1}, {1, 0}, 0.5).y, 0.75),
           "bezier cubica em t = 0.5");

    // Grupos 3n+1 sao curvas independentes; caso contrario, uma curva encadeada.
    const std::vector<std::vector<Ponto>> independentes = {
        {{0, 0}, {1, 0}, {2, 0}, {3, 0}}, {{0, 1}, {1, 1}, {2, 1}, {3, 1}, {4, 1}, {5, 1}, {6, 1}}};
    checar(organizarCurvas(independentes).size() == 2, "grupos 3n+1 => curvas independentes");
    const std::vector<std::vector<Ponto>> encadeados = {{{0, 0}, {1, 0}}, {{2, 0}, {3, 0}, {4, 0}}};
    const auto unica = organizarCurvas(encadeados);
    checar(unica.size() == 1 && unica[0].size() == 4, "grupos irregulares => curva unica (como no 2D)");

    Malha rosa, mario;
    std::string msg;
    checar(carregarObj(caminhoDoProjeto("desenhos/rosa.obj"), rosa, msg), "rosa.obj (2D) carregado");
    checar(rosa.triangulos.size() > 1000 && rosa.possuiCoordTextura(), "rosa convertida em tubos 3D com vt");
    checar(rosa.maximo.z > 0.0f && rosa.minimo.z < 0.0f, "tubos tem volume em Z");
    checar(carregarObj(caminhoDoProjeto("desenhos/mario.obj"), mario, msg) && mario.triangulos.size() > 1600,
           "mario.obj (2D) convertido em malha 3D");
}

void testeOrdenacao()
{
    // Tres triangulos em z = -1, -5 e -3 (camera na origem olhando para -z).
    std::vector<float> pos;
    for (float z : {-1.0f, -5.0f, -3.0f}) {
        const float tri[9] = {0, 0, z, 1, 0, z, 0, 1, z};
        pos.insert(pos.end(), tri, tri + 9);
    }

    double identidade[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
    std::vector<unsigned int> indices;
    ordenarTriangulosPorProfundidade(pos, identidade, indices);
    checar(indices.size() == 9 && indices[0] == 3 && indices[3] == 6 && indices[6] == 0,
           "triangulos ordenados do mais distante ao mais proximo");

    // Camera girada 180 graus em Y: a ordem se inverte.
    double girada[16] = {-1, 0, 0, 0, 0, 1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1};
    ordenarTriangulosPorProfundidade(pos, girada, indices);
    checar(indices[0] == 0 && indices[6] == 3, "ordenacao depende da camera");
}

void testeImagens()
{
    Imagem img;
    std::string msg;
    checar(carregarImagem(caminhoDoProjeto("texturas/madeira.jpg"), img, msg) && img.largura > 0, "carrega .jpg");
    checar(carregarImagem(caminhoDoProjeto("texturas/xadrez.bmp"), img, msg) && img.largura > 0, "carrega .bmp");
    // Linha 0 = base da imagem (convencao OpenGL): a faixa vermelha do topo
    // do xadrez deve estar na ultima linha.
    checar(img.pixel(img.largura / 2, img.altura - 1)[0] > 150 && img.pixel(img.largura / 2, img.altura - 1)[2] < 100,
           "imagem invertida verticalmente para o OpenGL");
    checar(carregarImagem(caminhoDoProjeto("texturas/marmore.png"), img, msg) && img.largura > 0, "carrega .png");
    checar(!carregarImagem(caminhoDoProjeto("texturas/nao_existe.png"), img, msg), "imagem inexistente gera erro");

    const Imagem ambiente = gerarMapaAmbienteEstudio(64);
    const std::string saida = juntarCaminho(caminhoDoProjeto("build"), "teste_ambiente.png");
    Imagem relida;
    checar(salvarPNG(saida, ambiente) && carregarImagem(saida, relida, msg) &&
               relida.largura == 64 && relida.rgba == ambiente.rgba,
           "mapa de ambiente gerado e PNG salvo/relido sem perdas");

    checar(ehArquivoOBJ("a/b/Modelo.OBJ") && !ehArquivoOBJ("x.png"), "filtro de extensao de modelos");
    checar(ehImagemSuportada("x.JPEG") && ehImagemSuportada("x.bmp") && ehImagemSuportada("x.png") &&
               !ehImagemSuportada("x.obj"),
           "filtro de extensao de imagens");
}

void testeSalvarOBJ()
{
    Malha m, relida;
    std::string msg;
    carregarObj(arquivoTemporario("teste_cubo2.obj", CUBO_OBJ), m, msg);

    // Reflexao + translacao: a ordem dos cantos e invertida para manter as normais para fora.
    const Matriz4 t = Matriz4::translacao(10, 0, 0) * Matriz4::reflexao(0);
    const std::string saida = juntarCaminho(caminhoDoProjeto("build"), "teste_salvo.obj");
    checar(salvarObj(saida, m, t, msg), "salva OBJ: " + msg);
    checar(carregarObj(saida, relida, msg) && relida.triangulos.size() == m.triangulos.size(), "OBJ salvo e relido");
    checar(perto(relida.centro(), Vetor3(10, 0, 0)), "vertices transformados");

    bool paraFora = true;
    for (std::size_t f = 0; f < relida.triangulos.size(); ++f) {
        const Triangulo& tr = relida.triangulos[f];
        const Vetor3 c = (relida.vertices[tr.v[0]] + relida.vertices[tr.v[1]] + relida.vertices[tr.v[2]]) * (1.0f / 3);
        paraFora = paraFora && produtoEscalar(relida.normaisFace[f], c - relida.centro()) > 0.0f;
    }
    checar(paraFora, "normais continuam para fora apos reflexao");
}

}

int main(int, char** argv)
{
    definirExecutavel(argv[0]);
    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::path(caminhoDoProjeto("build")), ec);

    const std::vector<std::pair<const char*, std::function<void()>>> testes = {
        {"Matriz4", testeMatrizes},
        {"Transformacoes acumuladas", testeTransformacoesAcumuladas},
        {"Leitura de OBJ e normais", testeLeituraOBJ},
        {"Normais em costuras de UV", testeCosturaUV},
        {"Coordenadas de textura", testeCoordenadasTextura},
        {"Modelos do projeto", testeModelos},
        {"Bezier 2D -> 3D", testeBezierLegado},
        {"Ordenacao por profundidade", testeOrdenacao},
        {"Imagens e filtros", testeImagens},
        {"Salvar OBJ", testeSalvarOBJ},
    };

    for (const auto& [nome, funcao] : testes) {
        testeAtual = nome;
        const int falhasAntes = falhas;
        funcao();
        std::cout << (falhas == falhasAntes ? "[OK]     " : "[FALHOU] ") << nome << '\n';
    }

    std::cout << "\n" << verificacoes << " verificacoes, " << falhas << " falha(s)\n";
    return falhas == 0 ? 0 : 1;
}
