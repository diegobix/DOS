# OS Dev

# Building
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

# Linking
Cuando un programa se compila y linkea se suele usar una configuración de linking por defecto, que viene dada en un archivo conocido como linker script. En este archivo se definen qué secciones se mantienen, dónde y cómo se colocan y en qué direcciones de memoria, tanto física como virtual. 
Al estar en un entorno bare metal se debe pasar un linker script personalizado, principalmente por tres motivos:
1. GRUB coloca el kernel en la dirección que establezcamos en el linker script
2. El kernel tiene una sección entry en la que no hay paginación activa, por lo que las direcciones deben ser las físicas, y otra sección con paginación activa, en la que se deben usar las direcciones virtuales.
3. Alinear las secciones a 4KiB (alinearlas a las páginas de memoria).

## Objetivos
Con el linker script se pretende hacer lo siguiente:
- Establecer la entrada del kernel como kernel_entry
- Pedirle a GRUB que coloque el kernel en la dirección física 0x100000 (1 MiB)
- Mantener la sección .text.boot en direcciones físicas y el resto en direcciones virtuales a partir de 0xC0100000
- Alinear las secciones a páginas de memoria
- Exportar símbolos al kernel sobre comienzo y fin de secciones.

## Sintaxis
- `ENTRY(kernel_entry)` establece el símbolo kernel_entry como la entrada del programa. Para que sea visible por el linker script debe marcarse como global en asm o extern en cpp.
- `  . = KERNEL_PHYS_BASE;` Indica que lo que venga después se coloca desde la dirección KERNEL_PHYS_BASE, que es 0x100000.
- `.text.boot` En esta sección se coloca la entrada del kernel, parte que no tiene aún paginación activa por lo que debe linkarse con direcciones físicas. Se agrega la sección .multiboot al principio para que sea detectable por los bootloaders multiboot.
- `. += KERNEL_VIRT_BASE;` Se salta a direcciones virtuales a partir de este punto
- Ahora el resto de seccioens usan la sintaxis de AT: `.text ALIGN(4K) : AT(ADDR(.text) - KERNEL_VIRT_BASE)`. Estas secciones se linkan con direcciones virtuales establecidas en el punto anterior, pero con AT indicamos en qué dirección física se debe cargar la sección, que será la misma en la que esté - 0xC0000000. Al final la idea es que el kernel se cargue en dirección física 0x00100000 y dirección virtual 0xC0100000, que sean equivalentes y esten desplazadas por 0xC0000000.
```ld
    __stack_bottom = .;
    . += 0x4000;
    __stack_top = .;
```
- Esta parte de la sección .bss reserva 16KiB para el early stack del kernel.
## Secciones
- `.text.boot` -> Entrada del kernel sin paginación activa.
- `.text` -> Zona del código ejecutable.
- `.data` -> Zona de datos (variables y demás) modificables.
- `.rodata` -> Igual pero read only
- `.init_array` -> Sección especial de los programas en C++ que contiene la lista de constructores de objetos globales. Estos objetos se construyen al arrancar el proceso llamando a todos los constructores presentes en esta sección. Es algo que generalmente hace el runtime de C++ en linux, pero aquí tenemos que hacerlo a mano.
- `.bss` -> Datos static no inicializados. Se debe inicializar a 0 al arrancar el programa.
	- `.bss.bootstrap` -> Esta parte de bss contiene la primera page directory table usada en el arranque. No se debe poner a 0 hasta que la paginación se maneje por el kernel high half.
	- `__stack` -> Parte de .bss reservada para hacer de stack del kernel en el arranque. Se dejará de usar cuando se tenga gestión de memoria en el kernel.
# entry.asm
Este archivo es la entrada del kernel. Lo primero que debe hacer es definir el header Multiboot, con el fin de ser detectable por bootloaders compatibles con el estandar. Posteriormente se comienza a ejectuar código. Lo primero que se hace es establecer el stack en la zona que se había reservado, y posteriormente se comienza a activar la paginación. Esto es importante ya que se quiere que el kernel funcione en high half directamente para evitar futuros problemas de direccionamiento y demás.
La paginación que se activa es una temporal para el arranque, por lo que por conveniencia y facilidad se activan las páginas de 4MB, lo que permite tener que hacer solo dos entradas de la tabla, frente a las 3 tablas que habría que hacer con páginas de 4KB. 
Se mapean dos cosas:
- Identity paging: Se mapean de forma identitaria las primeras direcciones (los 4 primeros MB). Esto hará que la dirección 0x00100000 virtual corresponda con la misma física.
- High Half paging: Se mapean las direcciones a partir de 0xC0000000 a los primeros 4 MB físicos. Esto hará que la dirección 0xC0100000 virtual corresponda con la física 0x00100000.
![e4ea2e22a35f58d8eebfecfda20e9667.png](:/1098f302f2a2476db176d862a4e4eeb4)

La idea de esto es que se tenga acceso a las direccioens bajas de memoria en el arranque de forma sencilla, por ejemplo para poder usar el buffer VGA sin complicación. Cuando el proceso de arranque esté suficientemente avanzado se elimina el identity paging para dejar solo el high half.
La idea del High Half paging es que el kernel resida en las direcciones altas de la memoria virtual, de forma que los procesos de usuario ocupen el rango de direcciones bajo y tengan mapeado el kernel en la zona alta de memoria.
![67694a67a103d8f6bace9caba18449d1.png](:/4c11aa3b43314861954e26de6f8d8cf9)

Una vez se tiene activada la paginación se salta a kernel_entry_high para cambiar el EIP a direcciones altas. Tras ello se limpia la sección .bss y se salta a kernel_main, entrada del kernel ya en C++.
