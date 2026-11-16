# Imagem de desenvolvimento do Projeto_CG_3D
# C++ + OpenGL + FreeGLUT + GLUI, com compilador e bibliotecas de
# desenvolvimento incluídos para permitir editar e recompilar o projeto.

FROM debian:bookworm-slim

ENV DEBIAN_FRONTEND=noninteractive

# Dependências de compilação e execução.
# - build-essential/cmake/git: compilar o projeto e compilar a GLUI do fonte
# - libgl1-mesa-dev/libglu1-mesa-dev/freeglut3-dev: headers e libs de OpenGL/GLUT/GLU
# - libgl1-mesa-dri/libglx-mesa0: renderização OpenGL (llvmpipe por padrão)
# - libX*: bibliotecas usadas pela GLUT/GLUI em tempo de execução
# - xauth/x11-utils/mesa-utils: diagnóstico e conexão com o servidor X11
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        ca-certificates \
        cmake \
        git \
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
        x11-utils \
        mesa-utils \
    && rm -rf /var/lib/apt/lists/*

# A GLUI não existe nos repositórios do Debian/Ubuntu, então ela é
# compilada do código-fonte. Versão fixada para builds reproduzíveis.
ARG GLUI_COMMIT=093edc777c02118282910bdee59f8db1bd46a84d
RUN git clone https://github.com/libglui/glui.git /tmp/glui \
    && cd /tmp/glui \
    && git checkout "${GLUI_COMMIT}" \
    && cmake -S . -B build \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr/local \
    && cmake --build build --target glui glui_static -j"$(nproc)" \
    && cmake --install build \
    && ldconfig \
    && rm -rf /tmp/glui

# Usuário sem privilégios com o mesmo UID/GID do usuário do host, para que
# os arquivos criados dentro do container (ex.: build/, curvas) pertençam
# ao usuário do host. Pode ser sobrescrito no build ou no compose.
ARG HOST_UID=1000
ARG HOST_GID=1000
RUN if ! getent group "${HOST_GID}" >/dev/null; then \
        groupadd -g "${HOST_GID}" dev; \
    fi \
    && useradd -m -u "${HOST_UID}" -g "${HOST_GID}" -s /bin/bash dev \
    && mkdir -p /app \
    && chown -R "${HOST_UID}:${HOST_GID}" /app

WORKDIR /app

# Copia o projeto e compila. No docker-compose o diretório do host é
# montado sobre /app, então essa cópia serve para rodar a imagem isolada.
COPY --chown=${HOST_UID}:${HOST_GID} . .
RUN make -j"$(nproc)"

USER dev

# Permite abrir a janela no X11/XWayland do host via socket montado.
ENV LIBGL_ALWAYS_SOFTWARE=1

CMD ["tail", "-f", "/dev/null"]
