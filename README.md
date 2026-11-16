# Projeto_CG_3D
Projeto de CG em 3D

Opa guys, esse é o repo novo do novo projeto que legal né, sejam bem vindos
obrigada arthur

dale

nos não vamos fazer ele pensando em usar o visual estudio community, então quem usar o windows vai ter que passar os arquivos para ele

## Rodando com Docker

O projeto tem `Dockerfile` e `docker-compose.yml` prontos. A imagem já instala
o compilador, OpenGL, GLU, FreeGLUT e compila a GLUI do fonte.

Pré-requisitos do host (não instalados pela imagem): servidor X11/XWayland
rodando. GPU é opcional — por padrão a renderização é feita por software.
Detalhes em [`docs/dependencias-3d.md`](docs/dependencias-3d.md).

```bash
# compilar a imagem (só precisa repetir quando mudar o Dockerfile)
docker compose build

# compilar o projeto e abrir a janela
docker compose up

# abrir outro desenho
docker compose run --rm app ./curvas desenhos/rosa.obj

# recompilar manualmente após editar o código
docker compose run --rm app make
```

O diretório do projeto é montado dentro do container, então as edições feitas
no host aparecem imediatamente. Os arquivos gerados (`build/` e `curvas`)
pertencem ao seu usuário do host.

