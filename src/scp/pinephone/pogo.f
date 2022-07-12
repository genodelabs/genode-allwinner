\ configure INT pogo pin (PL12) as input
: r_pio   1f02c00 ;
: pl_cfg1 r_pio 4 + ;
: pl_data r_pio 10 + ;
pl_cfg1 @ ffff and pl_cfg1 !

\ direct output to UART
: output_uart  xtalit do_uart 'mainloop !  xtalit emit_uart 'emit ! ;

\ activate command line not before pogo pin INT gets connected to GND
: do_pogo  pl_data @ 1000 and 0 = IF output_uart reset_tib THEN ;
: mainloop  do_pogo do_mib ;
' mainloop 'mainloop !
