# Visualizador de Objetos 3D — Documentação

Trabalho 2 de Computação Gráfica (UNICAP, 2026.2): sistema visualizador de
objetos 3D com interface gráfica interativa em OpenGL, GLUT e GLUI.

---

## 1. Visão geral

### 1.1 Objetivo

Abrir modelos 3D no formato Wavefront `.obj` e explorá-los com recursos
clássicos de computação gráfica: projeção perspectiva, iluminação de Phong,
sombreamento flat e Gouraud, transparência com mistura alfa, brilho especular,
mapeamento de reflexão do ambiente e mapeamento de textura. Todo o uso é
feito pela interface gráfica (painel GLUI, menu de contexto e diálogos de
arquivo), sem passar parâmetros pela linha de comando.

### 1.2 De 2D para 3D

O projeto original era um **visualizador 2D de curvas de Bézier cúbicas**:
lia pontos de controle (`v x y`) de um `.obj`, desenhava a curva com
`gluOrtho2D` e aplicava transformações 2D acumuladas (rotação, translação,
escala, reflexão e cisalhamento) por um painel GLUI.

A versão 3D mantém a proposta (visualizador com painel GLUI de controles) e
reaproveita cada parte do original:

| Projeto 2D | Versão 3D |
|---|---|
| `gluOrtho2D` e curvas em linhas | `gluPerspective`, câmera orbital, malhas triangulares iluminadas |
| `.obj` só com pontos de controle | `.obj` completo (`v`, `vt`, `f`, `mtllib`); os desenhos 2D antigos (`desenhos/*.obj`) continuam abrindo e viram **tubos 3D ao longo das curvas de Bézier** |
| Transformações 2D (`glRotated` em Z etc.) | Transformações 3D com matrizes 4×4 próprias (rotação em X/Y/Z, translação e escala XYZ, reflexão por plano, 6 tipos de cisalhamento) |
| "Desenho original + transformado" | Opção "Mostrar original (aramado)" |
| Campo de texto com o caminho do arquivo | Diálogo "Abrir arquivo" com navegação por pastas (GLUI) + diálogo nativo no Windows |
| Botão Salvar | Salvar OBJ com as transformações aplicadas |
| Menu do botão direito (GLUT) | Mantido, com opções 3D (sombreamento, superfície, luzes, câmera) |

### 1.3 Tecnologias

| Item | Uso |
|---|---|
| C++17 | Linguagem do projeto |
| OpenGL 1.x (pipeline fixo) + GLU | Renderização, iluminação, texturas, `gluPerspective`, `gluBuild2DMipmaps` |
| FreeGLUT 3.6.0 | Janela, eventos, menu de contexto, roda do mouse |
| GLUI 2.37 (commit `093edc7`) | Painel de controles e diálogo de arquivos |
| stb_image / stb_image_write | Leitura de PNG/BMP/JPG e gravação das capturas PNG |
| Visual Studio 2026 (toolset v145) | Solução `Projeto_CG_3D.sln` (Windows) |
| GNU Make + Docker (Debian 12) | Build e testes no Linux |
| Xvfb | X virtual para o autoteste sem monitor |

FreeGLUT, GLUI e stb ficam com o **código-fonte em `third_party/`** e são
compilados junto com o projeto (ligação estática). Nada precisa ser instalado
além do Visual Studio (Windows) ou do compilador e do FreeGLUT do sistema
(Linux).

---

## 2. Alterações realizadas

### 2.1 Principais modificações

- **Renderização 3D real**: malhas triangulares com buffer de profundidade,
  projeção perspectiva, câmera orbital e iluminação por vértice do OpenGL. Os
  dados vão para a GPU por *vertex arrays* (`glDrawArrays`/`glDrawElements`).
- **Leitor de OBJ completo** (`ArquivoOBJ.cpp`): `v`, `vt`, `f` com polígonos
  de qualquer número de lados (triangulação em leque), formatos
  `v`, `v/vt`, `v//vn`, `v/vt/vn`, índices negativos, `mtllib`/`map_Kd`,
  mensagens de erro com o número da linha.
- **Normais calculadas pelo programa** (`Malha.cpp`): normal por face (flat) e
  normal média por vértice ponderada pelo ângulo (smooth), com solda por
  posição para não "rachar" nas costuras de UV.
- **Painel GLUI reorganizado** em três colunas (Arquivo, Iluminação,
  Sombreamento, Material, Superfície/Textura, Transformações 3D, Visualização)
  com status da última operação.
- **Diálogo de abertura de arquivos** (`DialogoArquivo.cpp`).
- **Testes**: testes unitários do núcleo (`tests/testes.cpp`) e autoteste
  visual de integração (`src/AutoTeste.cpp`) que roda o programa de verdade e
  verifica cada requisito.
- **Build**: solução do Visual Studio 2026 com as bibliotecas compiladas do
  fonte; Makefile e Docker atualizados (a GLUI deixou de ser baixada no
  `docker build`); scripts de terminal para Windows.

### 2.2 Funcionalidades 3D implementadas

| Funcionalidade | Como funciona |
|---|---|
| Projeção perspectiva | `gluPerspective(45°, largura/altura, 0.05, 200)` refeita a cada quadro com o tamanho atual da janela |
| Câmera orbital | Arrastar com o botão esquerdo gira; roda do mouse, botão do meio ou `+`/`-` aproximam; setas giram; `R` reseta |
| Normalização do modelo | Qualquer `.obj` é centralizado e escalado para caber numa esfera de raio 1 (sem alterar o arquivo) |
| Iluminação | 2 luzes pontuais (`GL_LIGHT0`/`GL_LIGHT1`), cada uma com liga/desliga e posição X/Y/Z; marcadores das luzes na cena |
| Sombreamento flat | `glShadeModel(GL_FLAT)` + normal da face em todos os cantos do triângulo |
| Sombreamento smooth (Gouraud) | `glShadeModel(GL_SMOOTH)` + normais médias por vértice |
| Especular de Phong | `GL_SPECULAR` + `GL_SHININESS` (1–128), observador local e cor especular separada (`GL_SEPARATE_SPECULAR_COLOR`) |
| Transparência | Alfa do material pelo slider, `GL_BLEND`, `glDepthMask(GL_FALSE)` e triângulos ordenados de trás para frente a cada quadro |
| Reflexão do ambiente | *Sphere map* (`GL_SPHERE_MAP`) com mapa de estúdio gerado por código ou imagem escolhida pelo usuário |
| Textura | PNG/BMP/JPG com mipmaps; coordenadas `vt` do arquivo ou geradas (esférica, cilíndrica, planar) |
| Cena | Piso de madeira texturizado e iluminado (como a mesa da figura do PDF), eixos opcionais |
| Transformações 3D | Rotação, translação, escala, reflexão e cisalhamento acumulados |
| Curvas de Bézier → 3D | Desenhos 2D do projeto original viram tubos com tampas e `vt` |

### 2.3 Novos componentes

| Arquivo | Responsabilidade |
|---|---|
| `Vetor.h` | `Vetor2`/`Vetor3`, produto escalar e vetorial |
| `Matriz4.h/.cpp` | Matriz 4×4 (column-major): translação, escala, rotação, reflexão, cisalhamento, determinante, inversa-transposta |
| `Malha.h/.cpp` | Malha triangular, caixa envolvente e normais (face e vértice) |
| `CoordenadasTextura.h/.cpp` | Geração de UV esférica, cilíndrica e planar com correção da costura |
| `OrdenacaoProfundidade.h/.cpp` | Ordenação dos triângulos por profundidade (algoritmo do pintor) |
| `Imagem.h/.cpp` | Leitura e gravação de imagens; mapa de ambiente e textura do piso gerados por código |
| `Textura.h/.cpp` | Textura OpenGL (mipmaps, repetição) |
| `Camera.h/.cpp` | Câmera orbital e projeção perspectiva |
| `Configuracao.h` | Estado da cena ligado aos controles GLUI (*live variables*) |
| `Visualizacao.h/.cpp` | Renderizador: luzes, piso, objeto, modos de superfície, transparência |
| `Aplicacao.h/.cpp` | Estado central (modelo, texturas, câmera, transformações) e operações da interface |
| `DialogoArquivo.h/.cpp` | Navegador de arquivos GLUI e diálogo nativo do Windows |
| `Caminhos.h/.cpp` | Caminhos de arquivo, filtros de extensão e localização da pasta do projeto |
| `OpenGL.h` | Inclusão única de OpenGL/GLU/FreeGLUT (Windows e Linux) |
| `AutoTeste.h/.cpp` | Teste visual automatizado (`--autoteste`) |
| `tests/testes.cpp` | Testes unitários do núcleo |

Arquivos do projeto original que continuam com o mesmo papel: `main.cpp`
(inicialização), `Menu.cpp` (interface GLUI + menu GLUT), `Visualizacao.cpp`
(desenho), `ArquivoOBJ.cpp` (arquivos), `Bezier.cpp` (curvas),
`Transformacoes.cpp` (transformações) e `Ponto.h`.

### 2.4 Melhorias de arquitetura

- **Núcleo sem OpenGL**: leitura de OBJ, malha, normais, UV, matrizes,
  transformações, ordenação e imagens não dependem de janela; por isso têm
  testes unitários que rodam em qualquer máquina.
- **Estado centralizado** em `Aplicacao` (antes eram variáveis globais
  espalhadas com `extern`).
- **Um único callback** de controles GLUI com identificadores (`IdControle`),
  em vez de uma função por botão.
- **Matrizes calculadas pelo programa** (`Matriz4`) em vez de chamadas
  soltas a `glRotated`/`glScaled`. Assim é possível salvar o OBJ transformado,
  detectar reflexões (determinante negativo → `glFrontFace(GL_CW)`) e testar.
- **Caminhos absolutos**: o programa acha `modelos/` e `texturas/` a partir
  da pasta do executável, então funciona pelo Visual Studio, pelo terminal ou
  pelo Docker.
- **Build reprodutível**: bibliotecas em `third_party/`; o projeto do VS
  ignora a integração global do vcpkg (`VcpkgEnabled=false`) para não ligar
  outras cópias de GLUT/GLUI instaladas na máquina.
- Zero avisos com `/W4` (MSVC) e `-Wall -Wextra` (GCC) no código do projeto.

---

## 3. Checklist de requisitos do PDF

Legenda: **[x]** implementado e validado (por execução); **[ ]** não
implementado ou não validado. "Autoteste" = verificação de `src/AutoTeste.cpp`;
"Unitário" = `tests/testes.cpp`; "Manual" = teste feito na interface.

### 1. Descrição geral

- [x] **Visualizador de objetos 3D com interface gráfica usando OpenGL, GLUT e GLUI.**
  Janela 3D em FreeGLUT e painel "Controles" em GLUI. (`src/main.cpp`, `src/Menu.cpp`, `src/Visualizacao.cpp`)
- [x] **Controles visuais para carregar arquivos e acionar as funcionalidades, sem
  depender de linha de comando.** Tudo é acessível pelo painel GLUI, pelo menu
  do botão direito e pelos diálogos de arquivo. O programa abre sem argumentos e
  já carrega o modelo principal. A única opção de linha de comando
  (`--autoteste`) é para testes automatizados. (`src/Menu.cpp`, `src/DialogoArquivo.cpp`)
  — Manual: abrir modelo, trocar textura, sombreamento, luzes e transformações
  só com o mouse, no Windows e no Linux.

### 2. Visualização e interface gráfica

- [x] **2a. Carregamento via interface: navegar pelo sistema de arquivos e
  escolher o `.obj`.** Botão **"Abrir OBJ..."** abre a janela GLUI "Abrir
  arquivo" (`GLUI_FileBrowser`): duplo clique entra em pastas, `..` sobe,
  botões "Pasta acima" e "Pasta do projeto"; só aceita `.obj`. No Windows
  também há **"Abrir OBJ (Windows)..."** com o diálogo nativo. O navegador da
  GLUI tem um defeito no Linux (pastas terminam em `/` e são entregues como
  arquivo), que foi tratado em `aoEscolherItem`. (`src/DialogoArquivo.cpp`)
  — Manual: Windows (abrir `spot.obj`) e Linux/Xvfb (navegar
  `modelos → .. → desenhos` e abrir `rosa.obj`); diálogo nativo testado no Windows.
- [x] **2b. Projeção perspectiva.** `gluPerspective` em `Camera::aplicarProjecao`.
  (`src/Camera.cpp`) — Autoteste: matriz de projeção com `P[11] = -1` e `P[15] = 0`.
- [x] **2b. Janela inicial com no mínimo 600×600 pixels.** Área de desenho de
  800×700 (em telas baixas a altura diminui, nunca abaixo de 600). No Windows o
  processo é *DPI-aware* para o sistema não reduzir a janela com escala de
  125%/150%. (`src/main.cpp`) — Autoteste: 800×700 medido no Windows e no Linux.
- [x] **2b. Redimensionamento livre mantendo a proporção, sem distorção.**
  Callback de *reshape* atualiza o viewport e a perspectiva usa
  `aspecto = largura / altura` da janela atual. (`src/Visualizacao.cpp`,
  `src/Camera.cpp`) — Autoteste: 900×600 → aspecto 1,5; 600×900 → aspecto 0,667.
- [x] **2c. Ligar e desligar a(s) fonte(s) de luz pela interface.** Checkbox
  "Ligada" para cada uma das 2 luzes e opções no menu do botão direito.
  (`src/Menu.cpp`, `Visualizacao::configurarLuzes`) — Autoteste: luzes
  desligadas reduzem a luminância de 61 para 3,6; a segunda luz aumenta para 79,5.
- [x] **2c. Modificar a posição da fonte de luz.** Spinners X/Y/Z por luz;
  marcador amarelo (ligada) ou cinza (desligada) e linha até o piso mostram a
  posição. (`src/Menu.cpp`, `Visualizacao::desenharMarcadoresLuz`) —
  Autoteste: mover a luz muda a imagem (diferença média 18).

### 3. Funcionalidades

#### a) Sombreamento

- [x] **Flat shading (iluminação por face).** Normal da face (`normaisFace`)
  nos três cantos + `GL_FLAT`. (`src/Malha.cpp`, `Visualizacao::desenharObjeto`)
  — Autoteste: `GL_FLAT` ativo e imagem diferente do smooth. Unitário: normais
  de face unitárias e para fora.
- [x] **Smooth shading (Gouraud) com normais médias nos vértices.** Média das
  normais das faces vizinhas, ponderada pelo ângulo, + `GL_SMOOTH`.
  (`Malha::calcularNormais`) — Autoteste: `GL_SMOOTH` ativo. Unitário: no canto
  do cubo a normal é (1,1,1)/√3; vértices duplicados na costura compartilham a normal.
- [x] **Alternar entre os modelos por seletores na interface.** Radio "Flat /
  Smooth (Gouraud)" no painel e submenu "Sombreamento" no botão direito.
  (`src/Menu.cpp`) — Manual.

#### b) Transparência

- [x] **Canal alfa e mistura de cores (`glEnable(GL_BLEND)`).** Alfa no
  material, `glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)`.
  (`Visualizacao::desenharObjeto`) — Autoteste: `GL_BLEND` ativo com opacidade < 1;
  objeto translúcido fica entre a imagem opaca e o fundo (2,4 contra 4,2 de distância).
- [x] **Slider de opacidade na interface.** `GLUI_Scrollbar` de 0 a 1, com o
  valor exibido no rótulo. (`src/Menu.cpp`) — Manual.
- [x] **Ordenação correta / depth buffer no desenho translúcido.** Objetos
  opacos (piso, marcadores) são desenhados antes; o objeto translúcido é
  desenhado com `glDepthMask(GL_FALSE)` e com os triângulos ordenados de trás
  para frente pela profundidade no espaço da câmera, recalculada a cada quadro.
  (`src/OrdenacaoProfundidade.cpp`) — Autoteste: escrita no depth buffer
  desligada e 6320 triângulos ordenados. Unitário: ordem correta e dependente da câmera.

#### c) Reflexão

- [x] **Reflexão especular de Phong com controle do expoente (shininess).**
  `GL_SPECULAR`, `GL_SHININESS` e `GL_LIGHT_MODEL_LOCAL_VIEWER`; slider
  "Brilho especular (shininess)" de 1 a 128 e slider "Intensidade especular".
  (`src/Visualizacao.cpp`, `src/Menu.cpp`) — Autoteste: shininess 4 e 128
  geram imagens diferentes.
- [x] **Reflexão do ambiente (sphere map) simulando superfície metálica, com
  ativação/desativação na interface.** Radio "Reflexão do ambiente" (e
  submenu do botão direito) liga `GL_TEXTURE_GEN_S/T` com `GL_SPHERE_MAP`;
  escolher outra opção desliga. O mapa padrão é um estúdio gerado por código;
  "Escolher mapa de ambiente..." carrega outra imagem (ex.:
  `texturas/ambiente_por_do_sol.png`). (`Visualizacao::desenharObjeto`,
  `gerarMapaAmbienteEstudio`) — Autoteste: `GL_SPHERE_MAP` ativo e aparência
  alterada. Manual: ligar e desligar pelo menu e pelo painel.

#### d) Mapeamento de textura

- [x] **Carregar imagens (.png, .bmp, .jpg) e mapear sobre a malha.**
  stb_image + `gluBuild2DMipmaps` (aceita tamanhos que não são potência de 2).
  (`src/Imagem.cpp`, `src/Textura.cpp`) — Autoteste: `madeira.jpg`,
  `xadrez.bmp` e `marmore.png` aplicadas. Unitário: leitura dos três formatos e
  orientação vertical correta.
- [x] **Selecionar a imagem de textura pela interface.** "Escolher textura..."
  (navegador GLUI) e "Escolher textura (Windows)..."; o `.mtl` (`map_Kd`) ou uma
  imagem com o mesmo nome do `.obj` é carregada automaticamente.
  (`src/Menu.cpp`, `src/ArquivoOBJ.cpp`) — Manual + Autoteste (`spot.png` carregada com `spot.obj`).
- [x] **Alternar entre texturizado, sólido monocromático e reflexão.** Radio
  "Sólido (monocromático) / Texturizado / Reflexão do ambiente" e submenu
  "Superfície". A cor do sólido é ajustável (R, G, B). (`src/Menu.cpp`) —
  Manual + Autoteste (os três modos renderizados).
- [x] **Usar `vt` do `.obj` quando existir; senão gerar coordenadas
  automaticamente (esférica, cilíndrica ou planar).** Ao carregar, o seletor
  vai para "Do arquivo OBJ (vt)" se a malha tiver `vt`; caso contrário usa a
  projeção esférica. O usuário pode trocar para cilíndrica ou planar.
  (`src/CoordenadasTextura.cpp`, `Aplicacao::carregarModelo`) — Autoteste:
  `bule.obj` (sem `vt`) usa esférica; `spot.obj` (com `vt`) usa as do arquivo;
  as três projeções diferem. Unitário: costura corrigida e intervalos válidos.

### 4. Exemplo de cena

- [x] **Cena semelhante à figura 1 (ilustrativa).** O objeto fica apoiado
  sobre uma mesa de madeira iluminada, sobre fundo escuro, e pode ficar
  translúcido ou espelhado como os objetos da figura. Ver captura
  `build/autoteste/22_demo_reflexo_translucido.png` gerada pelo autoteste.

### 5. Critérios de entrega

- [x] **Modelo principal com no mínimo 1600 triângulos.** `modelos/bule.obj`
  (Utah Teapot) com **6320 triângulos**, aberto na inicialização. Também
  acompanham `modelos/spot.obj` (5856 triângulos, com `vt` e textura).
  — Unitário + Autoteste.
- [x] **Projeto configurado no Visual Studio C++ 2026 com as bibliotecas
  gráficas vinculadas.** `Projeto_CG_3D.sln` com os projetos `Visualizador3D`,
  `freeglut`, `glui` (bibliotecas estáticas compiladas do fonte) e `Testes`,
  toolset **v145** (VS 2026), x64 Debug/Release, ligado com `opengl32.lib`,
  `glu32.lib`, `winmm.lib`, `gdi32.lib`, `user32.lib` e `comdlg32.lib`. O
  diretório de trabalho da depuração é a raiz do projeto. (`vs/`) — Compilado
  com MSBuild 18 / MSVC 14.51 em Release x64 sem erros nem avisos.
- [ ] **Entregar a pasta completa compactada em um único .ZIP no Teams.**
  O pacote é gerado por `scripts/empacotar.ps1` (testado: cria
  `build/entrega/Projeto_CG_3D.zip`, ~1,6 MB, sem `build/` e `.git`). **O envio
  no Teams é feito pela equipe** e por isso não está marcado.

---

## 4. Estrutura do projeto

```
Projeto_CG_3D/
├── Projeto_CG_3D.sln          Solução do Visual Studio 2026
├── Makefile                   Build/testes no Linux (make, make testes, make autoteste)
├── Dockerfile                 Imagem Debian com compilador, OpenGL, FreeGLUT e Xvfb
├── docker-compose.yml         Monta o projeto no container e abre a janela no X do host
├── README.md                  Início rápido
├── docs/DOCUMENTACAO.md       Este documento
├── pdf_da_atividade/          Enunciado do trabalho
├── include/                   Cabeçalhos (.h) — descrição na seção 2.3
├── src/                       Código-fonte (.cpp)
│   ├── main.cpp               Cria a janela, o menu, o painel e inicia o loop
│   ├── Aplicacao.cpp          Estado central e operações (abrir, salvar, texturas)
│   ├── Menu.cpp               Painel GLUI, menu GLUT, mouse e teclado
│   ├── DialogoArquivo.cpp     Navegador de arquivos GLUI e diálogo nativo do Windows
│   ├── Visualizacao.cpp       Renderização da cena
│   ├── Camera.cpp             Câmera orbital + perspectiva
│   ├── Textura.cpp, Imagem.cpp
│   ├── ArquivoOBJ.cpp, Malha.cpp, CoordenadasTextura.cpp
│   ├── Bezier.cpp, Transformacoes.cpp, Matriz4.cpp
│   ├── OrdenacaoProfundidade.cpp, Caminhos.cpp
│   └── AutoTeste.cpp          Teste visual automatizado
├── tests/testes.cpp           Testes unitários (sem janela)
├── modelos/                   bule.obj (principal), spot.obj + spot.png
├── texturas/                  madeira.jpg, xadrez.bmp, marmore.png, ambiente_por_do_sol.png
├── desenhos/                  Desenhos 2D do projeto original (abrem como tubos 3D)
├── scripts/
│   ├── windows.ps1            Compilar/executar/testar no Windows pelo terminal
│   ├── empacotar.ps1          Gera o .zip de entrega
│   └── gerar_texturas.py      Recria as texturas de exemplo (opcional, Python + Pillow)
├── vs/                        Projetos .vcxproj e Comum.props
├── third_party/
│   ├── freeglut/              FreeGLUT 3.6.0 (fonte) + msvc/config.h
│   ├── glui/                  GLUI (fonte, commit 093edc7)
│   └── stb/                   stb_image.h, stb_image_write.h
├── freeglut.dll, glui.dll     DLLs que já estavam no repositório (não são mais necessárias)
└── build/                     Saída da compilação, capturas e relatórios (ignorado pelo git)
```

Origem dos recursos: Utah Teapot (Martin Newell, 1975) e Spot (Keenan Crane,
CC0), obtidos do repositório *common-3d-test-models* (alecjacobson); as
texturas foram geradas por `scripts/gerar_texturas.py`. Licenças das
bibliotecas: FreeGLUT (X-Consortium/MIT, `third_party/freeglut/COPYING`), GLUI
(zlib, `third_party/glui/LICENSE.md`), stb (domínio público/MIT, no cabeçalho).

---

## 5. Como executar o projeto

### 5.1 Windows — Visual Studio 2026 (IDE)

Requisito: Visual Studio 2026 com a carga de trabalho **"Desenvolvimento para
desktop com C++"** (MSVC v145 + Windows SDK 10).

1. Abrir `Projeto_CG_3D.sln`.
2. Escolher a configuração **Release | x64** (ou Debug | x64).
3. `Visualizador3D` já é o projeto de inicialização: pressionar **F5** (ou Ctrl+F5).

O executável fica em `build\vs\x64\Release\visualizador3d.exe`.

### 5.2 Windows — terminal (PowerShell)

Na raiz do projeto:

```powershell
# compilar e abrir o visualizador
powershell -ExecutionPolicy Bypass -File scripts\windows.ps1 -Acao executar

# apenas compilar (Release x64)
powershell -ExecutionPolicy Bypass -File scripts\windows.ps1 -Acao compilar

# compilar em Debug
powershell -ExecutionPolicy Bypass -File scripts\windows.ps1 -Acao compilar -Configuracao Debug

# abrir um executável já compilado
.\build\vs\x64\Release\visualizador3d.exe
```

O script localiza o MSBuild com o `vswhere` e compila a solução inteira.

### 5.3 Linux / Docker

```bash
docker compose build     # cria a imagem (instala dependências e compila)
docker compose up        # compila o que mudou e abre a janela
```

Para a janela aparecer, o host precisa de um servidor X11/XWayland (socket
`/tmp/.X11-unix`, já montado). Se não abrir: `xhost +local:`. A renderização
padrão é por software (`LIBGL_ALWAYS_SOFTWARE=1`), sem precisar de GPU.

### 5.4 Linux sem Docker (Debian/Ubuntu)

```bash
sudo apt install build-essential freeglut3-dev libglu1-mesa-dev xvfb x11-utils
make            # gera ./visualizador3d
make run        # compila e abre
```

### 5.5 Uso rápido

| Ação | Onde |
|---|---|
| Abrir modelo | "Abrir OBJ..." (ou "Abrir OBJ (Windows)...") |
| Girar / zoom | Arrastar com o botão esquerdo / roda ou botão do meio, `+`/`-`, setas |
| Luzes | Painel "Iluminação" (liga/desliga e posição X/Y/Z) ou menu do botão direito |
| Flat/Smooth | Painel "Sombreamento" ou menu do botão direito |
| Transparência e brilho | Sliders do painel "Material" |
| Sólido / Textura / Reflexão | Painel "Superfície / Textura" ou menu do botão direito |
| Coordenadas de textura | "Do arquivo OBJ (vt)", "Automática esférica/cilíndrica/planar" |
| Transformações | Rollout "Transformações 3D" |
| Sair | Botão "Sair", tecla `Q` ou `Esc` |

---

## 6. Pipeline de Testes

Há três níveis de teste:

| Nível | O que cobre | Como roda |
|---|---|---|
| **Testes unitários** (`tests/testes.cpp`) | Matrizes, transformações, leitor de OBJ, normais, UV, ordenação, imagens, Bézier, salvar OBJ (59 verificações) | Executável `testes`, sem janela |
| **Autoteste visual** (`src/AutoTeste.cpp`) | Roda o visualizador real, aplica cada requisito, salva capturas PNG e confere o estado do OpenGL e os pixels (28 verificações) | `visualizador3d --autoteste <pasta>`; no Linux sem monitor, via `xvfb-run` |
| **Testes manuais** | Interação com mouse, diálogos e painel | Roteiro do passo 5 |

### Passo 1 — Preparação do ambiente

- **Windows**: Visual Studio 2026 com "Desenvolvimento para desktop com C++".
  Conferir: `& "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property displayName`
- **Linux/Docker**: Docker com Compose v2 (`docker compose version`).
- Clonar ou extrair o projeto e abrir um terminal na pasta raiz.

### Passo 2 — Instalação das dependências

Não há bibliotecas externas para instalar no Windows (FreeGLUT, GLUI e stb são
compiladas de `third_party/`).

```bash
# Docker (instala compilador, OpenGL, FreeGLUT, Xvfb e já compila tudo)
docker compose build

# ou Linux nativo
sudo apt install build-essential freeglut3-dev libglu1-mesa-dev xvfb x11-utils
```

### Passo 3 — Compilação e inicialização

```powershell
# Windows: compila a solução (deve terminar com "Compilado em ...")
powershell -ExecutionPolicy Bypass -File scripts\windows.ps1 -Acao compilar
```

```bash
# Linux/Docker
docker compose run --rm app make
```

Critério: compilação sem erros. O projeto compila sem avisos com `/W4` (MSVC)
e `-Wall -Wextra` (GCC).

Inicialização: `scripts\windows.ps1 -Acao executar` (Windows) ou
`docker compose up` / `make run` (Linux). Devem abrir a janela 3D (800×700)
com o bule sobre a mesa e o painel "Controles".

### Passo 4 — Testes automatizados (elementos 3D e requisitos)

```powershell
# Windows: testes unitários + autoteste visual
powershell -ExecutionPolicy Bypass -File scripts\windows.ps1 -Acao tudo

# separadamente
powershell -ExecutionPolicy Bypass -File scripts\windows.ps1 -Acao testes
powershell -ExecutionPolicy Bypass -File scripts\windows.ps1 -Acao autoteste
```

```bash
# Linux/Docker (o autoteste usa X virtual: não precisa de monitor)
docker compose run --rm app make testes
docker compose run --rm app make autoteste
```

Saídas esperadas:

- Testes unitários: `59 verificacoes, 0 falha(s)` (código de saída 0).
- Autoteste: `Autoteste concluido: 28 verificacoes, 0 falha(s)` (código de saída 0).
  O autoteste abre a janela por cerca de 10 s; não mexa nela enquanto roda.
- Relatório em `build/autoteste/relatorio.txt` e capturas
  `build/autoteste/01_...png` a `22_...png`.

Resultado obtido nesta entrega (Windows 11 + VS 2026 e Docker Debian 12 com
llvmpipe): **59/59** e **28/28** nas duas plataformas.

### Passo 5 — Testes manuais das mecânicas e interações

Com o programa aberto (`-Acao executar`):

1. **Câmera**: arrastar com o botão esquerdo gira a cena; a roda do mouse
   aproxima e afasta; `R` volta à vista inicial.
2. **Abrir arquivo (GLUI)**: "Abrir OBJ..." → duplo clique em `spot.obj` → a vaca
   aparece e o painel mostra "5856 triângulos | vt: sim". Repetir navegando com
   `..` até `desenhos/` e abrir `mario.obj` (desenho 2D original em tubos 3D).
3. **Abrir arquivo (Windows)**: "Abrir OBJ (Windows)..." abre o diálogo nativo em `modelos/`.
4. **Luzes**: desmarcar "Ligada" da Luz 1 (cena escurece); ligar a Luz 2
   (marcador amarelo); alterar "Posição X/Y/Z" (o brilho especular e as
   regiões claras/escuras do objeto e do piso mudam).
5. **Sombreamento**: alternar Flat/Smooth (facetas visíveis no flat).
6. **Transparência**: arrastar o slider "Opacidade" até ~0,4 (o piso aparece
   através do objeto, sem buracos nem faces "piscando" ao girar a câmera).
7. **Especular**: variar "Brilho especular" de 1 a 128 (o reflexo fica menor e
   mais concentrado) e "Intensidade especular".
8. **Reflexão**: selecionar "Reflexão do ambiente" (aparência metálica); "Escolher
   mapa de ambiente..." → `texturas/ambiente_por_do_sol.png`; "Ambiente padrão" volta.
9. **Textura**: "Texturizado"; "Escolher textura..." → `xadrez.bmp`; alternar
   "Automática esférica / cilíndrica / planar" no bule. No `spot.obj`, "Do arquivo OBJ (vt)".
10. **Menu do botão direito**: Sombreamento, Superfície, ligar/desligar luzes,
    resetar câmera; o painel acompanha as mudanças.
11. **Transformações 3D**: abrir o rollout, "Rotacionar", "Escalar", "Refletir",
    "Cisalhar", marcar "Mostrar original (aramado)"; "Salvar OBJ" grava
    `modelos/salvo.obj`; "Resetar transformações".
12. **Janela**: redimensionar e maximizar: o objeto não deve achatar nem esticar.

### Passo 6 — Validação de cada requisito do PDF

| Requisito | Verificação automática | Verificação manual (passo 5) |
|---|---|---|
| 1 Interface sem linha de comando | — | 2, 3, 4–11 |
| 2a Abrir `.obj` navegando por pastas | Unitário: filtros de extensão | 2, 3 |
| 2b Perspectiva | Autoteste: matriz de projeção | 1 |
| 2b Janela ≥ 600×600 | Autoteste: 800×700 | — |
| 2b Redimensionar sem distorção | Autoteste: aspecto 1,5 e 0,667 | 12 |
| 2c Ligar/desligar luz | Autoteste: luminância 61 → 3,6 | 4, 10 |
| 2c Posição da luz | Autoteste: diferença 18 | 4 |
| 3a Flat / Smooth / seletor | Autoteste + Unitário (normais) | 5, 10 |
| 3b Alfa, blend, slider, ordenação | Autoteste (blend, depth mask, 6320 ordenados, mistura) + Unitário (ordenação) | 6 |
| 3c Especular + shininess | Autoteste | 7 |
| 3c Reflexão do ambiente on/off | Autoteste (`GL_SPHERE_MAP`) | 8, 10 |
| 3d Texturas png/bmp/jpg | Autoteste + Unitário | 9 |
| 3d Sólido / textura / reflexão | Autoteste | 8, 9, 10 |
| 3d `vt` ou geração automática | Autoteste + Unitário | 9 |
| 5 Malha ≥ 1600 triângulos | Autoteste + Unitário (6320) | Painel mostra "6320 triângulos" |
| 5 Visual Studio 2026 | Compilação pelo MSBuild (`-Acao compilar`) | Abrir a `.sln` e F5 |
| 5 ZIP de entrega | — | `scripts\empacotar.ps1` e conferir o conteúdo do zip |

As capturas do autoteste servem como evidência visual: `01` (smooth), `02`
(flat), `03` (luzes desligadas), `04` (luz movida), `05` (duas luzes),
`06`/`07` (shininess), `08`–`10` (transparência), `11` (reflexão), `12`–`16`
(texturas e projeções), `17` (spot com `vt`), `18`/`19` (redimensionamento),
`20` (transformações), `21` (Mario em 3D), `22` (cena de demonstração).

### Passo 7 — Verificação de erros e possíveis problemas

| Sintoma | Causa provável / solução |
|---|---|
| `MSBuild nao encontrado` | Instalar a carga "Desenvolvimento para desktop com C++" do VS 2026 |
| `LNK2005 ... glui.lib(glui.dll)` | Outra GLUI ligada pela integração global do vcpkg. Os projetos já desligam isso (`VcpkgEnabled=false` em `vs/Comum.props`); não remova essa linha |
| Toolset v145 não encontrado | Projeto aberto em versão anterior do VS: use o VS 2026 ou "Redirecionar projetos" |
| "Modelo padrão não encontrado" | Executável rodando fora do projeto: execute pela raiz ou mantenha `modelos/` e `texturas/` junto da pasta do projeto |
| Docker: janela não abre | Servidor X ausente ou sem permissão: `echo $DISPLAY`, `xhost +local:`. Os testes (`make testes`, `make autoteste`) não precisam de X |
| Autoteste falha | Abrir `build/autoteste/relatorio.txt` (mostra o valor medido em cada verificação) e a captura PNG da etapa |
| Textura não aparece | Modo "Texturizado" não selecionado ou imagem não suportada (use png/bmp/jpg) |
| Mensagens de erro | A última mensagem aparece no rodapé do painel e todas vão para o console, junto com os avisos do FreeGLUT |

### Passo 8 — Critérios de aprovação

O projeto é considerado aprovado quando:

1. A solução compila em Release x64 no Visual Studio 2026 (IDE ou
   `scripts\windows.ps1 -Acao compilar`) sem erros.
2. Os testes unitários terminam com `0 falha(s)` (59 verificações).
3. O autoteste termina com `0 falha(s)` (28 verificações) e gera as capturas `01` a `22`.
4. O roteiro manual do passo 5 é concluído sem travamentos, sem uso de linha de
   comando e com o comportamento descrito em cada item.
5. Todos os itens da seção 3 estão marcados, exceto o envio no Teams, que é
   feito pela equipe com o zip de `scripts\empacotar.ps1`.

---

## 7. Limitações conhecidas

- O smooth shading é **Gouraud** (iluminação por vértice do pipeline fixo),
  opção permitida pelo PDF ("Gouraud ou Phong"). Não há Phong por pixel (exigiria shaders).
- A reflexão usa **sphere map**: depende da direção de visão e reflete um
  ambiente fixo (estúdio ou imagem escolhida), não a própria cena.
- A transparência ordena por triângulo (algoritmo do pintor): triângulos que se
  cruzam podem gerar pequenos artefatos; malhas comuns não têm esse caso.
- O alfa das imagens de textura é ignorado: a opacidade vem só do slider (o
  `spot.png`, por exemplo, tem alfa sem significado de transparência).
- Não há sombras projetadas (não fazem parte dos requisitos).
- O navegador de arquivos da GLUI é simples (lista da pasta, duplo clique); no
  Windows há também o diálogo nativo.
- Em telas com escala de 150%, o painel GLUI fica com fonte pequena, pois a
  GLUI usa fontes bitmap fixas e o programa desenha em pixels reais.
