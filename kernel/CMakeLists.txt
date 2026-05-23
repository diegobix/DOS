set(SRCS
  entry.asm
  kernel.cpp
  printf.cpp
  drivers/vga.cpp
  drivers/serial.cpp
  arch/asm.cpp
  arch/gdt.cpp
)

add_executable(kernel.elf ${SRCS})

target_compile_options(kernel.elf PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:
        -ffreestanding
        -O2
        -Wall
        -Wextra
        -fno-exceptions
        -fno-rtti
        -g
        -masm=intel
    >
)

target_link_options(kernel.elf PRIVATE
  -ffreestanding
  -O2
  -nostdlib
  -T ${CMAKE_CURRENT_SOURCE_DIR}/linker.ld
  -lgcc
)

set_target_properties(kernel.elf PROPERTIES
  LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/linker.ld
)

target_include_directories(kernel.elf PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_custom_target(run
  COMMAND qemu-system-i386 -kernel $<TARGET_FILE:kernel.elf> -serial stdio
  DEPENDS kernel.elf
  COMMENT "Lanzando QEMU"
)

add_custom_target(debug
  COMMAND qemu-system-i386 -kernel $<TARGET_FILE:kernel.elf> -S -s &
  COMMAND gdb -ex "target remote localhost:1234" -ex "set disassembly-flavor intel" -ex "symbol-file $<TARGET_FILE:kernel.elf>"
  DEPENDS kernel.elf
  COMMENT "Lanzando QEMU modo debug en puerto 1234"
)

