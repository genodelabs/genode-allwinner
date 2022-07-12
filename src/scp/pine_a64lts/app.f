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

\ mbox input buffer, 32-bit length value followed by characters
: mib 13c00 ;
: eval_mib mib 4 + mib @ evaluate ;

\ mbox output buffer, 32-bit length value followed by up to 1000 characters
: mob  13800 ;

\ ( c -- ) emit character to mbox output buffer
: emit_mob  mob @ 1000 < IF dup  mob @ mob + 4 + c!  mob +!  THEN drop ;

\ direct output to mbox output buffer
: output_mob  0 mob !  xtalit emit_mob 'emit ! ;

\ mbox request sequence number
variable seq

\ a request is submitted as sequence number in mbox channel 0, whereas
\ the completion is confirmed by returning the number via channel 1
: mbox 1c17000 ;
: pending  mbox 140 + @ ;
: req      mbox 180 + @ seq ! ;
: ack      seq @ mbox 184 + ! ;
: do_mib   pending IF req output_mob eval_mib ack THEN ;

' do_mib 'mainloop !
