#ifndef AUTO_TESTE_H
#define AUTO_TESTE_H

#include <string>

// Teste automatizado de integracao: executa o visualizador de verdade,
// aplica cada funcionalidade exigida (luz, sombreamento, transparencia,
// especular, reflexao, textura, redimensionamento...), captura a imagem
// renderizada (PNG) e verifica o estado do OpenGL e os pixels.
//
// Ativado apenas por "visualizador3d --autoteste [pasta]" (uso do
// desenvolvedor/CI); o uso normal do programa e todo pela interface grafica.
// Ao terminar grava <pasta>/relatorio.txt e encerra com codigo 0 (sucesso)
// ou 1 (alguma verificacao falhou).
void iniciarAutoteste(const std::string& pastaSaida);

#endif
