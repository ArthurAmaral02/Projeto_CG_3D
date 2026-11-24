# Build do Visualizador 3D no Linux (ou dentro do Docker).
# No Windows use a solucao do Visual Studio (Projeto_CG_3D.sln) ou scripts/windows.ps1.
#
#   make              compila o visualizador (./visualizador3d)
#   make run          compila e abre o visualizador
#   make testes       compila e executa os testes unitarios
#   make autoteste    executa o autoteste visual (usa xvfb-run se nao houver DISPLAY)
#   make clean        remove os arquivos gerados

CXX      ?= g++
CXXFLAGS ?= -O2 -Wall -Wextra
CPPFLAGS += -Iinclude -isystem third_party/glui/include -isystem third_party/stb
LDLIBS   += -lglut -lGLU -lGL

STD      := -std=c++17
TARGET   := visualizador3d
TESTES   := build/testes

# Codigo do projeto
APP_SRC  := $(wildcard src/*.cpp)
APP_OBJ  := $(APP_SRC:src/%.cpp=build/app/%.o)

# GLUI compilada a partir do fonte em third_party/glui (mesmo commit da GLUI oficial)
GLUI_SRC := $(wildcard third_party/glui/src/*.cpp)
GLUI_OBJ := $(GLUI_SRC:third_party/glui/src/%.cpp=build/glui/%.o)

# Modulos sem OpenGL usados pelos testes unitarios
NUCLEO   := ArquivoOBJ Bezier Caminhos CoordenadasTextura Imagem Malha Matriz4 \
            OrdenacaoProfundidade Transformacoes
TEST_OBJ := build/tests/testes.o $(NUCLEO:%=build/app/%.o)

DEPS     := $(APP_OBJ:.o=.d) $(GLUI_OBJ:.o=.d) build/tests/testes.d

.PHONY: all run testes autoteste clean

all: $(TARGET)

$(TARGET): $(APP_OBJ) $(GLUI_OBJ)
	$(CXX) $^ $(LDLIBS) -o $@

$(TESTES): $(TEST_OBJ)
	$(CXX) $^ -o $@

build/app/%.o: src/%.cpp | build/app
	$(CXX) $(STD) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

build/tests/%.o: tests/%.cpp | build/tests
	$(CXX) $(STD) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

# A GLUI e codigo de terceiros: compilada sem avisos.
build/glui/%.o: third_party/glui/src/%.cpp | build/glui
	$(CXX) $(STD) -O2 -w -DGLUI_FREEGLUT -Ithird_party/glui/include -MMD -MP -c $< -o $@

build/app build/glui build/tests:
	mkdir -p $@

run: $(TARGET)
	./$(TARGET)

testes: $(TESTES)
	./$(TESTES)

# Sem servidor grafico (CI/Docker), cria um X virtual com xvfb-run.
autoteste: $(TARGET)
	@if [ -n "$$DISPLAY" ] && xdpyinfo >/dev/null 2>&1; then \
		./$(TARGET) --autoteste build/autoteste; \
	else \
		xvfb-run -a -s "-screen 0 1920x1080x24" ./$(TARGET) --autoteste build/autoteste; \
	fi

clean:
	rm -rf build/app build/glui build/tests build/autoteste $(TESTES) $(TARGET)

-include $(DEPS)
