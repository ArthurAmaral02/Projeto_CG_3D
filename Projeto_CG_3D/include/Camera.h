#ifndef CAMERA_H
#define CAMERA_H

#include "Vetor.h"

// Camera orbital com projecao perspectiva: gira em torno do alvo
// (arrastar com o botao esquerdo) e aproxima/afasta (roda do mouse,
// botao do meio ou teclas + e -).
class Camera {
public:
    float azimute = 35.0f;     // graus, em torno do eixo Y
    float elevacao = 20.0f;    // graus, acima do plano XZ
    float distancia = 3.3f;
    float campoVisao = 45.0f;  // fovy em graus
    Vetor3 alvo;

    void resetar();

    // Projecao perspectiva com a proporcao da janela (sem distorcao).
    void aplicarProjecao(int largura, int altura) const;
    void aplicarVisao() const;

    Vetor3 posicao() const;

    void girar(float deltaAzimute, float deltaElevacao);
    void aproximar(float fator);  // < 1 aproxima, > 1 afasta
};

#endif
