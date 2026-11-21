# PC Engine — 0.2 (experimental)

A primeira entrega do PC Engine é um **inspector estático de metadados PE**. Ele reconhece assinaturas DOS/PE, máquina, formato PE32/PE32+, quantidade de seções e flags de executável/DLL.

Máquinas identificadas: x86, x64, ARM e ARM64. Formatos de imagem reconhecidos: PE32 e PE32+.

## Limites desta versão
- Não executa, carrega, modifica ou instala executáveis.
- Não traduz instruções x86/x64.
- Não implementa Windows API, DirectX, drivers ou chamadas de sistema.
- Não garante compatibilidade com jogos.
- Entradas malformadas são rejeitadas por verificações de limites antes de ler os campos.

O inspector é um primeiro componente de pesquisa, testado com imagens PE sintéticas criadas nos testes do projeto. Não são incluídos executáveis de terceiros.
