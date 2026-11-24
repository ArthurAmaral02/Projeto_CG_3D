#include "Aplicacao.h"

#include "ArquivoOBJ.h"
#include "Caminhos.h"
#include "OpenGL.h"

#include <algorithm>
#include <iostream>

const char* const MODELO_PADRAO = "modelos/bule.obj";
const char* const TEXTURA_PADRAO = "texturas/madeira.jpg";

Aplicacao& aplicacao()
{
    static Aplicacao instancia;
    return instancia;
}

void Aplicacao::informar(const std::string& texto)
{
    ultimaMensagem = texto;
    std::cout << texto << std::endl;

    if (aoAtualizarInterface != nullptr) {
        aoAtualizarInterface();
    }
}

void Aplicacao::inicializar()
{
    visualizacao.inicializar();

    if (!carregarModelo(caminhoDoProjeto(MODELO_PADRAO))) {
        std::cerr << "Modelo padrao nao encontrado. Use o botao \"Abrir OBJ...\".\n";
    }

    // Se o modelo nao indicou textura propria, usa a textura padrao.
    if (!texturaObjeto.valida()) {
        carregarTextura(caminhoDoProjeto(TEXTURA_PADRAO));
    }
}

bool Aplicacao::carregarModelo(const std::string& caminho)
{
    std::string msg;
    Malha nova;

    if (!carregarObj(caminho, nova, msg)) {
        informar("Erro: " + msg);
        return false;
    }

    malha = std::move(nova);
    caminhoModelo = caminho;
    transformacoes.limpar();

    // Usa os vt do arquivo quando existem; sem vt, mantem a projecao
    // automatica escolhida (esferica se a escolha anterior era "OBJ (vt)").
    if (malha.possuiCoordTextura()) {
        config.geradorUV = static_cast<int>(GeradorUV::ArquivoOBJ);
    } else if (config.geradorUV == static_cast<int>(GeradorUV::ArquivoOBJ)) {
        config.geradorUV = static_cast<int>(GeradorUV::Esferica);
    }

    glutSetWindow(janela);
    visualizacao.definirMalha(malha);
    visualizacao.definirGeradorUV(malha, static_cast<GeradorUV>(config.geradorUV));

    if (!malha.possuiCoordTextura()) {
        msg += std::string(" (coordenadas de textura geradas: projecao ") +
               nomeGeradorUV(static_cast<GeradorUV>(config.geradorUV)) + ")";
    }
    informar(msg);

    // Textura indicada pelo .mtl ou com o mesmo nome do .obj.
    if (!malha.arquivoTextura.empty()) {
        carregarTextura(malha.arquivoTextura);
    }

    solicitarRedesenho();
    return true;
}

bool Aplicacao::carregarTextura(const std::string& caminho)
{
    std::string msg;
    glutSetWindow(janela);

    if (!texturaObjeto.carregarArquivo(caminho, msg)) {
        informar("Erro: " + msg);
        return false;
    }

    informar(msg);
    solicitarRedesenho();
    return true;
}

bool Aplicacao::carregarMapaAmbiente(const std::string& caminho)
{
    std::string msg;
    glutSetWindow(janela);

    if (!mapaAmbiente.carregarArquivo(caminho, msg)) {
        informar("Erro: " + msg);
        return false;
    }

    informar("Mapa de ambiente: " + msg);
    solicitarRedesenho();
    return true;
}

bool Aplicacao::salvarModelo(const std::string& caminho)
{
    // As transformacoes sao feitas no espaco normalizado (objeto no raio 1).
    // Para salvar nas unidades originais: M = N^-1 * T * N.
    const Vetor3 c = malha.centro();
    const double r = std::max(malha.raio(), 1e-6f);
    const Matriz4 normalizar = Matriz4::escala(1.0 / r, 1.0 / r, 1.0 / r) * Matriz4::translacao(-c.x, -c.y, -c.z);
    const Matriz4 desnormalizar = Matriz4::translacao(c.x, c.y, c.z) * Matriz4::escala(r, r, r);

    std::string msg;
    const bool ok = salvarObj(caminho, malha,
                              desnormalizar * transformacoes.matrizAcumulada() * normalizar, msg);
    informar(ok ? msg : "Erro: " + msg);
    return ok;
}

void Aplicacao::aplicarGeradorUV()
{
    visualizacao.definirGeradorUV(malha, static_cast<GeradorUV>(config.geradorUV));

    const GeradorUV usado = visualizacao.geradorUVEmUso();
    if (static_cast<int>(usado) != config.geradorUV) {
        config.geradorUV = static_cast<int>(usado);
        informar("O modelo nao possui vt: usando projecao esferica automatica.");
    }

    solicitarRedesenho();
}

void Aplicacao::resetarTransformacoes()
{
    transformacoes.limpar();
    informar("Transformacoes resetadas.");
    solicitarRedesenho();
}

void Aplicacao::desenhar()
{
    const Textura& ambiente = mapaAmbiente.valida() ? mapaAmbiente : visualizacao.mapaAmbientePadrao();

    visualizacao.desenhar(config, camera, transformacoes.matrizAcumulada(), texturaObjeto, ambiente);
}

void Aplicacao::redimensionar(int largura, int altura)
{
    visualizacao.redimensionar(largura, altura);
    solicitarRedesenho();
}

void Aplicacao::solicitarRedesenho() const
{
    if (janela != 0) {
        glutPostWindowRedisplay(janela);
    }
}
