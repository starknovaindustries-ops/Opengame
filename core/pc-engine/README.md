# PC Engine — 0.2.2 (experimental)

Inspector estático de arquivos Portable Executable (PE).

## API
- `inspect_pe(bytes)`: metadados básicos do cabeçalho PE.
- `inspect_pe_imports(bytes)`: módulos/DLLs e símbolos importados.
- `inspect_pe_exports(bytes)`: funções exportadas, ordinais, nomes e forwarders.

## Limites
- Importações: até 32 módulos e 64 símbolos por módulo.
- Exportações: até 128 funções por imagem.
- Nomes limitados a 127 caracteres mais terminador.
- RVAs devem mapear para dados brutos de seções.
- Índices de ordinais e tabelas são validados.
- Nenhuma DLL é carregada; nenhum símbolo é resolvido ou executado.

Não é um loader PE, ambiente Windows ou emulador de jogos. Não declara compatibilidade com jogos comerciais.
