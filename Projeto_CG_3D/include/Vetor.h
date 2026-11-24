#ifndef VETOR_H
#define VETOR_H

#include <cmath>

// Vetores 2D e 3D usados pela malha, pela camera e pelas transformacoes.

struct Vetor2 {
    float u = 0.0f;
    float v = 0.0f;
};

struct Vetor3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    Vetor3() = default;
    Vetor3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

    Vetor3 operator+(const Vetor3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vetor3 operator-(const Vetor3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vetor3 operator*(float k) const { return {x * k, y * k, z * k}; }
    Vetor3& operator+=(const Vetor3& o) { x += o.x; y += o.y; z += o.z; return *this; }

    float comprimento() const { return std::sqrt(x * x + y * y + z * z); }

    // Retorna o vetor unitario; vetores nulos continuam nulos.
    Vetor3 normalizado() const {
        const float c = comprimento();
        return (c > 1e-12f) ? Vetor3(x / c, y / c, z / c) : Vetor3();
    }
};

inline float produtoEscalar(const Vetor3& a, const Vetor3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline Vetor3 produtoVetorial(const Vetor3& a, const Vetor3& b) {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

#endif
