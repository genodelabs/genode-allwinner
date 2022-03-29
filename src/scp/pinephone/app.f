: .0x    48 emit 120 emit ;
: .:     58 emit ;
: .hex   .0x base @ >r hex <# # # # # # # # # #> type r> base ! ;
: #8bit  base @ >r 2 base ! # # # # # # # # r> base ! ;
: #.     46 hold ;
: .bits  <# #8bit #. #8bit #. #8bit #. #8bit #> type ;
: .reg   dup .hex space .: space space @ dup .hex space space .bits ;
: .regs  FOR AFT dup .reg cr cell + THEN NEXT drop ;

: +!  dup @ 1+ swap ! ;

hex

\ direct output to UART
: output_uart  xtalit do_uart 'mainloop !  xtalit emit_uart 'emit ! ;

: r_pio 1f02c00 ;
: pl_cfg1 r_pio 4 + ;
: pl_data r_pio 10 + ;

pl_cfg1 @ ffff and pl_cfg1 !

\ activate command line not before pogo pin INT gets connected to GND
: do_pogo  pl_data @ 1000 and 0 = IF output_uart reset_tib THEN ;
' do_pogo 'mainloop !
