# Opengame Core — API 0.1.1

## DeviceProfile

O aplicativo Android deverá preencher este perfil com dados obtidos em tempo de execução. O núcleo não presume fabricante, arquitetura, quantidade de memória ou suporte gráfico.

- `cpu_architecture`: arquitetura detectada; use `unknown` se não for possível identificar.
- `cpu_cores`: quantidade de núcleos lógicos reportada pelo sistema.
- `total_memory_bytes`: memória total reportada pelo Android.
- `android_api_level`: nível da API Android.
- `vulkan_supported`: resultado da consulta real de suporte Vulkan.
- `opengl_es_major/minor`: versão OpenGL ES reportada pelo dispositivo.

## SessionConfig

Define o motor pretendido, diretório de conteúdo e resolução. O diretório é apenas uma referência: esta versão não abre, executa nem modifica arquivos de jogos.

O chamador deve manter válido o armazenamento associado a `content_directory` enquanto a configuração estiver em uso, pois o campo é um `std::string_view`.

## Logging

O chamador pode fornecer um callback sem alocação para receber mensagens de diagnóstico. O callback e seu `user_data` devem permanecer válidos enquanto o núcleo puder emitir logs. O callback é declarado `noexcept`.

## Estado atual

A inicialização valida os parâmetros e prepara o núcleo. `start()` ainda retorna `false`, pois os motores PC e PS3 não estão implementados. O estado nunca muda para `running` apenas para simular uma execução.
