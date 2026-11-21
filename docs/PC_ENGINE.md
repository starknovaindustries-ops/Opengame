# PC Engine — etapas 0.2 e 0.2.1

## Inspeção básica
O inspector reconhece assinaturas DOS/PE, máquina (x86, x64, ARM e ARM64), formato PE32/PE32+, quantidade de seções e flags de executável/DLL.

## Inspeção estática de importações — 0.2.1
A API `inspect_pe_imports` lê a Import Directory do Optional Header e identifica:
- nomes de módulos/DLLs importados;
- símbolos importados por nome;
- símbolos importados por ordinal;
- endereços relativos (RVA) mapeados para dados brutos de seções.

O parser usa limites fixos: até 32 módulos e 64 símbolos por módulo, com nomes limitados a 127 caracteres mais terminador. RVAs sem correspondência em dados brutos de seção, strings sem terminador e tabelas truncadas são rejeitados.

## Segurança e limites
A entrada é tratada como bytes não confiáveis. A inspeção é somente estática: não carrega DLLs, não resolve símbolos no Windows, não executa código e não altera arquivos. O projeto ainda não implementa um carregador PE, Windows API, DirectX, tradução x86/x64 ou compatibilidade com jogos. Os testes usam apenas imagens PE sintéticas.
