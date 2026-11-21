# Opengame — Roadmap

## 0.1 — Fundação
- [x] Visão e arquitetura inicial
- [x] README e licença SDPL
- [x] Biblioteca C++ do núcleo
- [x] Testes básicos do ciclo de vida
- [ ] Validar compilação em ambiente Android/NDK

## 0.1.1 — Perfil, configuração e logs
- [x] DeviceProfile, SessionConfig e logging
- [x] Validação dos parâmetros básicos
- [x] Impedir que o núcleo simule execução de jogos
- [ ] Executar testes em ambiente CMake/NDK

## 0.2 — PC Engine: inspeção PE experimental
- [x] Parser estático de metadados PE
- [x] Identificação de máquina e formato PE32/PE32+
- [x] Validação de cabeçalhos e seções
- [x] Testes com imagens PE sintéticas

## 0.2.1 — Importações PE
- [x] Leitura da Import Directory
- [x] Identificação de módulos/DLLs
- [x] Símbolos importados por nome e ordinal
- [x] Mapeamento de RVA para dados brutos de seções
- [x] Limites para módulos, símbolos e nomes
- [x] Testes sintéticos para importação válida e dados inválidos
- [ ] Validar compilação e testes em ambiente CMake/NDK e Android

## 0.3 — PS3 Engine (pesquisa)
- [ ] Documentar arquitetura Cell
- [ ] Prototipar componentes PPU isolados
- [ ] Pesquisar modelo de execução SPU
- [ ] Criar testes sintéticos

## 0.4 — Integração Android
- [ ] Aplicativo Kotlin/Compose
- [ ] Integração do núcleo via JNI
- [ ] Detecção de GPU e memória
- [ ] Controles e áudio

## 1.0 — Experimental
- [ ] Diagnóstico e logs
- [ ] Gerenciador de sessões
- [ ] Matriz de compatibilidade baseada em testes
- [ ] Documentação de instalação e limitações

As etapas são metas de desenvolvimento, não promessas de compatibilidade.
