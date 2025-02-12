# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

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
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(Temporizador C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1
add_executable(Temporizador Temporizador.c)

# Set the name and version of the program
pico_set_program_name(Temporizador "Temporizador")
pico_set_program_version(Temporizador "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(Temporizador 1)
pico_enable_stdio_usb(Temporizador 1)

# Add the standard library to the build
target_link_libraries(Temporizador
        pico_stdlib
)

# Add the standard include files to the build
target_include_directories(Temporizador PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${PICO_SDK_PATH}/external/pico_cyw43_driver
        ${PICO_SDK_PATH}/lib/lwip/src/include
        ${PICO_SDK_PATH}/lib/lwip/src/include/ipv4
        ${PICO_SDK_PATH}/lib/lwip/src/include/ipv6
)

# Adicionar diretórios de bibliotecas
target_link_directories(Temporizador PRIVATE
        ${PICO_SDK_PATH}/lib/lwip/src
)

# Adicionando dependências do Wi-Fi e do lwIP
target_link_libraries(Temporizador
        pico_cyw43_arch_lwip_threadsafe_background  # Adicionado para o Wi-Fi
        lwip  # Adicionado para o lwIP
)

# Adicionar bibliotecas de hardware necessárias
target_link_libraries(Temporizador 
        hardware_timer
        hardware_clocks
        pico_cyw43_arch_lwip_threadsafe_background  # Adicionado para o Wi-Fi
        lwip  # Adicionado para o lwIP
)

# Gerar os arquivos extras para o projeto
pico_add_extra_outputs(Temporizador)
