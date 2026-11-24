#include "Transformacoes.h"

#include <sstream>

namespace {
const char* NOMES_EIXOS[] = {"X", "Y", "Z"};
const char* NOMES_PLANOS[] = {"YZ", "XZ", "XY"};

const char* nomeEixo(int eixo)
{
    return (eixo >= 0 && eixo < 3) ? NOMES_EIXOS[eixo] : "?";
}
}

Matriz4 Transformacao::matriz() const
{
    switch (tipo) {
        case TipoTransformacao::Rotacao:
            return Matriz4::rotacao(eixo, valores[0]);
        case TipoTransformacao::Translacao:
            return Matriz4::translacao(valores[0], valores[1], valores[2]);
        case TipoTransformacao::Escala:
            return Matriz4::escala(valores[0], valores[1], valores[2]);
        case TipoTransformacao::Reflexao:
            return Matriz4::reflexao(eixo);
        case TipoTransformacao::Cisalhamento:
            return Matriz4::cisalhamento(eixo, eixoReferencia, valores[0]);
    }

    return Matriz4::identidade();
}

std::string Transformacao::descricao() const
{
    std::ostringstream s;

    switch (tipo) {
        case TipoTransformacao::Rotacao:
            s << "Rotacao " << valores[0] << " graus em " << nomeEixo(eixo);
            break;
        case TipoTransformacao::Translacao:
            s << "Translacao (" << valores[0] << ", " << valores[1] << ", " << valores[2] << ")";
            break;
        case TipoTransformacao::Escala:
            s << "Escala (" << valores[0] << ", " << valores[1] << ", " << valores[2] << ")";
            break;
        case TipoTransformacao::Reflexao:
            s << "Reflexao no plano " << ((eixo >= 0 && eixo < 3) ? NOMES_PLANOS[eixo] : "?");
            break;
        case TipoTransformacao::Cisalhamento:
            s << "Cisalhamento " << nomeEixo(eixo) << " += " << valores[0]
              << " * " << nomeEixo(eixoReferencia);
            break;
    }

    return s.str();
}

void PilhaTransformacoes::adicionar(const Transformacao& t)
{
    lista.push_back(t);
    // A nova transformacao age depois das anteriores (pre-multiplicacao).
    acumulada = t.matriz() * acumulada;
}

void PilhaTransformacoes::rotacionar(int eixo, double graus)
{
    Transformacao t;
    t.tipo = TipoTransformacao::Rotacao;
    t.eixo = eixo;
    t.valores[0] = graus;
    adicionar(t);
}

void PilhaTransformacoes::transladar(double x, double y, double z)
{
    Transformacao t;
    t.tipo = TipoTransformacao::Translacao;
    t.valores[0] = x;
    t.valores[1] = y;
    t.valores[2] = z;
    adicionar(t);
}

void PilhaTransformacoes::escalar(double x, double y, double z)
{
    Transformacao t;
    t.tipo = TipoTransformacao::Escala;
    t.valores[0] = x;
    t.valores[1] = y;
    t.valores[2] = z;
    adicionar(t);
}

void PilhaTransformacoes::refletir(int plano)
{
    Transformacao t;
    t.tipo = TipoTransformacao::Reflexao;
    t.eixo = plano;
    adicionar(t);
}

void PilhaTransformacoes::cisalhar(int eixoAlvo, int eixoReferencia, double fator)
{
    Transformacao t;
    t.tipo = TipoTransformacao::Cisalhamento;
    t.eixo = eixoAlvo;
    t.eixoReferencia = eixoReferencia;
    t.valores[0] = fator;
    adicionar(t);
}

void PilhaTransformacoes::limpar()
{
    lista.clear();
    acumulada = Matriz4::identidade();
}
