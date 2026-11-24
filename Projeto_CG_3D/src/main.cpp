// Visualizador de Objetos 3D com Interface Grafica Interativa
// OpenGL + GLUT (FreeGLUT) + GLUI
//
// Todo o uso e feito pela interface (painel "Controles" e menu do botao
// direito): nenhum parametro de linha de comando e necessario.

#include "Aplicacao.h"
#include "AutoTeste.h"
#include "Caminhos.h"
#include "Menu.h"
#include "OpenGL.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <string>

int main(int argc, char** argv)
{
    definirExecutavel(argv[0]);

#ifdef _WIN32
    // Sem isso, com escala de tela de 125%/150% o Windows "estica" a janela
    // e pode encolhe-la abaixo do tamanho pedido. Assim 1 pixel = 1 pixel.
    SetProcessDPIAware();
#endif

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

    // Janela inicial de 800x700 (requisito: no minimo 600x600), redimensionavel.
    // Em telas baixas a altura diminui para caber, mas nunca abaixo de 600.
    const int alturaTela = glutGet(GLUT_SCREEN_HEIGHT);
    const int altura = alturaTela > 0 ? std::clamp(alturaTela - 140, 600, 700) : 700;
    glutInitWindowPosition(40, 40);
    glutInitWindowSize(800, altura);

    Aplicacao& app = aplicacao();
    app.janela = glutCreateWindow("Visualizador 3D - OpenGL/GLUT/GLUI");

    registrarCallbacksJanela();
    criarMenu();

    app.inicializar();
    criarInterface();

    // Opcao apenas para testes automatizados (ver docs/DOCUMENTACAO.md).
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--autoteste") == 0) {
            const std::string pasta = (i + 1 < argc) ? argv[i + 1] : caminhoDoProjeto("build/autoteste");
            std::cout << "Executando autoteste. Capturas em: " << pasta << std::endl;
            iniciarAutoteste(pasta);
        }
    }

    glutMainLoop();
    return 0;
}
