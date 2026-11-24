#include "Matriz4.h"

#include <cmath>

namespace {
const double PI = 3.14159265358979323846;
}

Matriz4::Matriz4()
{
    for (int i = 0; i < 16; ++i) {
        m[i] = (i % 5 == 0) ? 1.0 : 0.0;
    }
}

Matriz4 Matriz4::identidade()
{
    return Matriz4();
}

Matriz4 Matriz4::translacao(double x, double y, double z)
{
    Matriz4 r;
    r(0, 3) = x;
    r(1, 3) = y;
    r(2, 3) = z;
    return r;
}

Matriz4 Matriz4::escala(double x, double y, double z)
{
    Matriz4 r;
    r(0, 0) = x;
    r(1, 1) = y;
    r(2, 2) = z;
    return r;
}

Matriz4 Matriz4::rotacao(int eixo, double graus)
{
    const double rad = graus * PI / 180.0;
    const double c = std::cos(rad);
    const double s = std::sin(rad);

    Matriz4 r;

    switch (eixo) {
        case 0:  // X
            r(1, 1) = c;  r(1, 2) = -s;
            r(2, 1) = s;  r(2, 2) = c;
            break;
        case 1:  // Y
            r(0, 0) = c;  r(0, 2) = s;
            r(2, 0) = -s; r(2, 2) = c;
            break;
        default: // Z
            r(0, 0) = c;  r(0, 1) = -s;
            r(1, 0) = s;  r(1, 1) = c;
            break;
    }

    return r;
}

Matriz4 Matriz4::reflexao(int plano)
{
    switch (plano) {
        case 0:  return escala(-1.0, 1.0, 1.0);
        case 1:  return escala(1.0, -1.0, 1.0);
        default: return escala(1.0, 1.0, -1.0);
    }
}

Matriz4 Matriz4::cisalhamento(int eixoAlvo, int eixoReferencia, double fator)
{
    Matriz4 r;

    if (eixoAlvo != eixoReferencia &&
        eixoAlvo >= 0 && eixoAlvo < 3 &&
        eixoReferencia >= 0 && eixoReferencia < 3) {
        r(eixoAlvo, eixoReferencia) = fator;
    }

    return r;
}

Matriz4 Matriz4::operator*(const Matriz4& o) const
{
    Matriz4 r;

    for (int l = 0; l < 4; ++l) {
        for (int c = 0; c < 4; ++c) {
            double soma = 0.0;
            for (int k = 0; k < 4; ++k) {
                soma += (*this)(l, k) * o(k, c);
            }
            r(l, c) = soma;
        }
    }

    return r;
}

Vetor3 Matriz4::transformarPonto(const Vetor3& p) const
{
    const double x = (*this)(0, 0) * p.x + (*this)(0, 1) * p.y + (*this)(0, 2) * p.z + (*this)(0, 3);
    const double y = (*this)(1, 0) * p.x + (*this)(1, 1) * p.y + (*this)(1, 2) * p.z + (*this)(1, 3);
    const double z = (*this)(2, 0) * p.x + (*this)(2, 1) * p.y + (*this)(2, 2) * p.z + (*this)(2, 3);
    const double w = (*this)(3, 0) * p.x + (*this)(3, 1) * p.y + (*this)(3, 2) * p.z + (*this)(3, 3);

    const double inv = (std::fabs(w) > 1e-12) ? 1.0 / w : 1.0;

    return {static_cast<float>(x * inv), static_cast<float>(y * inv), static_cast<float>(z * inv)};
}

Vetor3 Matriz4::transformarDirecao(const Vetor3& d) const
{
    return {
        static_cast<float>((*this)(0, 0) * d.x + (*this)(0, 1) * d.y + (*this)(0, 2) * d.z),
        static_cast<float>((*this)(1, 0) * d.x + (*this)(1, 1) * d.y + (*this)(1, 2) * d.z),
        static_cast<float>((*this)(2, 0) * d.x + (*this)(2, 1) * d.y + (*this)(2, 2) * d.z)
    };
}

double Matriz4::determinante3x3() const
{
    const Matriz4& a = *this;

    return a(0, 0) * (a(1, 1) * a(2, 2) - a(1, 2) * a(2, 1))
         - a(0, 1) * (a(1, 0) * a(2, 2) - a(1, 2) * a(2, 0))
         + a(0, 2) * (a(1, 0) * a(2, 1) - a(1, 1) * a(2, 0));
}

Matriz4 Matriz4::inversaTranspostaNormal() const
{
    const Matriz4& a = *this;
    const double det = determinante3x3();

    Matriz4 r;

    if (std::fabs(det) < 1e-15) {
        return r;  // matriz degenerada: mantem as normais
    }

    const double inv = 1.0 / det;

    // A inversa-transposta e a matriz de cofatores dividida pelo determinante.
    r(0, 0) = (a(1, 1) * a(2, 2) - a(1, 2) * a(2, 1)) * inv;
    r(0, 1) = (a(1, 2) * a(2, 0) - a(1, 0) * a(2, 2)) * inv;
    r(0, 2) = (a(1, 0) * a(2, 1) - a(1, 1) * a(2, 0)) * inv;
    r(1, 0) = (a(0, 2) * a(2, 1) - a(0, 1) * a(2, 2)) * inv;
    r(1, 1) = (a(0, 0) * a(2, 2) - a(0, 2) * a(2, 0)) * inv;
    r(1, 2) = (a(0, 1) * a(2, 0) - a(0, 0) * a(2, 1)) * inv;
    r(2, 0) = (a(0, 1) * a(1, 2) - a(0, 2) * a(1, 1)) * inv;
    r(2, 1) = (a(0, 2) * a(1, 0) - a(0, 0) * a(1, 2)) * inv;
    r(2, 2) = (a(0, 0) * a(1, 1) - a(0, 1) * a(1, 0)) * inv;

    return r;
}
