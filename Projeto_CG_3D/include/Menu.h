#ifndef MENU_H
#define MENU_H

// Interface grafica: painel de controles GLUI, menu de contexto GLUT
// (botao direito na janela 3D) e callbacks de mouse/teclado.

void criarMenu();
void criarInterface();
void registrarCallbacksJanela();

// Atualiza textos da interface (modelo, textura, status) a partir do estado.
void atualizarInterface();

#endif
