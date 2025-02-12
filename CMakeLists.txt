# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
cmake_minimum_required(VERSION 3.13)

# Configurações do projeto
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Configurações de build
set(CMAKE_BUILD_TYPE Debug)
add_compile_definitions(DEBUG=1)

# Configurações de otimização
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")

# Configuração do Pico W
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Importação do SDK
include(pico_sdk_import.cmake)

# Definição do projeto
project(Temporizador C CXX ASM)

# Inicialização do SDK
pico_sdk_init()

# Definição dos arquivos fonte
add_executable(Temporizador Temporizador.c)

# Configurações do programa
pico_set_program_name(Temporizador "Temporizador")
pico_set_program_version(Temporizador "0.1")

# Configuração das saídas
pico_enable_stdio_uart(Temporizador 1)
pico_enable_stdio_usb(Temporizador 1)

# Diretórios de inclusão
target_include_directories(Temporizador PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${PICO_SDK_PATH}/external/pico_cyw43_driver
    ${PICO_SDK_PATH}/lib/lwip/src/include
    ${PICO_SDK_PATH}/lib/lwip/contrib/ports/embedded/include
)

# Bibliotecas necessárias
target_link_libraries(Temporizador
pico_stdlib
pico_cyw43_arch_lwip_threadsafe_background
pico_lwip  # Principal biblioteca lwIP
pico_lwip_nosys
pico_lwip_mqtt
hardware_timer
hardware_clocks
hardware_watchdog
)

# Configurações de compilação condicional
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(Temporizador PRIVATE
        DEBUG=1
        PICO_DEBUG=1
    )
endif()

# Geração dos arquivos extras
pico_add_extra_outputs(Temporizador)

# Configurações de memória
pico_set_binary_type(Temporizador copy_to_ram)
pico_set_boot_stage2(Temporizador boot2_generic_03h)