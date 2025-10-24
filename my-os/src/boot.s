.set ALIGN,    1<<0
.set MEMINFO,  1<<1
.set FLAGS,    ALIGN | MEMINFO
.set MAGIC,    0x1BADB002
.set CHECKSUM, -(MAGIC + FLAGS)

.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

.section .text
.global start
.type start, @function
start:
    movl $stack_top, %esp
    call kernel_main
    cli
    hlt
.size start, . - start

.section .bss
.align 16
stack_bottom:
.skip 16384 # 16 KiB
stack_top: