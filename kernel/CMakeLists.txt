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
    >
    $<$<COMPILE_LANGUAGE:ASM_NASM>:
        -f elf32
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

