set(SRCS
  entry.asm
  kernel.cpp
)

set_source_files_properties(entry.asm
  PROPERTIES LANGUAGE ASM_NASM
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

add_custom_target(run
  COMMAND qemu-system-i386 -kernel $<TARGET_FILE:kernel.elf>
  DEPENDS kernel.elf
  COMMENT "Lanzando QEMU"
)

add_custom_target(debug
  COMMAND qemu-system-i386 -kernel $<TARGET_FILE:kernel.elf> -S -s &
  COMMAND gdb -ex "target remote localhost:1234" -ex "set disassembly-flavor intel" -ex "symbol-file $<TARGET_FILE:kernel.elf>"
  DEPENDS kernel.elf
  COMMENT "Lanzando QEMU modo debug en puerto 1234"
)

