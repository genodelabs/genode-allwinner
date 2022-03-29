hex

: r_pio 1f02c00 ;
: pl_cfg1 r_pio 4 + ;
: pl_data r_pio 10 + ;

pl_cfg1 @ ffff and pl_cfg1 !

: do_pogo  pl_data @ 1000 and 0 = IF xtalit do_uart 'mainloop ! reset_tib THEN ;
' do_pogo 'mainloop !

