# Dependências 3D e de ambiente gráfico

Este arquivo documenta as dependências que **não** foram instaladas na imagem
Docker e que dependem da máquina onde o projeto vai rodar (host). Elas não são
necessárias para compilar o projeto, mas são necessárias para **exibir a janela
OpenGL** e, opcionalmente, usar aceleração por GPU.

A imagem Docker já inclui tudo o que o projeto precisa para compilar e para
renderizar por software (`llvmpipe`). O que está listado aqui é responsabilidade
do host.

---

## 1. O que já vem instalado na imagem

| Componente | Origem |
| --- | --- |
| `g++`, `make`, `cmake`, `pkg-config` | `build-essential` / Debian |
| OpenGL (`libGL`, `libGLX`) | `libgl1`, `libglx-mesa0` |
| GLU (`libGLU`) | `libglu1-mesa` |
| FreeGLUT (`libglut`) + headers | `freeglut3-dev` |
| Drivers Mesa para renderização por software | `libgl1-mesa-dri` |
| Bibliotecas X11 usadas pela GLUT/GLUI | `libx11-6`, `libxext6`, `libxi6`, `libxmu6`, `libxt6`, `libxrandr2`, `libxxf86vm1` |
| **GLUI** (`libglui.so` + `GL/glui.h`) | compilada do fonte no build (não existe pacote Debian/Ubuntu) |

> A GLUI é compilada do repositório <https://github.com/libglui/glui> com o
> commit fixado no `Dockerfile` (`GLUI_COMMIT`), pois não há pacote oficial
> dela nos repositórios do Debian/Ubuntu usados como base.

---

## 2. Dependências do host (NÃO instaladas)

### 2.1 Servidor gráfico (obrigatório para ver a janela)

O container não tem servidor gráfico próprio. Ele se conecta ao servidor X11 do
host pelo socket `/tmp/.X11-unix` (já montado no `docker-compose.yml`).

| Ambiente | O que precisa existir no host |
| --- | --- |
| Xorg (sessão X11) | Servidor X rodando, variável `DISPLAY` definida |
| Wayland (GNOME/KDE/Hyprland) | XWayland rodando, com `DISPLAY` apontando para ele |
| SSH / máquina remota | Encaminhamento X11 (`ssh -X`/`-Y`) **ou** VNC/Xvfb (ver seção 5) |

Verifique com:

```bash
echo "$DISPLAY"                       # ex.: :1
ls -l /tmp/.X11-unix/                 # deve mostrar o socket X0, X1, ...
xdpyinfo >/dev/null && echo "X11 OK"  # pacote x11-utils no host
```

Se o container não conseguir abrir a janela, autorize o acesso local uma vez:

```bash
# o container roda com o mesmo UID do seu usuário (UID 1000 por padrão)
xhost +local:
# se você rodar o container como root:
xhost +local:root
```

> Em sessões Wayland puras (sem XWayland) não é possível usar GLX/FreeGLUT.
> Ative o XWayland na sessão ou use Xvfb.

### 2.2 Drivers de GPU (opcional — só para aceleração)

Sem GPU o projeto funciona normalmente com renderização por software
(`LIBGL_ALWAYS_SOFTWARE=1`, padrão no compose). Para usar a GPU, o host precisa
ter o driver instalado e o dispositivo `/dev/dri` disponível:

| GPU | Driver no host | Pacotes típicos (Arch) | Pacotes típicos (Debian/Ubuntu) |
| --- | --- | --- | --- |
| Intel | Mesa (i915/xe) | `mesa` | `mesa-utils`, `libgl1-mesa-dri` |
| AMD | Mesa (amdgpu/radeonsi) | `mesa` | `mesa-utils`, `libgl1-mesa-dri` |
| NVIDIA | driver proprietário | `nvidia`, `nvidia-utils` | `nvidia-driver` |

Requisitos adicionais para GPU dentro do container:

1. Passar o dispositivo para o container (descomente no compose):
   ```yaml
   volumes:
     - /dev/dri:/dev/dri
   ```
2. Permitir acesso aos grupos do dispositivo (descubra os GIDs com
   `getent group video render`):
   ```yaml
   group_add:
     - "${VIDEO_GID:-985}"
     - "${RENDER_GID:-989}"
   ```
3. Desligar a renderização forçada por software:
   ```bash
   LIBGL_ALWAYS_SOFTWARE=0 docker compose up
   ```
4. **NVIDIA**: além do driver no host, é necessário o
   [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html)
   e reservar a GPU no compose:
   ```yaml
   deploy:
     resources:
       reservations:
         devices:
           - driver: nvidia
             count: all
             capabilities: [gpu]
   ```
   Também é preciso usar uma imagem base compatível com o driver (ou instalar
   as bibliotecas de runtime da NVIDIA via `nvidia-container-toolkit`).

Teste dentro do container:

```bash
docker compose run --rm app glxinfo -B
# "OpenGL renderer string" deve mostrar a GPU; se mostrar "llvmpipe",
# a aceleração não está ativa.
```

### 2.3 Dependências específicas de plataforma

| Plataforma | Observação |
| --- | --- |
| **Linux (recomendado)** | Funciona com X11/XWayland, conforme acima |
| **Windows** | Docker Desktop não expõe o socket X11. Use um servidor X no Windows (ex.: VcXsrv, X410, Xming), habilite TCP e use `DISPLAY=host.docker.internal:0`; remova o volume de `/tmp/.X11-unix` e ajuste o compose |
| **macOS** | Instale XQuartz, habilite "Allow connections from network clients" e use `DISPLAY=host.docker.internal:0`; também é preciso remover o volume de `/tmp/.X11-unix` |

---

## 3. Rodando sem Docker (referência)

Se um dia for rodar direto no host Linux, estas são as dependências (não
instaladas por este projeto):

- **Arch Linux**: `sudo pacman -S base-devel freeglut glu mesa` e a GLUI via
  AUR (`yay -S glui`) ou compilada do fonte como no `Dockerfile`.
- **Debian/Ubuntu**: `sudo apt install build-essential freeglut3-dev libglu1-mesa-dev libgl1-mesa-dev`,
  e a GLUI compilada do fonte.

O comando de compilação usado (equivalente ao `Makefile`):

```bash
g++ src/*.cpp -Iinclude -lglui -lglut -lGLU -lGL -o curvas
./curvas desenhos/mario.obj
```

---

## 4. Problemas comuns

| Mensagem / sintoma | Causa provável | Solução |
| --- | --- | --- |
| `freeglut (./curvas): failed to open display ':0'` | `DISPLAY` errado ou socket X não montado | Confira `echo $DISPLAY`, monte `/tmp/.X11-unix` e use `network_mode: host` |
| `Authorization required, but no authorization protocol specified` | Servidor X com controle de acesso | `xhost +local:root` no host, ou monte `~/.Xauthority` (linha comentada no compose) |
| `libGL error: MESA-LOADER: failed to open ...` | GPU sem `/dev/dri` ou sem permissão | Use `LIBGL_ALWAYS_SOFTWARE=1` ou configure `/dev/dri` + `group_add` |
| Janela abre mas renderização lenta | Renderização por software (llvmpipe) | Normal sem GPU; habilite o passthrough de `/dev/dri` |
| `GLX` não disponível | Wayland puro sem XWayland | Ative o XWayland ou use Xvfb |
| Arquivos criados como `root` | UID/GID do container diferente do host | Use `HOST_UID=$(id -u) HOST_GID=$(id -g) docker compose up` |

---

## 5. Execução sem monitor (headless / servidor)

Para rodar em uma máquina sem tela (CI, servidor, WSL), é preciso um servidor X
virtual. Estas dependências **não** estão na imagem:

- `xvfb` (servidor X virtual);
- opcionalmente `x11vnc` + um cliente VNC para ver a janela;
- opcionalmente `virtualgl` para aceleração remota por GPU.

Exemplo de uso avulso (o `xvfb` precisa ser instalado na imagem antes; ele não
faz parte do `Dockerfile`):

```bash
docker compose run --rm -e DISPLAY=:99 app \
  sh -c "Xvfb :99 -screen 0 1024x768x24 & sleep 1 && ./curvas desenhos/mario.obj"
```
