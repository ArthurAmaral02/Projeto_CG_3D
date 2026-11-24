#include "Camera.h"

#include "OpenGL.h"

#include <algorithm>
#include <cmath>

namespace {
const float GRAUS_PARA_RAD = 3.14159265358979f / 180.0f;
}

void Camera::resetar()
{
    *this = Camera();
}

void Camera::aplicarProjecao(int largura, int altura) const
{
    const double aspecto = static_cast<double>(std::max(largura, 1)) / std::max(altura, 1);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(campoVisao, aspecto, 0.05, 200.0);
    glMatrixMode(GL_MODELVIEW);
}

Vetor3 Camera::posicao() const
{
    const float a = azimute * GRAUS_PARA_RAD;
    const float e = elevacao * GRAUS_PARA_RAD;

    return alvo + Vetor3(distancia * std::cos(e) * std::sin(a),
                         distancia * std::sin(e),
                         distancia * std::cos(e) * std::cos(a));
}

void Camera::aplicarVisao() const
{
    const Vetor3 olho = posicao();
    gluLookAt(olho.x, olho.y, olho.z, alvo.x, alvo.y, alvo.z, 0.0, 1.0, 0.0);
}

void Camera::girar(float deltaAzimute, float deltaElevacao)
{
    azimute = std::fmod(azimute + deltaAzimute, 360.0f);
    elevacao = std::clamp(elevacao + deltaElevacao, -89.0f, 89.0f);
}

void Camera::aproximar(float fator)
{
    distancia = std::clamp(distancia * fator, 0.3f, 60.0f);
}
