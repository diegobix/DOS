# OS Dev

## Building
En la primera iteración hice mi propio bootloader además del kernel, por lo que tenía dos partes completamente separadas. La stage 1, el bootsector, era 100% ensamblador, y la stage 2, kernel + setup, tenía una parte inicial en asm y luego pasaba a cpp. Para entender el proceso de building usaba comandos manuales que acababa metiendo en un makefile. Esto funciona bien y es fácil detectar errores, pero era un lío para irlo escalando.
En esta segunda iteración comienzo con el kernel bajo el estandar Multiboot 1, que permite que sea arrancable por bootloaders como grub o el interno de Qemu. Esto me permite además usar CMake para el kernel, ya que el ejjecutable final es un ELF.

Configurar CMake para usar ensamblador es un poco lío, pero son dos partes importantes.
### Configurar el toolchain
Por defecto CMake usa compiladores que encuentra en tu sistema para los lenguajes del proyecto, pero como estamos crosscompilando debemos especificar qué compilador queremos usar (i686-elf-g++), además del linker. También demos especificar qué se usa para compilar ensamblador y qué formato produce de objeto (elf32 en este caso). Para configurar esto se debe pasar un archivo de toolchain al configurar el proyecto con CMake, en el que se especifiquen estas opciones:
```cmake
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR x86)

set(CROSS_PATH "$ENV{HOME}/.local/opt/cross/bin")

set(CMAKE_C_COMPILER ${CROSS_PATH}/i686-elf-gcc)
set(CMAKE_CXX_COMPILER ${CROSS_PATH}/i686-elf-g++)
set(CMAKE_LINKER ${CROSS_PATH}/i686-elf-ld)
set(CMAKE_ASM_NASM_COMPILER nasm)
set(CMAKE_ASM_NASM_OBJECT_FORMAT elf32)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
```
También es importante configurar el proyecto sin lenguaje establecido, y establecerlo manualmente justo después. Esto se debe a que el toolchain se carga entre project y enable_language:
```cmake
project(dos LANGUAGES NONE)
enable_language(CXX ASM_NASM)
```
El toolchain se le puede pasar al configurar el proyecto en el comando de configuración:
`cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-x86.cmake`
### Establecer flags para cada compiler
Por defecto cmake no sabe separar las flags de compilación pasadas en `target_link_options` por lo que debemos especificar qué flags se usan para cada compilador o lenguaje. Para ello se puede usar la expresión generadora `$<$<COMPILE_LANGUAGE:CXX> -flag1 -flag2 >`, que hace que flag1 y flag2 solo se le pasen al compilador de cpp. A nasm hay que pasarle el formato de salida del objeto ensamblado, pero en vez de pasárselo como flag manual se ha configurado como opción en el toolchain `set(CMAKE_ASM_NASM_OBJECT_FORMAT elf32)`
