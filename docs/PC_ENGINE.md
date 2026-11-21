# PC Engine — inspeção PE experimental

## 0.2 — Metadados
Reconhece assinaturas DOS/PE, máquina (x86, x64, ARM e ARM64), PE32/PE32+, quantidade de seções e flags de executável/DLL.

## 0.2.1 — Importações
A API `inspect_pe_imports` lê a Import Directory e identifica módulos/DLLs e símbolos importados por nome ou ordinal. Usa limites de 32 módulos e 64 símbolos por módulo.

## 0.2.2 — Exportações
A API `inspect_pe_exports` lê a Export Directory e retorna:
- nome do módulo, quando presente;
- funções exportadas e seus ordinais;
- nomes associados aos índices da tabela de exportação;
- RVA de cada função;
- encaminhamentos (*forwarders*) cujo RVA aponta para dentro da própria Export Directory.

O parser limita a 128 funções por imagem, valida as tabelas de endereços, nomes e ordinais, verifica os índices e percorre strings por RVA sem ultrapassar os dados brutos mapeados de uma seção. Entradas com RVA zero são tratadas como slots não implementados e não são listadas.

## Segurança e limites
A inspeção é estática: não carrega DLLs, não resolve símbolos, não executa código e não altera arquivos. Não é um loader PE, ambiente Windows, tradutor de instruções ou emulador. Não há compatibilidade de jogos declarada. Os testes usam imagens PE sintéticas.

## 0.2.3 — Análise de dependências
A API `analyze_pe_dependencies(images)` recebe até 16 imagens PE nomeadas e correlaciona importações com módulos presentes no mesmo conjunto. O relatório indica módulo ausente, módulo encontrado, símbolos resolvidos, símbolos ausentes ou módulo inválido. A comparação por nome é sensível a maiúsculas/minúsculas apenas para símbolos; nomes de módulos são comparados sem distinção de caixa e usando o nome-base do arquivo.
A análise não consulta o Windows, não carrega módulos externos, não resolve forwarders e não prova compatibilidade em tempo de execução.
