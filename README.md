# Opengame

**Opengame** é um projeto experimental da StarkNova Industries para pesquisar e desenvolver uma plataforma Android capaz de hospedar motores de compatibilidade e emulação de sistemas de jogos de PC e PlayStation 3.

> **Estado:** PC Engine 0.2.1 experimental — inspeção estática de metadados e importações PE. Nenhum jogo de PC ou PS3 é atualmente declarado como compatível ou funcional.

## Objetivos
- Criar um aplicativo Android para gerenciar jogos, configurações e sessões.
- Desenvolver um núcleo próprio para coordenar os subsistemas.
- Pesquisar compatibilidade com jogos de PC (x86/x64) e PlayStation 3 (PowerPC Cell).
- Construir uma camada de integração com CPU, GPU, memória, áudio e controles do Android.
- Priorizar testes reproduzíveis, documentação e componentes independentes.

## Arquitetura inicial
| Módulo | Responsabilidade |
|---|---|
| Android App | Interface, biblioteca e configurações |
| OpenGame Core | Ciclo de vida, perfil do dispositivo, configuração e diagnóstico |
| PC Engine | Inspeção estática experimental de metadados e importações PE; sem execução |
| PS3 Engine | Pesquisa de arquitetura Cell, PPU e SPU |
| Hardware Bridge | Integração com recursos do dispositivo |
| Native Layer | Componentes nativos em C/C++ |

## Tecnologias planejadas
- Kotlin e Jetpack Compose para o aplicativo Android.
- C++ e CMake para componentes nativos.
- Android NDK para integração nativa.
- Vulkan como caminho gráfico preferencial, com alternativas conforme o dispositivo.
- Oboe/AAudio para pesquisa de áudio de baixa latência.

## Estrutura
```text
Opengame/
├── android/app/
├── core/
│   ├── opengame-core/
│   └── pc-engine/
├── docs/
├── tests/
├── CMakeLists.txt
├── LICENSE
└── README.md
```

## Compilação
A fundação nativa usa CMake e C++20:
```bash
cmake -S . -B build -DOPENGAME_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

## Aviso sobre jogos e firmware
O Opengame não distribui jogos comerciais, BIOS, firmware, chaves ou arquivos protegidos de terceiros. O usuário é responsável por utilizar apenas conteúdo que tenha direito legal de usar. A compatibilidade depende de testes em dispositivos reais e pode ser limitada por desempenho, drivers e requisitos técnicos.

## Licença
Este repositório é protegido pela **StarkNova Direct Protection License (SDPL) v1.0**. Consulte [LICENSE](LICENSE). O acesso público não concede permissão para copiar, modificar, redistribuir ou explorar comercialmente o código, salvo autorização escrita ou direitos que não possam ser excluídos pela legislação aplicável.

## Organização
Projeto: StarkNova Industries  
Repositório: [starknovaindustries-ops/Opengame](https://github.com/starknovaindustries-ops/Opengame)
