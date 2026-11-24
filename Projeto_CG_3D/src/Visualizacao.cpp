#include "Visualizacao.h"

#include "OpenGL.h"
#include "OrdenacaoProfundidade.h"

#include <algorithm>
#include <cmath>

namespace {

const float TAMANHO_PISO = 6.0f;  // meia largura do piso
const int DIVISOES_PISO = 48;     // subdivisao para a luz pontual "espalhar" no piso

void definirMaterial(const float difusa[4], float especular, float brilho)
{
    const float ambiente[4] = {difusa[0] * 0.35f, difusa[1] * 0.35f, difusa[2] * 0.35f, difusa[3]};
    const float spec[4] = {especular, especular, especular, difusa[3]};
    const float emissao[4] = {0.0f, 0.0f, 0.0f, 1.0f};

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambiente);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, difusa);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emissao);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, std::clamp(brilho, 0.0f, 128.0f));
}

}

void Visualizacao::inicializar()
{
    glClearColor(0.16f, 0.17f, 0.20f, 1.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_NORMALIZE);  // escalas e cisalhamentos alteram o tamanho das normais
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    // Modelo de iluminacao de Phong do pipeline fixo.
    const float ambienteGlobal[4] = {0.10f, 0.10f, 0.11f, 1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambienteGlobal);
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);  // especular com observador real
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);      // malhas abertas (ex.: bule)
    // Especular somada depois da textura: o brilho nao e escurecido por ela.
    glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    texturaPiso.criar(gerarTexturaPiso(256), "piso de madeira");
    ambientePadrao.criar(gerarMapaAmbienteEstudio(512), "estudio (padrao)");
}

void Visualizacao::definirMalha(const Malha& malha)
{
    const std::size_t n = malha.triangulos.size();

    posicoes.resize(n * 9);
    normaisSuaves.resize(n * 9);
    normaisPlanas.resize(n * 9);

    for (std::size_t f = 0; f < n; ++f) {
        const Triangulo& t = malha.triangulos[f];
        const Vetor3& nf = malha.normaisFace[f];

        for (int k = 0; k < 3; ++k) {
            const Vetor3& p = malha.vertices[t.v[k]];
            const Vetor3& nv = malha.normaisVertice[t.v[k]];
            const std::size_t i = f * 9 + k * 3;

            posicoes[i] = p.x;       posicoes[i + 1] = p.y;       posicoes[i + 2] = p.z;
            normaisSuaves[i] = nv.x; normaisSuaves[i + 1] = nv.y; normaisSuaves[i + 2] = nv.z;
            normaisPlanas[i] = nf.x; normaisPlanas[i + 1] = nf.y; normaisPlanas[i + 2] = nf.z;
        }
    }

    // Centraliza na origem e escala para o objeto caber numa esfera de raio 1.
    const Vetor3 c = malha.centro();
    const float r = std::max(malha.raio(), 1e-6f);
    normalizacao = Matriz4::escala(1.0 / r, 1.0 / r, 1.0 / r) * Matriz4::translacao(-c.x, -c.y, -c.z);
    pisoY = (malha.minimo.y - c.y) / r - 0.002f;

    definirGeradorUV(malha, geradorAtual);
}

void Visualizacao::definirGeradorUV(const Malha& malha, GeradorUV gerador)
{
    const std::vector<Vetor2> uv = gerarCoordenadasTextura(malha, gerador, &geradorAtual);

    coordenadasUV.resize(uv.size() * 2);
    for (std::size_t i = 0; i < uv.size(); ++i) {
        coordenadasUV[i * 2] = uv[i].u;
        coordenadasUV[i * 2 + 1] = uv[i].v;
    }
}

void Visualizacao::redimensionar(int largura, int altura)
{
    larguraJanela = std::max(largura, 1);
    alturaJanela = std::max(altura, 1);
    glViewport(0, 0, larguraJanela, alturaJanela);
}

void Visualizacao::configurarLuzes(const Configuracao& config)
{
    glEnable(GL_LIGHTING);
    ultimoQuadro.luzesLigadas = 0;

    for (int i = 0; i < NUMERO_LUZES; ++i) {
        const GLenum luz = GL_LIGHT0 + i;
        const FonteLuz& f = config.luzes[i];

        if (!f.ligada) {
            glDisable(luz);
            continue;
        }

        // w = 1: luz pontual. A posicao e transformada pela matriz de visao
        // atual, ou seja, fica fixa no mundo enquanto a camera se move.
        const float posicao[4] = {f.posicao[0], f.posicao[1], f.posicao[2], 1.0f};
        const float difusa[4] = {f.cor[0], f.cor[1], f.cor[2], 1.0f};
        const float ambiente[4] = {f.cor[0] * 0.12f, f.cor[1] * 0.12f, f.cor[2] * 0.12f, 1.0f};

        glLightfv(luz, GL_POSITION, posicao);
        glLightfv(luz, GL_DIFFUSE, difusa);
        glLightfv(luz, GL_SPECULAR, difusa);
        glLightfv(luz, GL_AMBIENT, ambiente);
        glEnable(luz);
        ++ultimoQuadro.luzesLigadas;
    }
}

void Visualizacao::desenharPiso()
{
    const float difusa[4] = {0.95f, 0.95f, 0.95f, 1.0f};
    definirMaterial(difusa, 0.15f, 20.0f);

    glShadeModel(GL_SMOOTH);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texturaPiso.identificador());

    const float passo = 2.0f * TAMANHO_PISO / DIVISOES_PISO;
    const float repeticoes = 3.0f;

    glNormal3f(0.0f, 1.0f, 0.0f);

    for (int i = 0; i < DIVISOES_PISO; ++i) {
        glBegin(GL_TRIANGLE_STRIP);
        for (int j = 0; j <= DIVISOES_PISO; ++j) {
            for (int k = 1; k >= 0; --k) {
                const float x = -TAMANHO_PISO + (i + k) * passo;
                const float z = -TAMANHO_PISO + j * passo;
                glTexCoord2f((x + TAMANHO_PISO) / (2 * TAMANHO_PISO) * repeticoes,
                             (z + TAMANHO_PISO) / (2 * TAMANHO_PISO) * repeticoes);
                glVertex3f(x, pisoY, z);
            }
        }
        glEnd();
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}

void Visualizacao::desenharEixos()
{
    glDisable(GL_LIGHTING);
    glLineWidth(2.0f);

    glBegin(GL_LINES);
    glColor3f(0.9f, 0.2f, 0.2f); glVertex3f(0, 0, 0); glVertex3f(1.5f, 0, 0);
    glColor3f(0.2f, 0.8f, 0.2f); glVertex3f(0, 0, 0); glVertex3f(0, 1.5f, 0);
    glColor3f(0.2f, 0.4f, 1.0f); glVertex3f(0, 0, 0); glVertex3f(0, 0, 1.5f);
    glEnd();

    glLineWidth(1.0f);
    glEnable(GL_LIGHTING);
}

void Visualizacao::desenharMarcadoresLuz(const Configuracao& config)
{
    glDisable(GL_LIGHTING);

    for (int i = 0; i < NUMERO_LUZES; ++i) {
        const FonteLuz& f = config.luzes[i];

        if (f.ligada) {
            glColor3f(1.0f, 0.92f, 0.35f);
        } else {
            glColor3f(0.35f, 0.35f, 0.35f);
        }

        glPushMatrix();
        glTranslatef(f.posicao[0], f.posicao[1], f.posicao[2]);
        glutSolidSphere(0.07, 16, 12);
        glPopMatrix();

        // Linha pontilhada ate o piso para mostrar a posicao no espaco.
        glEnable(GL_LINE_STIPPLE);
        glLineStipple(2, 0xAAAA);
        glBegin(GL_LINES);
        glVertex3f(f.posicao[0], f.posicao[1], f.posicao[2]);
        glVertex3f(f.posicao[0], pisoY, f.posicao[2]);
        glEnd();
        glDisable(GL_LINE_STIPPLE);
    }

    glEnable(GL_LIGHTING);
}

void Visualizacao::desenharContornoOriginal()
{
    glDisable(GL_LIGHTING);
    glColor4f(0.75f, 0.78f, 0.85f, 1.0f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, posicoes.data());
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(posicoes.size() / 3));
    glDisableClientState(GL_VERTEX_ARRAY);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_LIGHTING);
}

void Visualizacao::desenharObjeto(const Configuracao& config,
                                  const Textura& texturaObjeto,
                                  const Textura& mapaAmbiente,
                                  bool orientacaoInvertida)
{
    const float alfa = std::clamp(config.opacidade, 0.0f, 1.0f);

    // Cor do material: solida usa a cor escolhida; textura e reflexao usam
    // branco para que as cores da imagem aparecam sem tingimento.
    float difusa[4] = {1.0f, 1.0f, 1.0f, alfa};
    if (config.modoSuperficie == SUPERFICIE_SOLIDA) {
        difusa[0] = config.corObjeto[0];
        difusa[1] = config.corObjeto[1];
        difusa[2] = config.corObjeto[2];
    }
    definirMaterial(difusa, config.intensidadeEspecular, config.brilho);
    ultimoQuadro.brilho = config.brilho;

    // Reflexoes (determinante negativo) invertem a ordem dos cantos na tela.
    glFrontFace(orientacaoInvertida ? GL_CW : GL_CCW);

    // Sombreamento: flat usa a normal da face; smooth usa normais medias por
    // vertice interpoladas (Gouraud).
    const bool flat = config.sombreamento == SOMBREAMENTO_FLAT;
    glShadeModel(flat ? GL_FLAT : GL_SMOOTH);
    ultimoQuadro.modeloSombreamento = flat ? GL_FLAT : GL_SMOOTH;

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, posicoes.data());
    glNormalPointer(GL_FLOAT, 0, flat ? normaisPlanas.data() : normaisSuaves.data());

    ultimoQuadro.texturaAtiva = false;
    ultimoQuadro.reflexaoEsferica = false;

    if (config.modoSuperficie == SUPERFICIE_TEXTURA && texturaObjeto.valida()) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texturaObjeto.identificador());
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2, GL_FLOAT, 0, coordenadasUV.data());
        ultimoQuadro.texturaAtiva = true;
    }
    else if (config.modoSuperficie == SUPERFICIE_REFLEXAO && mapaAmbiente.valida()) {
        // Mapeamento de reflexao: as coordenadas vem do vetor refletido no
        // espaco da camera (GL_SPHERE_MAP), simulando um metal espelhado.
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, mapaAmbiente.identificador());
        glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
        glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
        glEnable(GL_TEXTURE_GEN_S);
        glEnable(GL_TEXTURE_GEN_T);
        ultimoQuadro.reflexaoEsferica = true;
    }

    const GLsizei cantos = static_cast<GLsizei>(posicoes.size() / 3);

    if (alfa < 0.999f) {
        // Transparencia: mistura alfa, sem escrita no depth buffer e com os
        // triangulos ordenados de tras para frente a cada quadro.
        double modelView[16];
        glGetDoublev(GL_MODELVIEW_MATRIX, modelView);
        ordenarTriangulosPorProfundidade(posicoes, modelView, indicesOrdenados);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);

        GLboolean mascara = GL_TRUE;
        glGetBooleanv(GL_DEPTH_WRITEMASK, &mascara);
        ultimoQuadro.translucido = true;
        ultimoQuadro.escritaProfundidadeDesligada = (mascara == GL_FALSE);
        ultimoQuadro.triangulosOrdenados = indicesOrdenados.size() / 3;

        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indicesOrdenados.size()),
                       GL_UNSIGNED_INT, indicesOrdenados.data());

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    } else {
        ultimoQuadro.translucido = false;
        ultimoQuadro.escritaProfundidadeDesligada = false;
        ultimoQuadro.triangulosOrdenados = 0;
        glDrawArrays(GL_TRIANGLES, 0, cantos);
    }

    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glFrontFace(GL_CCW);
}

void Visualizacao::desenhar(const Configuracao& config,
                            const Camera& camera,
                            const Matriz4& transformacao,
                            const Textura& texturaObjeto,
                            const Textura& mapaAmbiente)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    camera.aplicarProjecao(larguraJanela, alturaJanela);

    ultimoQuadro.largura = larguraJanela;
    ultimoQuadro.altura = alturaJanela;
    glGetDoublev(GL_PROJECTION_MATRIX, ultimoQuadro.projecao);
    // Na perspectiva a linha w da matriz e (0, 0, -1, 0).
    ultimoQuadro.perspectiva = ultimoQuadro.projecao[11] == -1.0 && ultimoQuadro.projecao[15] == 0.0;

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    camera.aplicarVisao();

    configurarLuzes(config);

    // 1) Elementos opacos.
    if (config.mostrarPiso) {
        desenharPiso();
    }
    if (config.mostrarEixos) {
        desenharEixos();
    }
    if (config.mostrarLuzes) {
        desenharMarcadoresLuz(config);
    }

    if (posicoes.empty()) {
        return;
    }

    glPushMatrix();

    if (config.mostrarOriginal) {
        glPushMatrix();
        glMultMatrixd(normalizacao.m);
        desenharContornoOriginal();
        glPopMatrix();
    }

    // 2) Objeto (por ultimo, pois pode ser translucido).
    // Matriz do objeto = transformacoes do usuario * normalizacao.
    glMultMatrixd(transformacao.m);
    glMultMatrixd(normalizacao.m);
    desenharObjeto(config, texturaObjeto, mapaAmbiente, transformacao.determinante3x3() < 0.0);

    glPopMatrix();
}
