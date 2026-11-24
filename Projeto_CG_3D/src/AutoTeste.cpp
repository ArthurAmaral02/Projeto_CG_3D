#include "AutoTeste.h"

#include "Aplicacao.h"
#include "Caminhos.h"
#include "CoordenadasTextura.h"
#include "Imagem.h"
#include "OpenGL.h"

#include <GL/glui.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <vector>

namespace {

Aplicacao& app = aplicacao();
const EstatisticasQuadro& stats = aplicacao().visualizacao.estatisticas();

std::string pasta;
std::vector<std::function<void()>> etapas;
std::size_t etapaAtual = 0;
std::vector<std::string> relatorio;
int falhas = 0;

// Capturas guardadas para comparacoes entre etapas.
Imagem base, semLuz, flat, luzMovida, duasLuzes, brilhoBaixo, brilhoAlto;
Imagem opaco, vazio, translucido, reflexao, texEsferica, texCilindrica, texPlanar;

void verificar(bool condicao, const std::string& descricao, const std::string& detalhe = "")
{
    std::string linha = (condicao ? "[OK]     " : "[FALHOU] ") + descricao;
    if (!detalhe.empty()) {
        linha += "  (" + detalhe + ")";
    }
    if (!condicao) {
        ++falhas;
    }
    relatorio.push_back(linha);
    std::cout << linha << std::endl;
}

std::string numero(double v)
{
    std::ostringstream s;
    s.precision(4);
    s << v;
    return s.str();
}

// Renderiza um quadro, le o back buffer e salva em PNG.
Imagem capturar(const std::string& nome)
{
    glutSetWindow(app.janela);

    app.desenhar();
    glFinish();

    Imagem img;
    img.largura = stats.largura;
    img.altura = stats.altura;
    img.rgba.resize(static_cast<std::size_t>(img.largura) * img.altura * 4);

    glReadBuffer(GL_BACK);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, img.largura, img.altura, GL_RGBA, GL_UNSIGNED_BYTE, img.rgba.data());
    glutSwapBuffers();

    salvarPNG(juntarCaminho(pasta, nome + ".png"), img);
    return img;
}

// Luminancia media (0..255) da regiao central (onde esta o objeto).
double luminanciaCentral(const Imagem& img)
{
    double soma = 0.0;
    long n = 0;

    for (int y = img.altura / 4; y < img.altura * 3 / 4; ++y) {
        for (int x = img.largura / 4; x < img.largura * 3 / 4; ++x) {
            const std::uint8_t* p = img.pixel(x, y);
            soma += 0.299 * p[0] + 0.587 * p[1] + 0.114 * p[2];
            ++n;
        }
    }

    return n ? soma / n : 0.0;
}

// Diferenca media absoluta por canal (0..255) entre duas capturas.
double diferenca(const Imagem& a, const Imagem& b)
{
    if (a.largura != b.largura || a.altura != b.altura || a.vazia()) {
        return 255.0;
    }

    double soma = 0.0;
    for (std::size_t i = 0; i < a.rgba.size(); i += 4) {
        for (int c = 0; c < 3; ++c) {
            soma += std::abs(static_cast<int>(a.rgba[i + c]) - static_cast<int>(b.rgba[i + c]));
        }
    }

    return soma / (a.rgba.size() / 4 * 3);
}

void configuracaoPadrao()
{
    app.config = Configuracao();
    app.config.geradorUV = static_cast<int>(app.malha.possuiCoordTextura() ? GeradorUV::ArquivoOBJ
                                                                           : GeradorUV::Esferica);
    app.camera.resetar();
    app.transformacoes.limpar();
    app.aplicarGeradorUV();
    GLUI_Master.sync_live_all();
}

void definirEtapas()
{
    // --- Janela, projecao e malha principal --------------------------------
    etapas.push_back([] {
        glutSetWindow(app.janela);  // a janela corrente pode ser o painel GLUI
        const int w = glutGet(GLUT_WINDOW_WIDTH);
        const int h = glutGet(GLUT_WINDOW_HEIGHT);
        verificar(w >= 600 && h >= 600, "2b Janela inicial com no minimo 600x600",
                  std::to_string(w) + "x" + std::to_string(h));

        verificar(nomeArquivo(app.caminhoModelo) == "bule.obj" && app.malha.triangulos.size() >= 1600,
                  "5  Malha principal (bule.obj) com >= 1600 triangulos",
                  std::to_string(app.malha.triangulos.size()) + " triangulos");
        configuracaoPadrao();
    });

    etapas.push_back([] {
        base = capturar("01_base_smooth_luz1");
        verificar(stats.perspectiva, "2b Projecao perspectiva (gluPerspective)",
                  "P[11]=" + numero(stats.projecao[11]) + " P[15]=" + numero(stats.projecao[15]));
        verificar(stats.modeloSombreamento == GL_SMOOTH, "3a Smooth shading ativo (GL_SMOOTH)");
        verificar(luminanciaCentral(base) > 20.0, "Objeto visivel e iluminado",
                  "luminancia " + numero(luminanciaCentral(base)));
    });

    // --- Sombreamento ------------------------------------------------------
    etapas.push_back([] {
        app.config.sombreamento = SOMBREAMENTO_FLAT;
        flat = capturar("02_flat");
        verificar(stats.modeloSombreamento == GL_FLAT, "3a Flat shading ativo (GL_FLAT, normal por face)");
        verificar(diferenca(flat, base) > 0.05, "3a Flat e Smooth produzem imagens diferentes",
                  "diferenca " + numero(diferenca(flat, base)));
        app.config.sombreamento = SOMBREAMENTO_SMOOTH;
    });

    // --- Iluminacao --------------------------------------------------------
    etapas.push_back([] {
        app.config.luzes[0].ligada = 0;
        app.config.luzes[1].ligada = 0;
        semLuz = capturar("03_luzes_desligadas");
        verificar(stats.luzesLigadas == 0 && luminanciaCentral(semLuz) < luminanciaCentral(base) * 0.7,
                  "2c Desligar as luzes escurece a cena",
                  numero(luminanciaCentral(base)) + " -> " + numero(luminanciaCentral(semLuz)));
        app.config.luzes[0].ligada = 1;
    });

    etapas.push_back([] {
        app.config.luzes[0].posicao[0] = -3.0f;
        app.config.luzes[0].posicao[1] = 1.0f;
        app.config.luzes[0].posicao[2] = 1.0f;
        luzMovida = capturar("04_luz1_movida");
        verificar(diferenca(luzMovida, base) > 1.0, "2c Mudar a posicao da luz altera a iluminacao",
                  "diferenca " + numero(diferenca(luzMovida, base)));
        app.config = Configuracao();
    });

    etapas.push_back([] {
        configuracaoPadrao();
        app.config.luzes[1].ligada = 1;
        duasLuzes = capturar("05_duas_luzes");
        verificar(stats.luzesLigadas == 2 && luminanciaCentral(duasLuzes) > luminanciaCentral(base),
                  "2c Segunda fonte de luz liga e soma iluminacao",
                  numero(luminanciaCentral(base)) + " -> " + numero(luminanciaCentral(duasLuzes)));
        app.config.luzes[1].ligada = 0;
    });

    // --- Reflexao especular (Phong) ----------------------------------------
    etapas.push_back([] {
        app.config.intensidadeEspecular = 1.0f;
        app.config.brilho = 4.0f;
        brilhoBaixo = capturar("06_shininess_4");
        app.config.brilho = 128.0f;
        brilhoAlto = capturar("07_shininess_128");
        verificar(stats.brilho == 128.0f && diferenca(brilhoBaixo, brilhoAlto) > 0.5,
                  "3c Expoente especular (shininess) altera o brilho",
                  "diferenca " + numero(diferenca(brilhoBaixo, brilhoAlto)));
        configuracaoPadrao();
    });

    // --- Transparencia -----------------------------------------------------
    etapas.push_back([] {
        opaco = capturar("08_opaco");
        app.config.opacidade = 0.0f;
        vazio = capturar("09_opacidade_0");
        app.config.opacidade = 0.4f;
        translucido = capturar("10_opacidade_0_4");

        verificar(stats.translucido, "3b GL_BLEND ativo com opacidade < 1");
        verificar(stats.escritaProfundidadeDesligada, "3b Escrita no depth buffer desligada no desenho translucido");
        verificar(stats.triangulosOrdenados == app.malha.triangulos.size(),
                  "3b Triangulos ordenados de tras para frente",
                  std::to_string(stats.triangulosOrdenados) + " ordenados");
        const double dOpaco = diferenca(opaco, vazio);
        const double dTrans = diferenca(translucido, vazio);
        verificar(dTrans > 0.2 && dTrans < dOpaco,
                  "3b Objeto translucido fica entre o opaco e o fundo",
                  "dist. ao fundo: opaco " + numero(dOpaco) + ", 0.4 " + numero(dTrans));
        configuracaoPadrao();
    });

    // --- Reflexao do ambiente ----------------------------------------------
    etapas.push_back([] {
        app.config.modoSuperficie = SUPERFICIE_REFLEXAO;
        reflexao = capturar("11_reflexao_ambiente");
        verificar(stats.reflexaoEsferica, "3c Reflexao do ambiente (GL_SPHERE_MAP) ativa");
        verificar(diferenca(reflexao, base) > 2.0, "3c Reflexao altera a aparencia do objeto",
                  "diferenca " + numero(diferenca(reflexao, base)));
        configuracaoPadrao();
    });

    // --- Textura com coordenadas geradas (bule nao tem vt) -----------------
    etapas.push_back([] {
        app.config.modoSuperficie = SUPERFICIE_TEXTURA;
        app.config.geradorUV = static_cast<int>(GeradorUV::ArquivoOBJ);
        app.aplicarGeradorUV();
        verificar(!app.malha.possuiCoordTextura() &&
                      app.visualizacao.geradorUVEmUso() == GeradorUV::Esferica,
                  "3d Sem vt no OBJ: coordenadas geradas automaticamente (esferica)");
        texEsferica = capturar("12_textura_esferica");
        const bool texturaNoQuadro = stats.texturaAtiva;

        // Comparacao justa: o mesmo objeto solido branco, sem textura.
        app.config.modoSuperficie = SUPERFICIE_SOLIDA;
        app.config.corObjeto[0] = app.config.corObjeto[1] = app.config.corObjeto[2] = 1.0f;
        const Imagem branco = capturar("12b_solido_branco");
        app.config = Configuracao();
        app.config.modoSuperficie = SUPERFICIE_TEXTURA;
        verificar(texturaNoQuadro && diferenca(texEsferica, branco) > 2.0,
                  "3d Textura mapeada na malha (madeira.jpg) x solido branco",
                  "diferenca " + numero(diferenca(texEsferica, branco)));

        app.config.geradorUV = static_cast<int>(GeradorUV::Cilindrica);
        app.aplicarGeradorUV();
        texCilindrica = capturar("13_textura_cilindrica");
        app.config.geradorUV = static_cast<int>(GeradorUV::Planar);
        app.aplicarGeradorUV();
        texPlanar = capturar("14_textura_planar");
        verificar(diferenca(texEsferica, texCilindrica) > 0.3 && diferenca(texCilindrica, texPlanar) > 0.3,
                  "3d Projecoes esferica, cilindrica e planar geram mapeamentos diferentes");
    });

    etapas.push_back([] {
        const bool carregou = app.carregarTextura(caminhoDoProjeto("texturas/xadrez.bmp"));
        capturar("15_textura_bmp");
        const bool png = app.carregarTextura(caminhoDoProjeto("texturas/marmore.png"));
        capturar("16_textura_png");
        verificar(carregou && png, "3d Carrega texturas .bmp, .png e .jpg");
    });

    // --- Textura com vt do arquivo -----------------------------------------
    etapas.push_back([] {
        app.carregarModelo(caminhoDoProjeto("modelos/spot.obj"));
        configuracaoPadrao();
        app.config.modoSuperficie = SUPERFICIE_TEXTURA;
        const Imagem spotOpaco = capturar("17_spot_textura_vt");
        verificar(app.malha.possuiCoordTextura() &&
                      app.visualizacao.geradorUVEmUso() == GeradorUV::ArquivoOBJ &&
                      app.texturaObjeto.descricao() == "spot.png" && stats.texturaAtiva,
                  "3d OBJ com vt: coordenadas do arquivo aproveitadas (spot.obj + spot.png)");

        // Regressao: o alfa da imagem nao pode "furar" o objeto translucido.
        app.config.opacidade = 0.99f;
        const Imagem spotQuaseOpaco = capturar("17b_spot_textura_opacidade_0_99");
        verificar(diferenca(spotOpaco, spotQuaseOpaco) < 1.0,
                  "3b Textura + opacidade 0.99 ~ opaco (alfa da imagem nao fura a malha)",
                  "diferenca " + numero(diferenca(spotOpaco, spotQuaseOpaco)));
    });

    // --- Redimensionamento mantendo a proporcao -----------------------------
    etapas.push_back([] {
        app.carregarModelo(caminhoDoProjeto("modelos/bule.obj"));
        app.carregarTextura(caminhoDoProjeto("texturas/madeira.jpg"));
        configuracaoPadrao();
        glutSetWindow(app.janela);
        glutReshapeWindow(900, 600);
    });
    etapas.push_back([] {});  // aguarda o evento de reshape
    etapas.push_back([] {
        capturar("18_janela_900x600");
        const double aspecto = stats.projecao[5] / stats.projecao[0];
        verificar(stats.largura == 900 && stats.altura == 600 && std::fabs(aspecto - 1.5) < 1e-6,
                  "2b Redimensionar 900x600 mantem a proporcao (aspect 1.5)",
                  std::to_string(stats.largura) + "x" + std::to_string(stats.altura) +
                      ", aspect da projecao " + numero(aspecto));
        glutReshapeWindow(600, 900);
    });
    etapas.push_back([] {});
    etapas.push_back([] {
        capturar("19_janela_600x900");
        const double aspecto = stats.projecao[5] / stats.projecao[0];
        verificar(stats.largura == 600 && stats.altura == 900 && std::fabs(aspecto - 600.0 / 900.0) < 1e-6,
                  "2b Redimensionar 600x900 mantem a proporcao (aspect 0.667)",
                  "aspect da projecao " + numero(aspecto));
        glutSetWindow(app.janela);
        glutReshapeWindow(800, 700);
    });
    etapas.push_back([] {});

    // --- Transformacoes 3D (funcionalidade do projeto original) -------------
    etapas.push_back([] {
        configuracaoPadrao();
        app.config.mostrarOriginal = 1;
        app.transformacoes.rotacionar(1, 40.0);
        app.transformacoes.escalar(1.0, 1.4, 1.0);
        app.transformacoes.transladar(0.4, 0.0, 0.0);
        app.transformacoes.refletir(0);
        const Imagem img = capturar("20_transformacoes");
        verificar(app.transformacoes.quantidade() == 4 &&
                      app.transformacoes.matrizAcumulada().determinante3x3() < 0.0 &&
                      diferenca(img, base) > 1.0,
                  "Transformacoes 3D acumuladas (rotacao, escala, translacao, reflexao)");

        const std::string saida = juntarCaminho(pasta, "bule_transformado.obj");
        verificar(app.salvarModelo(saida) && arquivoExiste(saida), "Salvar OBJ transformado");
    });

    // --- Desenhos 2D do projeto original viram tubos 3D ---------------------
    etapas.push_back([] {
        const bool ok = app.carregarModelo(caminhoDoProjeto("desenhos/mario.obj"));
        configuracaoPadrao();
        app.config.modoSuperficie = SUPERFICIE_REFLEXAO;
        capturar("21_mario_bezier_3d");
        verificar(ok && app.malha.triangulos.size() > 1600,
                  "Desenho Bezier 2D original (mario.obj) convertido em malha 3D",
                  std::to_string(app.malha.triangulos.size()) + " triangulos");
    });

    // --- Cena final de demonstracao ----------------------------------------
    etapas.push_back([] {
        app.carregarModelo(caminhoDoProjeto("modelos/bule.obj"));
        configuracaoPadrao();
        app.config.modoSuperficie = SUPERFICIE_REFLEXAO;
        app.config.opacidade = 0.55f;
        app.config.luzes[1].ligada = 1;
        capturar("22_demo_reflexo_translucido");
    });
}

void executarEtapa(int)
{
    if (etapaAtual < etapas.size()) {
        etapas[etapaAtual++]();
        glutTimerFunc(250, executarEtapa, 0);
        return;
    }

    std::ofstream arquivo(juntarCaminho(pasta, "relatorio.txt"));
    arquivo << "Autoteste do Visualizador 3D\n\n";
    for (const std::string& linha : relatorio) {
        arquivo << linha << '\n';
    }
    arquivo << "\nVerificacoes: " << relatorio.size() << ", falhas: " << falhas << '\n';

    std::cout << "\nAutoteste concluido: " << relatorio.size() << " verificacoes, "
              << falhas << " falha(s). Capturas em " << pasta << std::endl;

    std::exit(falhas == 0 ? 0 : 1);
}

}

void iniciarAutoteste(const std::string& pastaSaida)
{
    pasta = caminhoAbsoluto(pastaSaida);
    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::path(pasta), ec);

    definirEtapas();
    glutTimerFunc(500, executarEtapa, 0);
}
