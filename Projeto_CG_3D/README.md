# Projeto_CG_3D
Projeto de CG em 3D

Opa guys, esse é o repo novo do novo projeto que legal né, sejam bem vindos
obrigada arthur

dale

**Visualizador de Objetos 3D com interface gráfica** (OpenGL + GLUT + GLUI), feito
a partir do visualizador 2D de curvas de Bézier. Carrega arquivos `.obj` pela
interface e tem perspectiva, iluminação, sombreamento flat/smooth, transparência,
reflexo especular, reflexão do ambiente e texturas.

A documentação completa (requisitos do PDF, arquitetura e pipeline de testes)
está em [`docs/DOCUMENTACAO.md`](docs/DOCUMENTACAO.md).

## Windows (Visual Studio 2026)

Abra `Projeto_CG_3D.sln`, escolha `Release | x64` e pressione F5. FreeGLUT e
GLUI vêm com o código-fonte em `third_party/` e são compiladas junto: não é
preciso instalar nenhuma biblioteca.

Pelo terminal:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\windows.ps1 -Acao executar   # compila e abre
powershell -ExecutionPolicy Bypass -File scripts\windows.ps1 -Acao tudo       # compila + todos os testes
```

## Linux / Docker

O `Dockerfile` instala compilador, OpenGL, GLU e FreeGLUT. A GLUI é compilada
pelo `Makefile` a partir de `third_party/glui`.

```bash
docker compose build                        # cria a imagem (só repetir se mudar o Dockerfile)
docker compose up                           # compila e abre a janela (precisa de servidor X no host)
docker compose run --rm app make testes     # testes unitários
docker compose run --rm app make autoteste  # teste visual automático (usa X virtual, não precisa de X no host)
```

Para abrir a janela a partir do container, o host precisa de um servidor
X11/XWayland com o socket `/tmp/.X11-unix` (já montado pelo compose). Se a
janela não abrir, autorize o acesso local com `xhost +local:`.

Sem Docker, em Debian/Ubuntu:

```bash
sudo apt install build-essential freeglut3-dev libglu1-mesa-dev xvfb
make run
```
