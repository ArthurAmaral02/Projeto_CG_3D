#ifndef CONFIGURACAO_H
#define CONFIGURACAO_H

// Estado da cena controlado pela interface. Os campos sao int/float porque
// a GLUI os usa como "live variables" (checkbox, radio, spinner, scrollbar).

enum ModoSombreamento {
    SOMBREAMENTO_FLAT = 0,    // uma normal por face (GL_FLAT)
    SOMBREAMENTO_SMOOTH = 1   // normais medias por vertice + Gouraud (GL_SMOOTH)
};

enum ModoSuperficie {
    SUPERFICIE_SOLIDA = 0,    // cor unica (monocromatico)
    SUPERFICIE_TEXTURA = 1,   // imagem mapeada na malha
    SUPERFICIE_REFLEXAO = 2   // mapeamento de reflexao do ambiente (sphere map)
};

const int NUMERO_LUZES = 2;

struct FonteLuz {
    int ligada;
    float posicao[3];  // coordenadas do mundo (o objeto cabe numa esfera de raio 1)
    float cor[3];
};

struct Configuracao {
    FonteLuz luzes[NUMERO_LUZES] = {
        {1, {2.5f, 3.0f, 2.5f}, {1.0f, 0.97f, 0.92f}},    // luz principal
        {0, {-3.0f, 1.5f, -1.5f}, {0.45f, 0.50f, 0.65f}}  // luz de preenchimento
    };

    int sombreamento = SOMBREAMENTO_SMOOTH;
    int modoSuperficie = SUPERFICIE_SOLIDA;
    int geradorUV = 0;  // valor de GeradorUV

    float opacidade = 1.0f;              // 0 = invisivel, 1 = opaco
    float brilho = 48.0f;                // expoente especular (shininess) 1..128
    float intensidadeEspecular = 0.8f;   // 0..1
    float corObjeto[3] = {0.80f, 0.38f, 0.22f};

    int mostrarPiso = 1;
    int mostrarEixos = 0;
    int mostrarLuzes = 1;
    int mostrarOriginal = 0;  // contorno (aramado) do objeto sem transformacoes
};

#endif
