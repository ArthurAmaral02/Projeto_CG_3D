#ifndef TRANSFORMACOES_H
#define TRANSFORMACOES_H

#include "Matriz4.h"

#include <string>
#include <vector>

// Transformacoes geometricas 3D acumuladas sobre o objeto (versao 3D das
// transformacoes 2D do projeto original: rotacao, translacao, escala,
// reflexao e cisalhamento).
//
// Cada nova transformacao e aplicada sobre o resultado das anteriores, no
// espaco do mundo: M = Tn * ... * T2 * T1.

enum class TipoTransformacao {
    Rotacao,
    Translacao,
    Escala,
    Reflexao,
    Cisalhamento
};

struct Transformacao {
    TipoTransformacao tipo = TipoTransformacao::Translacao;
    int eixo = 0;            // rotacao: eixo; reflexao: plano; cisalhamento: eixo alvo
    int eixoReferencia = 1;  // cisalhamento: eixo de referencia
    double valores[3] = {0.0, 0.0, 0.0};

    Matriz4 matriz() const;
    std::string descricao() const;
};

class PilhaTransformacoes {
public:
    void rotacionar(int eixo, double graus);
    void transladar(double x, double y, double z);
    void escalar(double x, double y, double z);
    void refletir(int plano);
    void cisalhar(int eixoAlvo, int eixoReferencia, double fator);

    void limpar();
    bool vazia() const { return lista.empty(); }
    std::size_t quantidade() const { return lista.size(); }
    const std::vector<Transformacao>& itens() const { return lista; }

    // Matriz acumulada (identidade quando nao ha transformacoes).
    const Matriz4& matrizAcumulada() const { return acumulada; }

private:
    void adicionar(const Transformacao& t);

    std::vector<Transformacao> lista;
    Matriz4 acumulada;
};

#endif
