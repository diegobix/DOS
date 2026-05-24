[bits 32]

%macro ISR_NOERR 1
global isr_stub_%1
isr_stub_%1:
  push dword 0
  push dword %1
  jmp isr_common
%endmacro

%macro ISR_ERR 1
global isr_stub_%1
isr_stub_%1:
  push dword %1
  jmp isr_common
%endmacro

extern isr_handler

ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERR   8
ISR_NOERR 9
ISR_ERR   10
ISR_ERR   11
ISR_ERR   12
ISR_ERR   13
ISR_ERR   14
ISR_NOERR 15
ISR_NOERR 16
ISR_ERR   17
ISR_NOERR 18
ISR_NOERR 19

isr_common:
  pushad

  push esp
  call isr_handler
  add esp, 4

  popad

  add esp, 8 ; Quitamos num de excepcion y err_code

  iret 

