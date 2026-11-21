# PC Engine — etapa 0.2

## Escopo inicial

O PC Engine começa com inspeção estática de executáveis no formato Portable Executable (PE), usado por programas Windows. O objetivo desta fase é reconhecer metadados sem carregar nem executar o arquivo.

## Dados identificados
- Assinatura DOS `MZ` e assinatura PE.
- Campo Machine: x86 (I386), x64 (AMD64), ARM e ARM64.
- Formato do cabeçalho opcional: PE32 ou PE32+.
- Quantidade de seções e flags de executável/DLL.

## Segurança e limites
A entrada é tratada como bytes não confiáveis. O parser verifica os limites dos campos, rejeita cabeçalhos truncados e limita a quantidade de seções reconhecida. Ele não executa código nem interpreta conteúdo de seções.

Este protótipo não é um carregador PE, tradutor de instruções, ambiente Windows ou emulador de jogos. Próximas pesquisas podem avaliar metadados adicionais e, separadamente, a viabilidade de uma camada de tradução.
