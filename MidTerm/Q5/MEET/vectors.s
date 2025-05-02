.globl _start
.extern _stack_top
.extern _sbss
.extern _ebss
        .section "vectors"
reset:  b     _start
undef:  b     undef
swi:    b     swi
pabt:   b     pabt
dabt:   b     dabt
        nop
irq:    b     irq
fiq:    b     fiq
        .text
_start:
        @@ Initialize .bss
        ldr   r0, =_sbss
        ldr   r1, =_ebss
        ldr   r2, =_bss_size

        @@ Handle bss_size == 0
        cmp   r2, #0
        beq   init_stack

        mov   r4, #0
zero:
        strb  r4, [r0], #1
        subs  r2, r2, #1
        bne   zero
init_stack:        
    ldr sp, =_stack_top
    bl main
hang: b hang
