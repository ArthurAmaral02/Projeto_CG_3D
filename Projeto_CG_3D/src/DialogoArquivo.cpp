#include "DialogoArquivo.h"

#include "Caminhos.h"
#include "OpenGL.h"

#include <GL/glui.h>

#include <system_error>
#include <filesystem>

#ifdef _WIN32
#include <commdlg.h>
#endif

namespace {

GLUI* janela = nullptr;
GLUI_StaticText* rotuloTitulo = nullptr;
GLUI_StaticText* rotuloPasta = nullptr;
GLUI_StaticText* rotuloAviso = nullptr;
GLUI_FileBrowser* navegador = nullptr;

TipoArquivo tipoAtual = TipoArquivo::ModeloOBJ;
RetornoArquivo retornoAtual = nullptr;

// A GLUI_StaticText nao quebra linhas: mostra o final de textos longos.
std::string encurtar(const std::string& texto, std::size_t maximo)
{
    if (texto.size() <= maximo) {
        return texto;
    }
    return "..." + texto.substr(texto.size() - (maximo - 3));
}

void mudarPasta(const std::string& pasta)
{
    std::error_code ec;
    std::filesystem::current_path(std::filesystem::path(pasta), ec);
}

void atualizarListagem()
{
    navegador->fbreaddir(".");
    rotuloPasta->set_text(("Pasta: " + encurtar(diretorioAtual(), 52)).c_str());
}

void aoEscolherItem(int)
{
    std::string item = navegador->get_file();

    if (item.empty()) {
        return;
    }

    // No Linux a GLUI marca pastas com "/" no final e as entrega como
    // arquivo; aqui a navegacao e tratada nos dois sistemas.
    if (item.back() == '/' || item.back() == '\\' || diretorioExiste(item)) {
        while (!item.empty() && (item.back() == '/' || item.back() == '\\')) {
            item.pop_back();
        }
        mudarPasta(item.empty() ? "/" : item);
        atualizarListagem();
        return;
    }

    const std::string caminho = juntarCaminho(diretorioAtual(), item);

    if (!extensaoAceita(tipoAtual, caminho)) {
        rotuloAviso->set_text(tipoAtual == TipoArquivo::ModeloOBJ
                                  ? "Escolha um arquivo .obj"
                                  : "Escolha uma imagem .png, .bmp ou .jpg");
        return;
    }

    janela->hide();

    if (retornoAtual != nullptr) {
        retornoAtual(caminho);
    }
}

void aoClicarPastaProjeto(int)
{
    mudarPasta(caminhoDoProjeto(tipoAtual == TipoArquivo::ModeloOBJ ? "modelos" : "texturas"));
    atualizarListagem();
}

void aoClicarPastaAcima(int)
{
    mudarPasta("..");
    atualizarListagem();
}

void aoCancelar(int)
{
    janela->hide();
}

void criarJanela()
{
    janela = GLUI_Master.create_glui("Abrir arquivo", 0, 940, 120);

    rotuloTitulo = new GLUI_StaticText(janela, "");
    rotuloPasta = new GLUI_StaticText(janela, "");

    navegador = new GLUI_FileBrowser(janela, "Duplo clique: abre pasta / escolhe arquivo",
                                     GLUI_PANEL_EMBOSSED, 0, aoEscolherItem);
    navegador->set_w(330);
    navegador->set_h(300);
    navegador->set_allow_change_dir(1);

    rotuloAviso = new GLUI_StaticText(janela, "");

    GLUI_Panel* botoes = new GLUI_Panel(janela, "", GLUI_PANEL_NONE);
    new GLUI_Button(botoes, "Pasta acima", 0, aoClicarPastaAcima);
    new GLUI_Column(botoes, false);
    new GLUI_Button(botoes, "Pasta do projeto", 0, aoClicarPastaProjeto);
    new GLUI_Column(botoes, false);
    new GLUI_Button(botoes, "Cancelar", 0, aoCancelar);
}

}

bool extensaoAceita(TipoArquivo tipo, const std::string& caminho)
{
    return tipo == TipoArquivo::ModeloOBJ ? ehArquivoOBJ(caminho) : ehImagemSuportada(caminho);
}

void abrirNavegadorGLUI(TipoArquivo tipo, RetornoArquivo retorno)
{
    const int janelaAnterior = glutGetWindow();

    if (janela == nullptr) {
        // Comeca na pasta de modelos/texturas do projeto.
        mudarPasta(caminhoDoProjeto(tipo == TipoArquivo::ModeloOBJ ? "modelos" : "texturas"));
        criarJanela();
    }

    tipoAtual = tipo;
    retornoAtual = retorno;

    rotuloTitulo->set_text(tipo == TipoArquivo::ModeloOBJ
                               ? "Abrir modelo 3D (.obj)"
                               : "Abrir imagem (.png, .bmp, .jpg)");
    rotuloAviso->set_text("");
    atualizarListagem();
    janela->show();

    if (janelaAnterior > 0) {
        glutSetWindow(janelaAnterior);
    }
}

bool dialogoNativoDisponivel()
{
#ifdef _WIN32
    return true;
#else
    return false;
#endif
}

bool abrirDialogoNativo(TipoArquivo tipo, std::string& caminhoEscolhido)
{
#ifdef _WIN32
    char arquivo[MAX_PATH] = "";
    const std::string pastaInicial =
        caminhoDoProjeto(tipo == TipoArquivo::ModeloOBJ ? "modelos" : "texturas");

    OPENFILENAMEA ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = GetActiveWindow();
    ofn.lpstrFile = arquivo;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrInitialDir = pastaInicial.c_str();
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

    if (tipo == TipoArquivo::ModeloOBJ) {
        ofn.lpstrFilter = "Modelos Wavefront (*.obj)\0*.obj\0Todos os arquivos (*.*)\0*.*\0";
        ofn.lpstrTitle = "Abrir modelo 3D";
    } else {
        ofn.lpstrFilter = "Imagens (*.png;*.bmp;*.jpg;*.jpeg)\0*.png;*.bmp;*.jpg;*.jpeg\0"
                          "Todos os arquivos (*.*)\0*.*\0";
        ofn.lpstrTitle = "Abrir imagem de textura";
    }

    if (!GetOpenFileNameA(&ofn)) {
        return false;
    }

    caminhoEscolhido = arquivo;
    return true;
#else
    (void)tipo;
    (void)caminhoEscolhido;
    return false;
#endif
}
