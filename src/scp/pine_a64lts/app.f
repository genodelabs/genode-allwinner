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

\ utilities
: /        0 swap um/mod swap drop ;
: invert   not ;
: lshift   FOR AFT dup + THEN NEXT ;
: rshift   FOR AFT 2 / THEN NEXT ;
: bitmask  1 swap lshift 1 - ;
: mdelay   1ab * FOR NEXT ;
: udelay   1 rshift FOR NEXT ;

\ bitfield access
: bf!  bitmask rot >r over lshift >r lshift r@ and r> invert r@ @ and or r> ! ;
: bf@  bitmask over lshift rot @ and swap rshift ;
: b!   1 bf! ;
: b@   1 bf@ ;

\ reduced serial bus (RSB) register addresses
: rsb 1f03400    ;
: ctrl  rsb      ;
: ccr   rsb 4  + ;
: stat  rsb c  + ;
: daddr rsb 10 + ;
: data  rsb 1c + ;
: pmcr  rsb 28 + ;
: cmd   rsb 2c + ;
: saddr rsb 30 + ;

: rtsaddr.bf  saddr 10 8 ;
: sladdr.bf   saddr 0 10 ;

: wait_stat    BEGIN stat @ 1 = UNTIL 1 stat ! ;
: start_trans  1 ctrl 7 b!  wait_stat ;

\ enable prcm_r two-wire interface (TWI)
: prcm_r_twi 1f01400 28 + ;
prcm_r_twi @ 40 or prcm_r_twi !

\ RSB reset (must be a complied word because of BEGIN/UNTIL)
: reset 1 ctrl 0 b!  BEGIN ctrl @ 0 = UNTIL ; reset

\ config CCR
1 ccr 0 b!  1 ccr 1 b!  2 ccr 8 2 bf!

\ trigger PMU init sequence
7c pmcr 10 8 bf!  1 pmcr 1f b!  wait_stat

\ set RSB slave
e8 cmd !  2d rtsaddr.bf bf!  3a3 sladdr.bf bf!  start_trans

\ ( reg -- ) prepare PMIC read/write operation
: setup_rw  daddr !  2d rtsaddr.bf bf!  0 sladdr.bf bf! ;

\ ( reg -- value ) read PMIC register
: pmic@  setup_rw  8b cmd !  0 data !  start_trans data @ ;

\ ( value reg -- ) write PMIC register
: pmic!  setup_rw  4e cmd !  data !  start_trans ;
