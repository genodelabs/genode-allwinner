
# Allwinner AR100 accesses the UART via 32-bit bus transactions

UART_HIGH    = 0x01c2
UART_LOW_LSR = 0x8014  # line status register
UART_LOW_THR = 0x8000  # transmit hold register
UART_LOW_RDR = 0x8000  # receive data register

.macro UART_READ_R1_RDR  ; l.lwz   r1,0(r4) ; .endm
.macro UART_READ_R5_LSR  ; l.lwz   r5,0(r3) ; .endm
.macro UART_WRITE_THR_R1 ; l.sw    0(r4),r1 ; .endm
