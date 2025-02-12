cmake_minimum_required(VERSION 3.13)

# Configuração da política CMP0057
if(POLICY CMP0057)
    cmake_policy(SET CMP0057 NEW)
endif()

# Configurações do projeto
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Configurações de build
set(CMAKE_BUILD_TYPE Debug)
add_compile_definitions(DEBUG=1)

# Configurações do compilador para o Cortex-M0+
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=cortex-m0plus -mthumb")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mcpu=cortex-m0plus -mthumb")

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

# Configurações específicas da BitDogLab
target_compile_definitions(Temporizador PRIVATE
    CYW43_HOST_NAME="BitDogLab_Timer"
    PICO_DEFAULT_LED_PIN=15  # LED integrado da BitDogLab
)

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
    hardware_gpio    # Suporte a GPIO
    hardware_adc     # Suporte a ADC
    hardware_i2c     # Suporte a I2C
    hardware_spi     # Suporte a SPI
)

# Configurações de compilação condicional
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(Temporizador PRIVATE
        DEBUG=1
        PICO_DEBUG=1
        PICO_DEBUG_UART=1     # Debug via UART
        PICO_STDIO_UART=1     # Saída via UART
        PICO_STDIO_USB=1      # Saída via USB
    )
endif()

# Geração dos arquivos extras
pico_add_extra_outputs(Temporizador)

# Configurações de memória
pico_set_binary_type(Temporizador copy_to_ram)
pico_set_boot_stage2(Temporizador boot2_generic_03h)

# Otimizações de build
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Og")    # Otimização para debug
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og")