# Opengame — Arquitetura

## Visão geral
O Opengame é organizado em um aplicativo Android, um núcleo de orquestração e motores independentes. A separação permite desenvolver e testar cada subsistema sem declarar compatibilidade prematura.

## Camadas
- **Android App:** interface Kotlin/Jetpack Compose, biblioteca, configurações, permissões e controles.
- **OpenGame Core:** ciclo de vida, perfil do dispositivo, configuração de sessão e diagnóstico.
- **PC Engine (experimental):** pesquisa de compatibilidade x86/x64, tradução de instruções, APIs e chamadas de sistema.
- **PS3 Engine (experimental):** pesquisa da arquitetura Cell, PPU, SPU, memória e serviços do sistema.
- **Hardware Bridge:** abstração de CPU, GPU, memória, áudio, armazenamento e entrada do Android.
- **Native Layer:** componentes C/C++ com Android NDK, CMake e JNI quando necessário.

## Regras
1. Não declarar compatibilidade sem teste reproduzível.
2. Não incluir BIOS, firmware, chaves ou jogos comerciais no repositório.
3. Manter motores isolados e documentar dependências.
4. Registrar dispositivo, Android, driver e resultado de cada teste.
5. Começar com testes unitários e programas próprios.
6. Não presumir suporte Vulkan/OpenGL: consultar o dispositivo em tempo de execução.
7. Não marcar uma sessão como executando enquanto não existir um backend real.

## Limites
Emular PS3 é uma tarefa especialmente complexa. Compatibilidade, desempenho e estabilidade dependerão do hardware, dos drivers e da maturidade de cada subsistema.
