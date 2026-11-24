# Imagem de desenvolvimento do Projeto_CG_3D (Visualizador 3D)
# C++17 + OpenGL + FreeGLUT + GLUI, com compilador e bibliotecas de
# desenvolvimento incluidos para permitir editar e recompilar o projeto.

FROM debian:bookworm-slim

ENV DEBIAN_FRONTEND=noninteractive

# Dependencias de compilacao e execucao.
# - build-essential: compilador C++17 e make
# - libgl1-mesa-dev/libglu1-mesa-dev/freeglut3-dev: headers e libs de OpenGL/GLU/FreeGLUT
# - libgl1-mesa-dri/libglx-mesa0: renderizacao OpenGL (llvmpipe por padrao)
# - libX*: bibliotecas usadas pela GLUT/GLUI em tempo de execucao
# - xvfb/x11-utils/mesa-utils/xauth: X virtual para o autoteste e diagnostico
# A GLUI nao existe nos repositorios do Debian: ela e compilada pelo Makefile
# a partir do codigo-fonte em third_party/glui (sem download na hora do build).
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        ca-certificates \
        pkg-config \
        libgl1-mesa-dev \
        libglu1-mesa-dev \
        freeglut3-dev \
        libgl1 \
        libglu1-mesa \
        libglut3.12 \
        libgl1-mesa-dri \
        libglx-mesa0 \
        libx11-6 \
        libxext6 \
        libxi6 \
        libxmu6 \
        libxt6 \
        libxrandr2 \
        libxxf86vm1 \
        xauth \
        xvfb \
        x11-utils \
        mesa-utils \
    && rm -rf /var/lib/apt/lists/*

# Usuario sem privilegios com o mesmo UID/GID do usuario do host, para que
# os arquivos criados dentro do container (ex.: build/, visualizador3d)
# pertencam ao usuario do host. Pode ser sobrescrito no build ou no compose.
ARG HOST_UID=1000
ARG HOST_GID=1000
RUN if ! getent group "${HOST_GID}" >/dev/null; then \
        groupadd -g "${HOST_GID}" dev; \
    fi \
    && useradd -m -u "${HOST_UID}" -g "${HOST_GID}" -s /bin/bash dev \
    && mkdir -p /app \
    && chown -R "${HOST_UID}:${HOST_GID}" /app

WORKDIR /app

# Copia o projeto e compila o visualizador e os testes. No docker-compose o
# diretorio do host e montado sobre /app, entao essa copia serve para rodar
# a imagem isolada (ex.: CI).
COPY --chown=${HOST_UID}:${HOST_GID} . .
USER dev
RUN make -j"$(nproc)" all build/testes

# Renderizacao por software (funciona sem GPU).
ENV LIBGL_ALWAYS_SOFTWARE=1

CMD ["tail", "-f", "/dev/null"]
