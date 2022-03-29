
# UART is accessed via 8-bit bus transactions

UART_HIGH    = 0x9000
UART_LOW_LSR = 0x0005  # line status register
UART_LOW_THR = 0x0000  # transmit hold register
UART_LOW_RDR = 0x0000  # receive data register

.macro UART_READ_R1_RDR  ; l.lbz   r1,0(r4) ; .endm
.macro UART_READ_R5_LSR  ; l.lbz   r5,0(r3) ; .endm
.macro UART_WRITE_THR_R1 ; l.sb    0(r4),r1 ; .endm
