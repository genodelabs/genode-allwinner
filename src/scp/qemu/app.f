: .0x    48 emit 120 emit ;
: .:     58 emit ;
: .hex   .0x base @ >r hex <# # # # # # # # # #> type r> base ! ;
: #8bit  base @ >r 2 base ! # # # # # # # # r> base ! ;
: #.     46 hold ;
: .bits  <# #8bit #. #8bit #. #8bit #. #8bit #> type ;
: .reg   dup .hex space .: space space @ dup .hex space space .bits ;
: .regs  FOR AFT dup .reg cr cell + THEN NEXT drop ;

hex
