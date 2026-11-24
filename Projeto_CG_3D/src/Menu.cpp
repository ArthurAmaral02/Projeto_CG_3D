#include "Menu.h"

#include "Aplicacao.h"
#include "Caminhos.h"
#include "CoordenadasTextura.h"
#include "DialogoArquivo.h"
#include "OpenGL.h"

#include <GL/glui.h>

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

namespace {

// ---------------------------------------------------------------------------
// Identificadores dos controles (um unico callback trata todos).
// ---------------------------------------------------------------------------
enum IdControle {
    ID_REDESENHAR = 1,
    ID_ABRIR_OBJ,
    ID_ABRIR_OBJ_WINDOWS,
    ID_SALVAR_OBJ,
    ID_ESCOLHER_TEXTURA,
    ID_ESCOLHER_TEXTURA_WINDOWS,
    ID_ESCOLHER_AMBIENTE,
    ID_AMBIENTE_PADRAO,
    ID_GERADOR_UV,
    ID_ROTACIONAR,
    ID_TRANSLADAR,
    ID_ESCALAR,
    ID_REFLETIR,
    ID_CISALHAR,
    ID_RESETAR_TRANSFORMACOES,
    ID_RESETAR_CAMERA,
    ID_SAIR
};

// Opcoes do menu de contexto (botao direito) da janela 3D.
enum OpcaoMenu {
    MENU_FLAT = 1,
    MENU_SMOOTH,
    MENU_SOLIDO,
    MENU_TEXTURA,
    MENU_REFLEXAO,
    MENU_LUZ_1,
    MENU_LUZ_2,
    MENU_RESETAR_CAMERA,
    MENU_SAIR
};

// Pares (eixo alvo, eixo referencia) da lista de cisalhamentos.
const int CISALHAMENTOS[6][2] = {{0, 1}, {0, 2}, {1, 0}, {1, 2}, {2, 0}, {2, 1}};

// Valores digitados nos campos de transformacao (live variables da GLUI).
struct EntradasTransformacao {
    int eixoRotacao = 1;
    float angulo = 45.0f;
    float translacao[3] = {0.5f, 0.0f, 0.0f};
    float escala[3] = {1.0f, 1.5f, 1.0f};
    int planoReflexao = 0;
    int tipoCisalhamento = 0;
    float fatorCisalhamento = 0.5f;
};

EntradasTransformacao entradas;
std::string caminhoSalvar = "modelos/salvo.obj";

GLUI* painel = nullptr;
GLUI_StaticText* rotuloModelo = nullptr;
GLUI_StaticText* rotuloMalha = nullptr;
GLUI_StaticText* rotuloTextura = nullptr;
GLUI_StaticText* rotuloAmbiente = nullptr;
GLUI_StaticText* rotuloTransformacoes = nullptr;
GLUI_StaticText* rotuloStatus = nullptr;

// Sliders: o rotulo mostra o valor atual (a GLUI_Scrollbar nao exibe numeros).
struct RotuloSlider {
    GLUI_StaticText* rotulo;
    const char* nome;
    const float* valor;
    const char* formato;
};
std::vector<RotuloSlider> rotulosSliders;

// Estado do arraste do mouse na janela 3D.
int botaoArrastando = -1;
int ultimoX = 0;
int ultimoY = 0;

std::string encurtar(const std::string& texto, std::size_t maximo)
{
    if (texto.size() <= maximo) {
        return texto;
    }
    return texto.substr(0, maximo - 3) + "...";
}

void sincronizarControles()
{
    GLUI_Master.sync_live_all();
    aplicacao().solicitarRedesenho();
}

// ---------------------------------------------------------------------------
// Retornos dos dialogos de arquivo
// ---------------------------------------------------------------------------
void aoEscolherModelo(const std::string& caminho)
{
    if (aplicacao().carregarModelo(caminho)) {
        sincronizarControles();
    }
}

void aoEscolherTextura(const std::string& caminho)
{
    if (aplicacao().carregarTextura(caminho)) {
        // Escolher uma textura ja mostra o objeto texturizado.
        aplicacao().config.modoSuperficie = SUPERFICIE_TEXTURA;
        sincronizarControles();
    }
}

void aoEscolherAmbiente(const std::string& caminho)
{
    if (aplicacao().carregarMapaAmbiente(caminho)) {
        aplicacao().config.modoSuperficie = SUPERFICIE_REFLEXAO;
        sincronizarControles();
    }
}

// ---------------------------------------------------------------------------
// Callback unico dos controles GLUI
// ---------------------------------------------------------------------------
void aoMudarControle(int id)
{
    Aplicacao& app = aplicacao();
    PilhaTransformacoes& pilha = app.transformacoes;
    std::string caminho;

    switch (id) {
        case ID_ABRIR_OBJ:
            abrirNavegadorGLUI(TipoArquivo::ModeloOBJ, aoEscolherModelo);
            break;

        case ID_ABRIR_OBJ_WINDOWS:
            if (abrirDialogoNativo(TipoArquivo::ModeloOBJ, caminho)) {
                aoEscolherModelo(caminho);
            }
            break;

        case ID_SALVAR_OBJ:
            app.salvarModelo(caminhoDoProjeto(caminhoSalvar));
            break;

        case ID_ESCOLHER_TEXTURA:
            abrirNavegadorGLUI(TipoArquivo::Imagem, aoEscolherTextura);
            break;

        case ID_ESCOLHER_TEXTURA_WINDOWS:
            if (abrirDialogoNativo(TipoArquivo::Imagem, caminho)) {
                aoEscolherTextura(caminho);
            }
            break;

        case ID_ESCOLHER_AMBIENTE:
            abrirNavegadorGLUI(TipoArquivo::Imagem, aoEscolherAmbiente);
            break;

        case ID_AMBIENTE_PADRAO:
            app.mapaAmbiente.liberar();
            app.informar("Mapa de ambiente padrao (estudio).");
            break;

        case ID_GERADOR_UV:
            app.aplicarGeradorUV();
            sincronizarControles();
            break;

        case ID_ROTACIONAR:
            pilha.rotacionar(entradas.eixoRotacao, entradas.angulo);
            app.informar("Adicionada: " + pilha.itens().back().descricao());
            break;

        case ID_TRANSLADAR:
            pilha.transladar(entradas.translacao[0], entradas.translacao[1], entradas.translacao[2]);
            app.informar("Adicionada: " + pilha.itens().back().descricao());
            break;

        case ID_ESCALAR:
            pilha.escalar(entradas.escala[0], entradas.escala[1], entradas.escala[2]);
            app.informar("Adicionada: " + pilha.itens().back().descricao());
            break;

        case ID_REFLETIR:
            pilha.refletir(entradas.planoReflexao);
            app.informar("Adicionada: " + pilha.itens().back().descricao());
            break;

        case ID_CISALHAR: {
            const int* par = CISALHAMENTOS[entradas.tipoCisalhamento];
            pilha.cisalhar(par[0], par[1], entradas.fatorCisalhamento);
            app.informar("Adicionada: " + pilha.itens().back().descricao());
            break;
        }

        case ID_RESETAR_TRANSFORMACOES:
            app.resetarTransformacoes();
            break;

        case ID_RESETAR_CAMERA:
            app.camera.resetar();
            break;

        case ID_SAIR:
            std::exit(0);

        default:
            break;
    }

    atualizarInterface();
    app.solicitarRedesenho();
}

// ---------------------------------------------------------------------------
// Construcao dos paineis
// ---------------------------------------------------------------------------
GLUI_Scrollbar* criarSlider(GLUI_Node* pai, const char* rotulo, float* variavel, float minimo, float maximo,
                            const char* formato = "%.2f")
{
    rotulosSliders.push_back({new GLUI_StaticText(pai, rotulo), rotulo, variavel, formato});
    GLUI_Scrollbar* slider = new GLUI_Scrollbar(pai, rotulo, GLUI_SCROLL_HORIZONTAL, variavel,
                                                ID_REDESENHAR, aoMudarControle);
    slider->set_float_limits(minimo, maximo);
    return slider;
}

GLUI_Spinner* criarSpinner(GLUI_Node* pai, const char* rotulo, float* variavel, float minimo, float maximo)
{
    GLUI_Spinner* spinner = new GLUI_Spinner(pai, rotulo, variavel, ID_REDESENHAR, aoMudarControle);
    spinner->set_float_limits(minimo, maximo);
    return spinner;
}

void criarPainelArquivo(GLUI_Node* pai)
{
    GLUI_Panel* p = new GLUI_Panel(pai, "Arquivo");

    rotuloModelo = new GLUI_StaticText(p, "Modelo: -");
    rotuloMalha = new GLUI_StaticText(p, "");
    new GLUI_Button(p, "Abrir OBJ...", ID_ABRIR_OBJ, aoMudarControle);
    if (dialogoNativoDisponivel()) {
        new GLUI_Button(p, "Abrir OBJ (Windows)...", ID_ABRIR_OBJ_WINDOWS, aoMudarControle);
    }
    new GLUI_Separator(p);
    GLUI_EditText* campo = new GLUI_EditText(p, "Salvar como:", caminhoSalvar);
    campo->set_w(220);
    new GLUI_Button(p, "Salvar OBJ", ID_SALVAR_OBJ, aoMudarControle);
}

void criarPainelIluminacao(GLUI_Node* pai)
{
    Configuracao& c = aplicacao().config;
    GLUI_Panel* p = new GLUI_Panel(pai, "Iluminacao");

    const char* nomes[NUMERO_LUZES] = {"Luz 1 (principal)", "Luz 2 (preenchimento)"};

    for (int i = 0; i < NUMERO_LUZES; ++i) {
        GLUI_Panel* luz = new GLUI_Panel(p, nomes[i]);
        new GLUI_Checkbox(luz, "Ligada", &c.luzes[i].ligada, ID_REDESENHAR, aoMudarControle);
        criarSpinner(luz, "Posicao X:", &c.luzes[i].posicao[0], -8.0f, 8.0f);
        criarSpinner(luz, "Posicao Y:", &c.luzes[i].posicao[1], -8.0f, 8.0f);
        criarSpinner(luz, "Posicao Z:", &c.luzes[i].posicao[2], -8.0f, 8.0f);
    }

    new GLUI_Checkbox(p, "Mostrar fontes de luz", &c.mostrarLuzes, ID_REDESENHAR, aoMudarControle);
}

void criarPainelSombreamento(GLUI_Node* pai)
{
    Configuracao& c = aplicacao().config;
    GLUI_Panel* p = new GLUI_Panel(pai, "Sombreamento");

    GLUI_RadioGroup* grupo = new GLUI_RadioGroup(p, &c.sombreamento, ID_REDESENHAR, aoMudarControle);
    new GLUI_RadioButton(grupo, "Flat (constante por face)");
    new GLUI_RadioButton(grupo, "Smooth (Gouraud)");
}

void criarPainelMaterial(GLUI_Node* pai)
{
    Configuracao& c = aplicacao().config;
    GLUI_Panel* p = new GLUI_Panel(pai, "Material");

    criarSlider(p, "Opacidade (0 = transparente):", &c.opacidade, 0.0f, 1.0f);
    criarSlider(p, "Brilho especular (shininess):", &c.brilho, 1.0f, 128.0f, "%.0f");
    criarSlider(p, "Intensidade especular:", &c.intensidadeEspecular, 0.0f, 1.0f);

    new GLUI_StaticText(p, "Cor do objeto solido:");
    criarSpinner(p, "R:", &c.corObjeto[0], 0.0f, 1.0f);
    criarSpinner(p, "G:", &c.corObjeto[1], 0.0f, 1.0f);
    criarSpinner(p, "B:", &c.corObjeto[2], 0.0f, 1.0f);
}

void criarPainelSuperficie(GLUI_Node* pai)
{
    Configuracao& c = aplicacao().config;
    GLUI_Panel* p = new GLUI_Panel(pai, "Superficie / Textura");

    GLUI_RadioGroup* modo = new GLUI_RadioGroup(p, &c.modoSuperficie, ID_REDESENHAR, aoMudarControle);
    new GLUI_RadioButton(modo, "Solido (monocromatico)");
    new GLUI_RadioButton(modo, "Texturizado");
    new GLUI_RadioButton(modo, "Reflexao do ambiente");

    new GLUI_Separator(p);
    rotuloTextura = new GLUI_StaticText(p, "Textura: -");
    new GLUI_Button(p, "Escolher textura...", ID_ESCOLHER_TEXTURA, aoMudarControle);
    if (dialogoNativoDisponivel()) {
        new GLUI_Button(p, "Escolher textura (Windows)...", ID_ESCOLHER_TEXTURA_WINDOWS, aoMudarControle);
    }

    new GLUI_StaticText(p, "Coordenadas de textura:");
    GLUI_RadioGroup* uv = new GLUI_RadioGroup(p, &c.geradorUV, ID_GERADOR_UV, aoMudarControle);
    new GLUI_RadioButton(uv, "Do arquivo OBJ (vt)");
    new GLUI_RadioButton(uv, "Automatica esferica");
    new GLUI_RadioButton(uv, "Automatica cilindrica");
    new GLUI_RadioButton(uv, "Automatica planar");

    new GLUI_Separator(p);
    rotuloAmbiente = new GLUI_StaticText(p, "Ambiente: -");
    new GLUI_Button(p, "Escolher mapa de ambiente...", ID_ESCOLHER_AMBIENTE, aoMudarControle);
    new GLUI_Button(p, "Ambiente padrao", ID_AMBIENTE_PADRAO, aoMudarControle);
}

void criarPainelTransformacoes(GLUI_Node* pai)
{
    GLUI_Rollout* r = new GLUI_Rollout(pai, "Transformacoes 3D", false);

    GLUI_Listbox* eixo = new GLUI_Listbox(r, "Eixo:", &entradas.eixoRotacao);
    eixo->add_item(0, "X");
    eixo->add_item(1, "Y");
    eixo->add_item(2, "Z");
    new GLUI_EditText(r, "Angulo:", &entradas.angulo);
    new GLUI_Button(r, "Rotacionar", ID_ROTACIONAR, aoMudarControle);
    new GLUI_Separator(r);

    new GLUI_EditText(r, "Transl. X:", &entradas.translacao[0]);
    new GLUI_EditText(r, "Transl. Y:", &entradas.translacao[1]);
    new GLUI_EditText(r, "Transl. Z:", &entradas.translacao[2]);
    new GLUI_Button(r, "Transladar", ID_TRANSLADAR, aoMudarControle);
    new GLUI_Separator(r);

    new GLUI_EditText(r, "Escala X:", &entradas.escala[0]);
    new GLUI_EditText(r, "Escala Y:", &entradas.escala[1]);
    new GLUI_EditText(r, "Escala Z:", &entradas.escala[2]);
    new GLUI_Button(r, "Escalar", ID_ESCALAR, aoMudarControle);
    new GLUI_Separator(r);

    GLUI_Listbox* plano = new GLUI_Listbox(r, "Plano:", &entradas.planoReflexao);
    plano->add_item(0, "YZ (inverte X)");
    plano->add_item(1, "XZ (inverte Y)");
    plano->add_item(2, "XY (inverte Z)");
    new GLUI_Button(r, "Refletir", ID_REFLETIR, aoMudarControle);
    new GLUI_Separator(r);

    GLUI_Listbox* cis = new GLUI_Listbox(r, "Cisalhar:", &entradas.tipoCisalhamento);
    cis->add_item(0, "X += k*Y");
    cis->add_item(1, "X += k*Z");
    cis->add_item(2, "Y += k*X");
    cis->add_item(3, "Y += k*Z");
    cis->add_item(4, "Z += k*X");
    cis->add_item(5, "Z += k*Y");
    new GLUI_EditText(r, "Fator k:", &entradas.fatorCisalhamento);
    new GLUI_Button(r, "Cisalhar", ID_CISALHAR, aoMudarControle);
    new GLUI_Separator(r);

    rotuloTransformacoes = new GLUI_StaticText(r, "Transformacoes: 0");
    new GLUI_Checkbox(r, "Mostrar original (aramado)", &aplicacao().config.mostrarOriginal,
                      ID_REDESENHAR, aoMudarControle);
    new GLUI_Button(r, "Resetar transformacoes", ID_RESETAR_TRANSFORMACOES, aoMudarControle);
}

void criarPainelVisualizacao(GLUI_Node* pai)
{
    Configuracao& c = aplicacao().config;
    GLUI_Panel* p = new GLUI_Panel(pai, "Visualizacao");

    new GLUI_Checkbox(p, "Mostrar piso (mesa)", &c.mostrarPiso, ID_REDESENHAR, aoMudarControle);
    new GLUI_Checkbox(p, "Mostrar eixos", &c.mostrarEixos, ID_REDESENHAR, aoMudarControle);
    new GLUI_Button(p, "Resetar camera", ID_RESETAR_CAMERA, aoMudarControle);
    new GLUI_StaticText(p, "Mouse: arrastar = girar");
    new GLUI_StaticText(p, "Roda/botao do meio = zoom");
    new GLUI_StaticText(p, "Botao direito = menu");
}

// ---------------------------------------------------------------------------
// Menu de contexto (GLUT) e callbacks da janela 3D
// ---------------------------------------------------------------------------
void menuCallback(int opcao)
{
    Configuracao& c = aplicacao().config;

    switch (opcao) {
        case MENU_FLAT:      c.sombreamento = SOMBREAMENTO_FLAT; break;
        case MENU_SMOOTH:    c.sombreamento = SOMBREAMENTO_SMOOTH; break;
        case MENU_SOLIDO:    c.modoSuperficie = SUPERFICIE_SOLIDA; break;
        case MENU_TEXTURA:   c.modoSuperficie = SUPERFICIE_TEXTURA; break;
        case MENU_REFLEXAO:  c.modoSuperficie = SUPERFICIE_REFLEXAO; break;
        case MENU_LUZ_1:     c.luzes[0].ligada = !c.luzes[0].ligada; break;
        case MENU_LUZ_2:     c.luzes[1].ligada = !c.luzes[1].ligada; break;
        case MENU_RESETAR_CAMERA: aplicacao().camera.resetar(); break;
        case MENU_SAIR:      std::exit(0);
        default: break;
    }

    sincronizarControles();
}

void exibir()
{
    aplicacao().desenhar();
    glutSwapBuffers();
}

void redimensionar(int largura, int altura)
{
    aplicacao().redimensionar(largura, altura);
}

void mouse(int botao, int estado, int x, int y)
{
    if (estado == GLUT_DOWN) {
        botaoArrastando = botao;
        ultimoX = x;
        ultimoY = y;
    } else {
        botaoArrastando = -1;
    }
}

void movimento(int x, int y)
{
    Camera& camera = aplicacao().camera;
    const int dx = x - ultimoX;
    const int dy = y - ultimoY;
    ultimoX = x;
    ultimoY = y;

    if (botaoArrastando == GLUT_LEFT_BUTTON) {
        camera.girar(-dx * 0.4f, dy * 0.4f);
    } else if (botaoArrastando == GLUT_MIDDLE_BUTTON) {
        camera.aproximar(1.0f + dy * 0.01f);
    }

    aplicacao().solicitarRedesenho();
}

void rodaMouse(int, int direcao, int, int)
{
    aplicacao().camera.aproximar(direcao > 0 ? 0.9f : 1.1f);
    aplicacao().solicitarRedesenho();
}

void teclado(unsigned char tecla, int, int)
{
    Camera& camera = aplicacao().camera;

    switch (tecla) {
        case 'q': case 'Q': case 27:
            std::exit(0);
        case '+': case '=':
            camera.aproximar(0.9f);
            break;
        case '-': case '_':
            camera.aproximar(1.1f);
            break;
        case 'r': case 'R':
            camera.resetar();
            break;
        default:
            return;
    }

    aplicacao().solicitarRedesenho();
}

void teclasEspeciais(int tecla, int, int)
{
    Camera& camera = aplicacao().camera;

    switch (tecla) {
        case GLUT_KEY_LEFT:  camera.girar(-5.0f, 0.0f); break;
        case GLUT_KEY_RIGHT: camera.girar(5.0f, 0.0f); break;
        case GLUT_KEY_UP:    camera.girar(0.0f, 5.0f); break;
        case GLUT_KEY_DOWN:  camera.girar(0.0f, -5.0f); break;
        default: return;
    }

    aplicacao().solicitarRedesenho();
}

}

void criarMenu()
{
    glutSetWindow(aplicacao().janela);

    const int sombreamento = glutCreateMenu(menuCallback);
    glutAddMenuEntry("Flat", MENU_FLAT);
    glutAddMenuEntry("Smooth (Gouraud)", MENU_SMOOTH);

    const int superficie = glutCreateMenu(menuCallback);
    glutAddMenuEntry("Solido", MENU_SOLIDO);
    glutAddMenuEntry("Texturizado", MENU_TEXTURA);
    glutAddMenuEntry("Reflexao do ambiente", MENU_REFLEXAO);

    glutCreateMenu(menuCallback);
    glutAddSubMenu("Sombreamento", sombreamento);
    glutAddSubMenu("Superficie", superficie);
    glutAddMenuEntry("Ligar/desligar luz 1", MENU_LUZ_1);
    glutAddMenuEntry("Ligar/desligar luz 2", MENU_LUZ_2);
    glutAddMenuEntry("Resetar camera", MENU_RESETAR_CAMERA);
    glutAddMenuEntry("Sair", MENU_SAIR);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void registrarCallbacksJanela()
{
    glutSetWindow(aplicacao().janela);

    glutDisplayFunc(exibir);
    glutMotionFunc(movimento);
    glutMouseWheelFunc(rodaMouse);
    // Registrados pela GLUI para que ela continue recebendo os eventos.
    GLUI_Master.set_glutReshapeFunc(redimensionar);
    GLUI_Master.set_glutMouseFunc(mouse);
    GLUI_Master.set_glutKeyboardFunc(teclado);
    GLUI_Master.set_glutSpecialFunc(teclasEspeciais);
    GLUI_Master.set_glutIdleFunc(nullptr);
}

void criarInterface()
{
    // Janela de controles ao lado da janela 3D.
    painel = GLUI_Master.create_glui("Controles - Visualizador 3D", 0, 860, 20);

    criarPainelArquivo(painel);
    criarPainelIluminacao(painel);
    criarPainelSombreamento(painel);

    new GLUI_Column(painel, true);

    criarPainelMaterial(painel);
    criarPainelSuperficie(painel);

    new GLUI_Column(painel, true);

    criarPainelTransformacoes(painel);
    criarPainelVisualizacao(painel);

    rotuloStatus = new GLUI_StaticText(painel, "");
    new GLUI_Button(painel, "Sair", ID_SAIR, aoMudarControle);

    painel->set_main_gfx_window(aplicacao().janela);

    aplicacao().aoAtualizarInterface = atualizarInterface;
    atualizarInterface();
}

void atualizarInterface()
{
    if (painel == nullptr) {
        return;
    }

    const Aplicacao& app = aplicacao();

    rotuloModelo->set_text(("Modelo: " + encurtar(nomeArquivo(app.caminhoModelo), 30)).c_str());
    rotuloMalha->set_text((std::to_string(app.malha.triangulos.size()) + " triangulos | vt: " +
                           (app.malha.possuiCoordTextura() ? "sim" : "nao (gerado)")).c_str());
    rotuloTextura->set_text(("Textura: " + encurtar(app.texturaObjeto.valida()
                                                        ? app.texturaObjeto.descricao()
                                                        : std::string("nenhuma"), 26)).c_str());
    rotuloAmbiente->set_text(("Ambiente: " + encurtar(app.mapaAmbiente.valida()
                                                          ? app.mapaAmbiente.descricao()
                                                          : std::string("estudio (padrao)"), 26)).c_str());
    rotuloTransformacoes->set_text(("Transformacoes: " + std::to_string(app.transformacoes.quantidade())).c_str());
    rotuloStatus->set_text(encurtar(app.mensagem(), 44).c_str());

    for (const RotuloSlider& r : rotulosSliders) {
        char numero[32];
        std::snprintf(numero, sizeof(numero), r.formato, *r.valor);
        r.rotulo->set_text((std::string(r.nome) + " " + numero).c_str());
    }
}
