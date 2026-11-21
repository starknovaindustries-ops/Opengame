# PC Engine — 0.2.1 (experimental)

O PC Engine contém um inspector estático de arquivos Portable Executable (PE). A versão 0.2.1 amplia a inspeção básica com leitura limitada da Import Directory.

## API
- `inspect_pe(bytes)`: valida metadados básicos do cabeçalho PE.
- `inspect_pe_imports(bytes)`: lista módulos importados e símbolos por nome/ordinal.

## Limites de segurança
- Até 32 módulos por imagem e 64 símbolos por módulo.
- Nomes limitados a 127 caracteres.
- RVA precisa apontar para dados presentes em uma seção e no arquivo.
- Tabelas e strings truncadas ou inválidas são rejeitadas.
- Nenhuma DLL é carregada e nenhum símbolo é resolvido ou executado.

Este componente não é um loader PE, ambiente Windows, tradutor de instruções ou emulador de jogos. Não declara compatibilidade com jogos comerciais.
