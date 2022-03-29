#
# \brief  Forth interpreter for the OpenRISC instruction-set architecture
# \author Norman Feske
# \date   2022-03-29
#
# Copyright (C) 2022 Genode Labs GmbH
#
# This file is part of the Genode OS framework, which is distributed
# under the terms of the GNU Affero General Public License version 3.
#

#
# Nomenclature
#
# Regular assembly labels are prefixed with an underscore. All labels without
# a leading underscore are execution tokens for the inner interpreter. This
# convention is useful because execution tokens are the common case.
#
# Glossary
#
#   TOS  -  top element of stack (value cached in register)
#   XT   -  execution token
#   IP   -  interpreter pointer (points at XT to execute next)
#   TIB  -  terminal-input buffer
#   CP   -  compile pointer (where a new XT or dict entry goes)
#
# Register use
#
#   r0      always zero
#   r8      scratch register used for stack operations
#   r9      link register (lr)
#   r10     IP (interpreter pointer)
#   r11     TOS (top stack element)
#   r12     TEXT base pointer (address of '_start')
#   r13     BSS base pointer
#   r14     data stack position relative to '_dstack_lo'
#   r15     return stack pointer
#
# The data and return stacks grow upwards. The stack position is not a pointer
# but an offset relative to respective start address. This simplifies bounds
# checking. The first two cells at '_dstack_lo' remain zero because the TOS is
# cached in r11, and elements are pushed with pre increment.
#

CELL_BYTES = 4   # length of a single cell in bytes
XTA_BYTES  = 2   # length of execution-token address in bytes

.text


##
## CPU reset and exception vectors
##

.section .text.vectors
.org 0x100   # reset vector
	l.j   _start
	l.nop
.section .end


##
## Utility macros
##

#
# Load effective address of 'symbol' into register 'reg'
#
.macro LEA reg symbol
	l.jal   . + 4    # obtain program counter in link register
	l.nop
	l.addi  \reg,lr,\symbol - .
.endm

#
# Load effective address of BSS 'symbol' into register 'reg'
#
.macro LEA_BSS reg symbol
	LEA \reg,_text_end
	l.addi  \reg,\reg,\symbol - _bss_start
.endm

#
# Print immediate value as hex at a separate line (for debugging)
#
.macro DBG_HEX imm
	l.movhi r1,(\imm >> 16)
	l.ori   r1,r1,(\imm & 0xffff)
	l.jal   _emit_hex
	l.nop
	l.ori   r1,r0,'\n'
	l.jal   _emit_char
	l.nop
.endm

.macro DBG_FREEZE
	l.j .; l.nop
.endm

#
# Compile a list of execution-token addresses
#
# Each element is a 16-bit offset from the base address ('_start').
#
.macro .. elements:vararg
.irp element,\elements
	.2byte \element - _start
.endr
.endm

.macro INSTALL_VECTOR vector handler
	LEA     r1,\handler
	l.addi  r1,r1,-\vector      # opcode for 'l.j handler'
	l.srli  r1,r1,2
	l.sw    \vector(r0),r1
	l.movhi r1,0x1500           # opcode for 'l.nop'
	l.sw    \vector+4(r0),r1
.endm


##
## Main program
##

.section .text.main
.global _start

_start:

	# magic instruction expected by ARM trusted firmware (0xb4 0x40 0x0 0x12)
	l.mfspr r2, r0, 0x12        # save exception vector address to r2

	INSTALL_VECTOR 0x200 _bus_error_exception
	INSTALL_VECTOR 0x600 _unaligned_access_exception

	LEA     r12,_start          # TEXT base register
	LEA_BSS r13,_bss_start      # BSS base register
	LEA     r10,_main           # initial interpreter pointer

_recover:
	l.or    r14,r0,r0           # reset data stack position
	l.or    r15,r0,r0           # reset return stack position
	l.j     _next
	l.nop

_bus_error_exception:
	LEA     r10,_error_bus_access
	l.j     _recover
	l.nop

_unaligned_access_exception:
	LEA     r10,_error_unaligned_access
	l.j     _recover
	l.nop

# data and return stack offsets relative to BSS base
_dstack_rel = _dstack_lo - _bss_start
_rstack_rel = _rstack_lo - _bss_start

_main:
	.. xtalit interpret_one_word eval_xt store
	.. xtalit do_uart mainloop_xt store
	.. xtalit emit_uart emit_xt store
	.. current_init current store
	.. cp_init cp store
	.. app_start app_bytes evaluate
	.. cr
	.. reset_tib
1:	.. mainloop_xt at execute branch 1b

_error_dstack_overflow:
	.. data_stack_error msg_overflow restart

_error_dstack_underflow:
	.. data_stack_error msg_underflow restart

_error_rstack_overflow:
	.. return_stack_error msg_overflow restart

_error_rstack_underflow:
	.. return_stack_error msg_underflow restart

_error_unaligned_access:
	.. cr msg_unaligned space msg_access restart

_error_bus_access:
	.. cr msg_invalid space msg_bus space msg_access restart

_reset:
	.. cr cr msg_reset restart

.align CELL_BYTES


##
## Inner interpreter
##

.macro NEXT
	l.j     _next
	l.nop
.endm

_next:
	l.lhz   r1,0(r10)               # 16-bit XT address
	l.add   r1,r12,r1               # add TEXT base pointer
	l.lhz   r2,0(r1)                # jump to TEXT-relative address in code field
	l.add   r2,r12,r2               # add TEXT base pointer
	l.jr    r2
	l.addi  r10,r10,XTA_BYTES       # increment IP

#
# r1  execution token to execute
#
_dolist:
	l.addi  r15,r15,CELL_BYTES      # push r10 to return stack
	l.add   r8,r13,r15
	l.sw    _rstack_rel(r8),r10
	l.j     _next
	l.addi  r10,r1,2                # set IP to first list item


##
## Primitives
##

.macro CODE_XT label
\label :
	.2byte . - _start + 4  # point to the following instruction
	.2byte 0
.endm

_dstack_overflow:
	LEA     r10,_error_dstack_overflow
	l.j     _recover
	l.nop

_dstack_underflow:
	LEA     r10,_error_dstack_underflow
	l.j     _recover
	l.nop

.macro CHECK_DSTACK_OVERFLOW
	l.sfgeui r14,_dstack_hi - _dstack_lo - CELL_BYTES
	l.bf    _dstack_overflow
.endm

.macro CHECK_DSTACK_ONE_ITEM
	l.sfltui r14,CELL_BYTES
	l.bf    _dstack_underflow
.endm

.macro CHECK_DSTACK_TWO_ITEMS
	l.sfltui r14,2*CELL_BYTES
	l.bf    _dstack_underflow
.endm

.macro PUSHD reg
	CHECK_DSTACK_OVERFLOW
	l.addi  r14,r14,CELL_BYTES
	l.add   r8,r13,r14            # BSS base + stack position
	l.sw    _dstack_rel(r8),r11
	l.or    r11,r0,\reg           # update TOS
.endm

.macro POPD
	CHECK_DSTACK_ONE_ITEM
	l.add   r8,r13,r14            # BSS base + stack position
	l.lwz   r11,_dstack_rel(r8)   # fetch TOS
	l.addi  r14,r14,-CELL_BYTES
.endm

#
# Common exit code paths of primitives, not inlined to conserve binary size
#

_popd_popd_next:
	POPD
_popd_next:
	POPD
	NEXT

_pushd_r1_next:
	PUSHD   r1
	NEXT

.macro POPD_NEXT      ; l.j _popd_next      ; l.nop ; .endm
.macro POPD_POPD_NEXT ; l.j _popd_popd_next ; l.nop ; .endm
.macro PUSHD_R1_NEXT  ; l.j _pushd_r1_next  ; l.nop ; .endm

#
# Data-stack primitives
#

CODE_XT drop   # ( w -- )
	POPD_NEXT

CODE_XT dup   # ( w -- w w )
	CHECK_DSTACK_ONE_ITEM
	PUSHD   r11
	NEXT

CODE_XT swap   # ( w1 w2 -- w2 w1 )
	CHECK_DSTACK_TWO_ITEMS
	l.add   r8,r13,r14                  # BSS base + stack position
	l.lwz   r1,_dstack_rel(r8)
	l.sw    _dstack_rel(r8),r11
	l.j     _next
	l.or    r11,r0,r1

CODE_XT over   # ( w1 w2 -- w1 w2 w1 )
	CHECK_DSTACK_TWO_ITEMS
	l.add   r8,r13,r14                  # BSS base + stack position
	l.lwz   r1,_dstack_rel(r8)
	PUSHD_R1_NEXT

CODE_XT qdup   # dup top of stack if its is not zero
	CHECK_DSTACK_ONE_ITEM
	l.sfeq  r0,r11
	l.bf    1f
	l.nop
	PUSHD   r11
1:	NEXT

#
# Return stack
#

_rstack_overflow:
	LEA     r10,_error_rstack_overflow
	l.j     _recover
	l.nop

_rstack_underflow:
	LEA     r10,_error_rstack_underflow
	l.j     _recover
	l.nop

.macro CHECK_RSTACK_OVERFLOW
	l.sfgeui r15,_rstack_hi - _rstack_lo - CELL_BYTES
	l.bf    _rstack_overflow
.endm

.macro CHECK_RSTACK_ONE_ITEM
	l.sfltui r15,CELL_BYTES
	l.bf    _rstack_underflow
.endm

.macro CHECK_RSTACK_TWO_ITEMS
	l.sfltui r15,2*CELL_BYTES
	l.bf    _rstack_underflow
.endm

CODE_XT tor   # ( w -- )  push TOS to return stack
	CHECK_RSTACK_OVERFLOW
	l.addi  r15,r15,CELL_BYTES
	l.add   r8,r13,r15
	l.sw    _rstack_rel(r8),r11
	POPD_NEXT

CODE_XT rat   # ( -- w )  copy top of return stack to data stack
	CHECK_RSTACK_ONE_ITEM
	l.add   r8,r13,r15
	l.lwz   r1,_rstack_rel(r8)
	PUSHD_R1_NEXT

CODE_XT tworat   # ( -- w w )  copy top two items of return stack to data stack
	CHECK_RSTACK_TWO_ITEMS
	l.add   r8,r13,r15
	l.lwz   r1,_rstack_rel-CELL_BYTES(r8)
	l.lwz   r2,_rstack_rel(r8)
	PUSHD   r2
	PUSHD_R1_NEXT

CODE_XT rfrom   # ( -- w )  move top of return stack to the data stack
	CHECK_RSTACK_ONE_ITEM
	l.add   r8,r13,r15
	l.lwz   r1,_rstack_rel(r8)
	l.addi  r15,r15,-CELL_BYTES
	PUSHD_R1_NEXT

#
# Comparison
#

CODE_XT zless   # ( n -- f )  return true if n is negative
	CHECK_DSTACK_ONE_ITEM
	l.addi   r1,r0,-1   # true flag value
	l.sflts  r11,r0
	l.j      _next
	l.cmov   r11,r1,r0

.macro CODE_XT_CMP label op  # ( a b -- f )  compare top-most stack elements
CODE_XT \label
	CHECK_DSTACK_TWO_ITEMS
	l.addi   r1,r0,-1     # true flag value
	l.add    r8,r13,r14   # BSS base + stack position
	l.lwz    r2,_dstack_rel(r8)
	l.\op    r2,r11
	l.cmov   r11,r1,r0
	l.j      _next
	l.addi   r14,r14,-CELL_BYTES
.endm

CODE_XT_CMP equal sfeq    # true if a = b
CODE_XT_CMP ult   sfltu   # true if unsigned a < b
CODE_XT_CMP slt   sflts   # true if signed a < b

#
# Literals embedded in XT lists
#
# Number literals are 32-bit values.
# XT address literals are 16-bit values relative to the base address in r12.
#

CODE_XT dolit   # fetch 32-bit number literal following the execution token
	l.lhz   r1,0(r10)           # high
	l.lhz   r2,2(r10)           # low
	l.slli  r1,r1,16
	l.or    r1,r1,r2
	l.addi  r10,r10,CELL_BYTES
	PUSHD_R1_NEXT

CODE_XT xtalit   # fetch 16-bit XTA literal following the execution token
	l.lhz   r1,0(r10)
	l.addi  r10,r10,XTA_BYTES
	PUSHD_R1_NEXT

CODE_XT branch
	l.lhz r10,0(r10)            # r10 points at address literal, redirect IP to it
	l.j   _next
	l.add r10,r12,r10           # add base address to 16-bit XT address

CODE_XT qbranch   # ( f -- ) branch if flag is zero
	CHECK_DSTACK_ONE_ITEM
	l.lhz   r1,0(r10)           # target of taken branch
	l.add   r1,r12,r1           # add base address to 16-bit XT address
	l.addi  r2,r10,XTA_BYTES    # next IP if branch not taken (skip XT address literal)
	l.sfeq  r11,r0              # set flag if TOS is zero
	l.cmov  r10,r1,r2           # redirect IP to either branch depending on flag
	POPD_NEXT

CODE_XT next   # ( -- ) decrement top of return stack, branch if not below zero
	CHECK_RSTACK_ONE_ITEM
	l.add   r8,r13,r15
	l.lwz   r1,_rstack_rel(r8)  # counter is located at return stack
	l.sfeq  r1,r0
	l.bf    1f                  # exit loop if counter is zero
	l.addi  r1,r1,-1            # decrement counter
	l.sw    _rstack_rel(r8),r1
	l.lhz   r10,0(r10)          # take branch
	l.j     _next
	l.add   r10,r12,r10         # add base address to 16-bit XT address
1:	l.addi  r10,r10,XTA_BYTES   # next IP if branch not taken (skip literal)
	l.j     _next
	l.addi  r15,r15,-CELL_BYTES # drop counter from return stack

#
# Arithmetics and logical operations
#

CODE_XT umplus   # ( w w -- w cy )
	CHECK_DSTACK_TWO_ITEMS
	l.add   r8,r13,r14
	l.lwz   r1,_dstack_rel(r8)
	l.add   r2,r11,r1
	l.ori   r1,r0,1          # determine carry (addc does not work on AR100)
	l.sfltu r2,r11
	l.cmov  r11,r1,r0
	l.j     _next
	l.sw    _dstack_rel(r8),r2

.macro CODE_XT_BITWISE label op
CODE_XT \label
	CHECK_DSTACK_TWO_ITEMS
	l.add   r8,r13,r14
	l.lwz   r1,_dstack_rel(r8)
	l.\op   r11,r11,r1
	l.j     _next
	l.addi  r14,r14,-CELL_BYTES
.endm

CODE_XT_BITWISE and and
CODE_XT_BITWISE or  or
CODE_XT_BITWISE xor xor

#
# Memory access
#

CODE_XT store   # ( w a -- ) store 'w' to address 'a'
	CHECK_DSTACK_TWO_ITEMS
	l.add   r8,r13,r14
	l.lwz   r1,_dstack_rel(r8)
	l.sw    0(r11),r1
	POPD_POPD_NEXT

CODE_XT at   # ( a -- w ) load 'w' from address 'a'
	CHECK_DSTACK_ONE_ITEM
	l.j     _next
	l.lwz   r11,0(r11)

CODE_XT cstore   # ( c b -- ) store char 'c' to byte address 'b'
	CHECK_DSTACK_TWO_ITEMS
	l.add   r8,r13,r14
	l.lwz   r1,_dstack_rel(r8)
	l.sb    0(r11),r1
	POPD_POPD_NEXT

CODE_XT cat   # ( b -- c ) load char 'c' from byte address 'b'
	CHECK_DSTACK_ONE_ITEM
	l.j    _next
	l.lbz  r11,0(r11)

CODE_XT xtastore   # ( xt a -- ) store XTA at 'a'
	CHECK_DSTACK_TWO_ITEMS
	l.add   r8,r13,r14
	l.lwz   r1,_dstack_rel(r8)
	l.sh    0(r11),r1
	POPD_POPD_NEXT

CODE_XT xtaat   # ( a -- xta ) load XTA from address 'a'
	CHECK_DSTACK_ONE_ITEM
	l.j    _next
	l.lhz  r11,0(r11)

CODE_XT xta_aligned_store   # ( w a -- ) store cell value 'w' at XTA address 'a'
	CHECK_DSTACK_TWO_ITEMS
	l.add   r8,r13,r14
	l.lwz   r1,_dstack_rel(r8)
	l.sh    2(r11),r1       # lower 16-bit halfword
	l.srli  r1,r1,16
	l.sh    0(r11),r1       # higher 16-bit halfword
	POPD_POPD_NEXT

#
# Input / output
#

CODE_XT rawdot   # print top stack element as hex number
	CHECK_DSTACK_ONE_ITEM
	l.or    r1,r0,r11
	l.jal   _emit_hex
	l.nop
	POPD_NEXT

CODE_XT emit_uart   # ( w -- ) output ASCII value
	CHECK_DSTACK_ONE_ITEM
	l.or    r1,r0,r11
	POPD
	l.jal   _emit_char
	l.nop
	NEXT

#
# Utilities
#

CODE_XT depth   # return number of stack elements
	l.srli  r1,r14,2
	PUSHD_R1_NEXT

CODE_XT exit   # counterpart of _dolist
	CHECK_RSTACK_ONE_ITEM
	l.add   r8,r13,r15
	l.lwz   r10,_rstack_rel(r8)
	l.j     _next
	l.addi  r15,r15,-CELL_BYTES

CODE_XT execute   # execute XT found at TOS
	CHECK_DSTACK_ONE_ITEM
	l.add   r1,r12,r11  # add base pointer to XT address
	POPD
	l.lhz   r2,0(r1)    # jump to TEXT-relative address in code field
	l.add   r2,r12,r2
	l.jr    r2
	l.nop

CODE_XT reset
	LEA     r10,_reset
	l.j     _recover
	l.nop

CODE_XT aligned   # ( a n -- a ) align 'a' to an address multiple of 'n'
	CHECK_DSTACK_TWO_ITEMS
	l.addi  r1,r11,-1
	l.xori  r2,r1,-1    # align mask is !(n - 1)
	POPD
	l.add   r11,r11,r1
	l.j     _next
	l.and   r11,r11,r2


##
## Numeric constant execution tokens
##

.macro CONST_XT label value
	.align CELL_BYTES
\label :
	.2byte _dolist - _start
	..     dolit
	.long  \value
	..     exit
.endm

CONST_XT zero         0
CONST_XT one          1
CONST_XT two          2
CONST_XT nine         9
CONST_XT ten          10
CONST_XT sixteen      16
CONST_XT thirty_one   31
CONST_XT neg_one      "-1"
CONST_XT true         "-1"
CONST_XT false        0
CONST_XT bl_char      ' '
CONST_XT a_char       'a'
CONST_XT z_char       'z'
CONST_XT tab_char     9
CONST_XT cr_char      13
CONST_XT nl_char      '\n'
CONST_XT bs_char      8          # backspace
CONST_XT del_char     127        # del
CONST_XT reset_char   18         # control-r
CONST_XT zero_char    '0'
CONST_XT nine_char    '9'
CONST_XT minus_char   '-'
CONST_XT offset_9_a   'a'-'9'-1  # offset between ASCII codes for '0' and 'a'
CONST_XT pad          _pad_hi
CONST_XT cell         CELL_BYTES
CONST_XT stack_start  _dstack_lo + 2*CELL_BYTES
CONST_XT xt_base_ptr  _start     # base pointer of XT addresses
CONST_XT app_start    _app_lo
CONST_XT app_bytes    _app_hi - _app_lo
CONST_XT cp_init      _bss_end
CONST_XT dolist_ptr   _dolist - _start


##
## Higher-level execution tokens
##

#
# Header of list execution token
#
.macro LIST_XT label
	.align XTA_BYTES
\label :
	.2byte _dolist - _start
.endm

#
# Shorthand for simple list execution tokens without branches,
# exiting at the end
#
.macro XT label tokens:vararg
LIST_XT \label
	.. \tokens exit
.endm

XT twodrop  drop drop
XT twodup   over over
XT rot      tor swap rfrom swap
XT plus     umplus drop
XT not      neg_one xor
XT inc      one plus
XT negate   not inc
XT sub      negate plus
XT incvar   dup at inc swap store

XT cellinc       cell plus
XT celldec       cell sub
XT cells         dup plus dup plus
XT cell_aligned  cell aligned
XT xta_aligned   two  aligned


# ( s -- b n ) return byte address 'b' and length 'n' for counted string 's'
XT count   dup inc swap cat

#
# The algorithms for umdiv_mod and ummul are transcribed from "eForth and Zen",
# pages 53 and 55.
#

# ( udl udh u -- ur uq )  divide double unsigned number, return remainder and quotient
LIST_XT umdiv_mod
	.. twodup ult
	.. qbranch 1f
	..   negate
	..   thirty_one tor
	     # for
2:	..     tor
	..     dup umplus
	..     tor tor dup umplus
	..     rfrom plus dup
	..     rfrom rat swap
	..     tor umplus
	..     rfrom or
	..     qbranch 3f
	..       tor drop
	..       inc rfrom
	..       branch 4f
	       # else
3:	..       drop
	       # then
4:	..     rfrom
	..   next 2b
	..   drop swap exit
1:	.. drop twodrop neg_one dup
	.. exit

# ( u1 u2 -- udl udh ) return double product of two unsigned numbers
LIST_XT ummul
	.. zero swap
	.. thirty_one tor
1:	..   dup umplus tor tor
	..   dup umplus
	..   rfrom plus
	..   rfrom
	..   qbranch 2f
	..     tor over umplus rfrom plus
2:	.. next 1b
	.. rot drop exit

# ( n n -- n ) product of two unsigned numbers
XT mul  ummul drop

# ( u ul uh -- f ) true if u is within ul (inclusive) and uh (exclusive)
XT within  over sub tor sub rfrom ult

# ( n - n ) absolute value of 'n'
LIST_XT abs
	.. dup zless qbranch 1f negate
1:	.. exit

# ( a1 a2 n -- f ) true if byte sequences are equal
LIST_XT bytes_equal
	.. dup zero equal not qbranch 2f  # n = 0
	.. one sub                        # for loop count is n-1
	.. true                           # result
	.. swap tor                       # n-1 to return stack
1:	..   tor twodup cat swap cat equal rfrom and  # compare chars
	..   tor inc swap inc rfrom       # increment pointers
	.. next 1b
	.. swap drop swap drop            # drop running pointers a1 and a2
	.. exit
2:	.. twodrop drop true exit

# ( s a n -- f ) true if chars ( a n ) match string s
LIST_XT ask_string_match
	.. tor swap dup cat rfrom      # ( a s length n )
	.. equal qbranch 1f            # check if string length matches 'n'
	..   count bytes_equal exit    # check if content matches
1:	.. twodrop false exit

# ( a1 a2 u -- ) copy u bytes from a1 to a2
LIST_XT cmove
	.. qdup qbranch 2f             # check u = 0
	.. one sub tor                 # for-loop counter ( a1 a2 )
1:	..   tor dup cat               # read byte from a1
	..   rat cstore                # write byte to a2
	..   inc                       # increment a1
	..   rfrom inc                 # increment a2
	.. next 1b
2:	.. twodrop exit


#
# Variables
#

XT dovar   rfrom cell_aligned

.macro VAR_XT label, value
	.align XTA_BYTES
\label :
	.2byte _dolist - _start
	..     dovar
	.align CELL_BYTES
	.long \value
.endm

VAR_XT cp    0   # current pointer to end of dictionary
VAR_XT base  10  # radix for input and output of numbers
VAR_XT hld   0   # pointer into _pad buffer

VAR_XT latest_dentry 0   # pointer to dictionary entry under construction
VAR_XT eval_xt       0   # interpret_one_word or compile_one_word
VAR_XT emit_xt       0   # XT used to output one character
VAR_XT mainloop_xt   0   # XT repeatedly called in main loop

XT state_compile    xtalit compile_one_word   eval_xt store
XT state_interpret  xtalit interpret_one_word eval_xt store


#
# Error-handling helpers
#

XT data_stack_error    cr msg_data msg_hyphen msg_stack space
XT return_stack_error  cr msg_return msg_hyphen msg_stack space
XT restart             cr branch _main


##
## Outer interpreter
##

XT emit     emit_xt at execute
XT space    bl_char emit
XT cr       nl_char emit cr_char emit
XT hex      sixteen base, store
XT decimal  ten base store

# ( u -- c ) convert digit 'u' to ASCII character
XT digit  nine over ult offset_9_a and plus zero_char plus

# ( n base -- n/base c ) extract least significant digit from 'n'
XT extract  zero swap umdiv_mod swap digit

# ( -- ) prepare output of a number, reset HLD to end of pad
XT fmt_start  pad hld store

# ( c -- ) insert character into the pad buffer
XT hold  hld at one sub dup hld store cstore

# ( u -- u/base ) extract one digit from 'u' into pad buffer
XT fmt_digit  base at extract hold

# ( u -- 0 ) convert 'u' into digits in the pad buffer
LIST_XT fmt_number
1:	.. fmt_digit dup
	.. qbranch 2f
	.. branch 1b
2:	.. exit

# ( n -- ) prepend sign to the pad buffer if 'n' is negative
LIST_XT fmt_sign
	.. zless qbranch 1f
	.. minus_char hold
1:	.. exit

# ( w -- b u ) prepare the output string for 'type'
XT fmt_end  drop hld at pad over sub

# ( b u -- ) output 'u' characters starting at byte address 'b'
LIST_XT type
	.. dup qbranch 2f    # skip loop if u = 0
	.. one sub tor       # init for-loop counter at return stack
1:	..   dup cat emit    # emit character
	..   inc             # increment b
	.. next 1b
	.. drop exit
2:	.. twodrop exit

# ( n -- b u ) convert signed integer to string in pad
XT signed_str  dup tor abs fmt_start fmt_number rfrom fmt_sign fmt_end

# ( w -- ) display unsigned number preceded by a space
XT udot  fmt_start fmt_number fmt_end space type

# ( w -- ) display integer preceded by a space
LIST_XT dot
	.. base at ten xor qbranch 1f
	.. udot exit
1:	.. signed_str space type exit

LIST_XT show_stack
	.. depth qbranch 3f             # check depth 0
	.. depth two ult not qbranch 2f # check depth 1
	.. depth two sub tor            # for-loop counter
	.. stack_start zero             # ( base offset )
1:	..   twodup plus at dot         # print stack element
	..   cell plus                  # increment byte offset
	.. next 1b
	.. twodrop                      # drop ( base offset )
2:	.. dup dot                      # print TOS
3:	.. exit

XT domsg   rfrom count type

.macro MSG_XT label text
LIST_XT \label
	.. domsg
	.byte  2f - 1f
1:	.ascii "\text"
2:	.align CELL_BYTES
.endm

MSG_XT msg_prompt  "> "
MSG_XT msg_reset   "reset!"
MSG_XT msg_hyphen  "-"
MSG_XT msg_quote   "'"
MSG_XT msg_colon   ":"

.macro MSG_XT_WORDS words:vararg
.irp word,\words
MSG_XT msg_\word "\word"
.endr
.endm

MSG_XT_WORDS data stack underflow overflow invalid bus access unaligned return
MSG_XT_WORDS unknown word missing token after tick
MSG_XT_WORDS variable name


##
## Dictionary
##

LAST_DENTRY = 0

IMMEDIATE = 1  # flag word to be interpreted in compile state

#
# Create static dictionary entry
#
.macro DENTRY name xta immediate=0
	.long LAST_DENTRY                    # pointer to previous dictionary entry
LAST_DENTRY = . - 4
	.2byte (\xta - _start) | \immediate  # XTA, bit 0 holds immediate flag
	.byte  2f - 1f                       # length of name
1:	.ascii "\name"
2:	.align CELL_BYTES
.endm

.macro DENTRIES names:vararg
.irp name,\names
DENTRY \name \name
.endr
.endm

# utilities
DENTRIES depth words execute reset
DENTRY "'"       tick

# stack operations
DENTRIES drop dup swap over rot
DENTRY "2drop"   twodrop
DENTRY "2dup"    twodup
DENTRY "?dup"    qdup
DENTRY ">r"      tor
DENTRY "r@"      rat
DENTRY "r>"      rfrom

# arithmetic and logic operations
DENTRIES negate abs not and or xor
DENTRY "um+"     umplus
DENTRY "+"       plus
DENTRY "-"       sub
DENTRY "um/mod"  umdiv_mod
DENTRY "um*"     ummul
DENTRY "*"       mul
DENTRY "1+"      inc

# output
DENTRIES base decimal hex emit space cr count type hold
DENTRY "."       dot
DENTRY "u."      udot
DENTRY "<#"      fmt_start
DENTRY "#>"      fmt_end
DENTRY "#"       fmt_digit
DENTRY "#s"      fmt_number
DENTRY "sign"    fmt_sign
DENTRY ".s"      show_stack

# comparison
DENTRIES within
DENTRY "0<"      zless
DENTRY "u<"      ult
DENTRY "<"       slt
DENTRY "="       equal

# memory access
DENTRIES cell cells cmove aligned
DENTRY "!"       store
DENTRY "@"       at
DENTRY "c!"      cstore
DENTRY "c@"      cat
DENTRY "cell+"   cellinc
DENTRY "cell-"   celldec

# compiler
DENTRIES here variable xtalit
DENTRY ":"       colon
DENTRY ";"       semicolon       IMMEDIATE
DENTRY "["       state_interpret IMMEDIATE
DENTRY "]"       state_compile   IMMEDIATE

# interpreter
DENTRIES evaluate do_uart emit_uart reset_tib
DENTRY "'mainloop"  mainloop_xt
DENTRY "'emit"      emit_xt

# control structures
DENTRY "IF"      compile_if    IMMEDIATE
DENTRY "THEN"    compile_then  IMMEDIATE
DENTRY "BEGIN"   compile_begin IMMEDIATE
DENTRY "AGAIN"   compile_again IMMEDIATE
DENTRY "UNTIL"   compile_until IMMEDIATE
DENTRY "FOR"     compile_for   IMMEDIATE
DENTRY "NEXT"    compile_next  IMMEDIATE
DENTRY "AFT"     compile_aft   IMMEDIATE

CONST_XT current_init LAST_DENTRY   # reset value
VAR_XT   current      LAST_DENTRY   # head of dictionary-entry list

XT dentry_name      cellinc two plus          # return name ptr (counted string)
XT dentry_xta       cellinc xtaat one not and # return XTA without immediate flag
XT dentry_immediate cellinc xtaat one and     # return immediate flag

# ( -- ) list dictionary content
LIST_XT words
	.. current at
1:	.. dup qbranch 2f
	.. dup dentry_name count space type  # print word
	.. at                                # traverse list
	.. branch 1b
2:	.. drop exit

# ( a n -- a | f )
LIST_XT dentry_by_name
	.. tor tor                        # keep ( a n ) at return stack
	.. current at                     # dentry pointer
1:	.. qdup qbranch 2f                # end of dict if dentry pointer is zero
	..   dup dentry_name tworat ask_string_match not qbranch 3f
	..   at                           # follow linked list
	..   branch 1b
2:	.. false
3:	.. rfrom rfrom twodrop exit


##
## Parser and interpreter
##

XT ask_newline         dup nl_char equal swap cr_char equal or
XT ask_space           bl_char equal
XT ask_tab             tab_char equal
XT ask_whitespace      dup ask_space  over ask_tab or  swap ask_newline or
XT ask_non_whitespace  ask_whitespace not

# ( a n xta -- count ) return number of matching characters
LIST_XT scan
	.. swap                           # ( a xta n )
	.. qdup qbranch 1f                # handle n = 0
	..   one sub tor                  # ( a xta )
	..   zero swap                    # ( a count xta )
	     # for
2:	..     tor                        # save xta at return stack
	..     twodup plus cat            # access character at a + count
	..     rat execute not qbranch 3f # evaluate match function xta
	..       rfrom drop               # drop xta
	..       swap drop                # drop a
	..       rfrom drop exit          # drop for-loop index
3:	..     inc                        # increment count
	..     rfrom                      # ( a count xta )
	..   next 2b
	..   drop                         # drop xta
	..   swap drop exit
1:	.. drop drop zero exit

# ( a n -- count ) return up to 'n' number of whitespace characters at 'a'
XT num_whitespace  xtalit ask_whitespace scan

# ( a n -- count ) return up to 'n' number of non-whitespace characters at 'a'
XT num_non_whitespace  xtalit ask_non_whitespace scan

# ( a n count -- a+count n-count )
XT advance  dup tor sub swap rfrom plus swap

XT skip_whitespace  twodup num_whitespace advance

# ( c -- false | n true )
LIST_XT ask_digit
	.. dup zero_char nine_char inc within over a_char z_char inc within or qbranch 1f
	.. dup a_char ult not offset_9_a and sub
	.. zero_char sub
	.. dup base at ult qbranch 1f   # bound check against base
	.. true exit
1:	.. drop false exit

# ( a n -- false | n true )
LIST_XT ask_number
	.. qdup qbranch 4f              # check n = 0
	.. one sub tor                  # for-loop counter
	.. zero                         # ( a result )
1:	..   swap dup cat               # ( result a c )
	..   ask_digit qdup qbranch 3f
	..     drop                     # drop true flag -> ( result a digit )
	..     rot                      # ( a digit result )
	..     base at ummul
	..     zero equal qbranch 2f    # check for multiply overflow
	..     plus                     # result*base + digit
	..     tor inc rfrom            # increment a -> ( a result )
	.. next 1b
	.. swap drop                    # drop a, keep result
	.. true exit
2:	.. drop
3:	.. rfrom twodrop
4:	.. drop
	.. false exit


VAR_XT parse_ptr 0   # current parse pointer
VAR_XT parse_len 0   # number of present bytes at 'parse_ptr'

# ( -- a wordlen ) | ( -- false )
LIST_XT scan_one_word
	.. parse_ptr at parse_len at     # ( a n )
	.. twodup num_whitespace advance
	.. qdup qbranch 1f
	.. twodup                        # ( a n a n )
	.. num_non_whitespace            # ( a n wordlen )
	.. tor over swap rat             # ( a a n wordlen )
	.. advance                       # ( a a+wordlen n-wordlen )
	.. parse_len store
	.. parse_ptr store
	.. rfrom exit                    # ( a wordlen )
1:	.. false exit                    # ( false )

#
# Interpret 'parse_len' bytes in buffer at 'parse_ptr'
#
# Both variables are updated while interpreting the buffer.
#
LIST_XT interpret_one_word   # ( -- f )
	.. scan_one_word
	.. qdup qbranch 2f               # end of buffer
	.. twodup                        # remember word for error message
	   # try to parse word as number
	.. twodup ask_number qdup qbranch 1f
	..   drop                        # drop true flag returned by ask_number
	..   tor twodrop twodrop rfrom   # keep number on stack
	..   true exit
1:	   # try to lookup word in dictionary
	.. dentry_by_name                # ( ... xta | f )
	.. qdup qbranch 3f
	.. rot rot twodrop               # no error, drop word info
	.. try_exec_dentry
	.. true exit
2:	.. drop false exit
3:	.. cr msg_unknown space msg_word space msg_quote type msg_quote cr
	.. false exit

# ( -- xta ) lookup XTA for next word in parse buffer
LIST_XT tick
1:	.. scan_one_word                 # ( a n ) | ( false )
	.. qdup qbranch 2f               # end of buffer
	.. twodup                        # remember word for error message
	   # try to lookup word in dictionary
	.. dentry_by_name dentry_xta     # ( xta | f )
	.. qdup qbranch 3f               # check for unexpected end of buffer
	.. rot rot twodrop               # no error, drop word info
	.. exit                          # ( xta )
2:	.. cr msg_missing space msg_token space msg_after space msg_tick cr exit
3:	.. cr msg_unknown space msg_word space msg_quote type msg_quote cr exit

XT incr_one   dup at inc swap store
XT incr_cell  dup at cell plus swap store

XT xta_align_cp   cp at xta_aligned cp store
XT cell_align_cp  cp at cell_aligned cp store
XT advance_cp     cp at plus cp store

# ( n -- ) append cell-sized value at CP
XT compile_cell  cp at xta_aligned_store  cp incr_cell

# ( n -- ) append XT address at CP
XT compile_xta  cp at xtastore  two advance_cp

# ( b -- ) append byte at CP
XT compile_byte  cp at cstore  cp incr_one

# ( a n -- ) append counted string at CP
LIST_XT compile_string
	.. dup compile_byte      # marshal count byte
	.. tor cp at rat cmove   # copy characters ( n )
	.. rfrom advance_cp      # advance CP by string length
	.. exit

XT compile_exit  xtalit exit compile_xta

# ( xta -- ) set XT field of latest dictionary entry
XT update_latest_dentry_xta      latest_dentry at cell plus xtastore
XT mark_latest_dentry_immediate  latest_dentry at cell plus dup xtaat one or xtastore
XT commit_latest_dentry          latest_dentry at current store

# ( -- xta ) XTA matching the current CP
XT here  cp at xt_base_ptr sub

# ( a n -- ) create dictionary entry for word ( a n )
LIST_XT create_dentry
	.. cell_align_cp
	.. cp at latest_dentry store     # remember dictionary under construction
	.. current at compile_cell       # set next-entry pointer
	.. zero compile_xta              # XTA field will be filled out below
	.. compile_string
	.. cell_align_cp                 # align start of LIST_XT
	.. here
	.. update_latest_dentry_xta
	.. dolist_ptr compile_xta        # compile XT header
	.. exit

# ( a n -- ) create dictionary entry for variable ( a n )
LIST_XT variable
	.. scan_one_word                 # ( a n ) | ( false )
	.. qdup qbranch 2f               # end of buffer
	.. create_dentry
	.. xtalit dovar compile_xta
	.. cell_align_cp
	.. zero compile_cell
	.. commit_latest_dentry
	.. exit
2:	.. cr msg_missing space msg_variable space msg_name cr exit

# ( -- ) create new dictionary entry for next word in parse buffer
LIST_XT colon
	.. scan_one_word                 # ( a n ) | ( false )
	.. qdup qbranch 2f               # end of buffer
	.. create_dentry
	.. state_compile
	.. exit
2:	.. cr msg_missing space msg_token space msg_after space msg_colon cr exit

LIST_XT semicolon
	.. compile_exit
	.. commit_latest_dentry
	.. state_interpret
	.. exit

LIST_XT compile_one_word
	.. scan_one_word
	.. qdup qbranch 2f               # end of buffer
	.. twodup                        # remember word for error message
	   # try to parse word as number
	.. twodup ask_number qdup qbranch 1f
	..   drop                        # drop true flag returned by ask_number
	..   xtalit dolit compile_xta    # compile dolit
	..   compile_cell                # ...followed by number
	..   twodrop twodrop
	..   true exit
1:	   # try to lookup word in dictionary
	.. dentry_by_name                # ( ... xta | f )
	.. qdup qbranch 3f
	.. rot rot twodrop               # no error, drop word info
	.. dup dentry_immediate qbranch 5f
	..   try_exec_dentry             # execute word if marked as immediate
	..   true exit
5:	.. dentry_xta
	.. compile_xta
	.. true exit
2:	.. drop false exit
3:	.. cr msg_unknown space msg_word space msg_quote type msg_quote cr
	.. false exit

XT compile_qbranch  xtalit qbranch compile_xta
XT compile_branch   xtalit branch  compile_xta

# ( -- a ) compile conditional branch
LIST_XT compile_if
	.. compile_qbranch
	.. here              # leave pointer to branch target on stack, consumed by 'then'
	.. zero compile_xta  # placeholder for branch target
	.. exit

# ( a -- ) resolve target address of conditional branch
XT compile_then  here swap xt_base_ptr plus xtastore

# ( -- a ) return branch target of begin-again loop
XT compile_begin  here

# ( a -- ) compile unconditional branch to 'a'
XT compile_again  compile_branch compile_xta

# ( a -- ) compile conditional branch to 'a'
XT compile_until  compile_qbranch compile_xta

# ( -- a ) compile start of for loop, leave branch target in stack
XT compile_for  xtalit tor compile_xta here

# ( a -- ) compile end of for loop, using branch target of 'compile_for'
XT compile_next  xtalit next compile_xta compile_xta

# ( a -- a-next a-then ) redirect non-first iterations of for loop
LIST_XT compile_aft
	.. drop                                  # drop branch target of 'compile_for'
	.. compile_branch here zero compile_xta  # add unconditionally branch to THEN
	.. here                                  # leave branch target for NEXT
	.. swap exit                             # ( a-next a-then )


##
## Input / output
##

.align CELL_BYTES

.include "uart.inc"

CODE_XT ask_key   # ( -- false | c true )
	l.movhi r2,UART_HIGH
	l.ori   r3,r2,UART_LOW_LSR
	l.ori   r4,r2,UART_LOW_RDR
	UART_READ_R5_LSR
	l.andi  r5,r5,(1 << 0) # receive data ready?
	l.sfeq  r0,r5
	l.bnf   1f
	l.or    r1,r0,r0       # push false flag
	PUSHD_R1_NEXT
1:  # got key
	UART_READ_R1_RDR
	PUSHD   r1             # push received character
	l.addi  r1,r0,-1       # push true flag
	PUSHD_R1_NEXT

#
# r1  ASCII character
#
_emit_char:
	l.movhi r2,UART_HIGH
	l.ori   r3,r2,UART_LOW_LSR
	l.ori   r4,r2,UART_LOW_THR
1:	UART_READ_R5_LSR
	l.andi  r5,r5,(1 << 5) # transmit ready?
	l.sfne  r0,r5
	l.bnf   1b
	l.nop
	l.jr    lr
	UART_WRITE_THR_R1

#
# Output hexadecimal number
#
# r1  number
#
_emit_hex:
	l.or     r6,r0,r1     # backup r1 to r6
	l.or     r8,r0,r9     # backup LR (changed by call to _emit_char)
	l.ori    r7,r0,32-4
1:
	l.srl    r1,r6,r7
	l.andi   r1,r1,0xf    # extracted digit in r1
	l.ori    r2,r0,'a'-'9'-1
	l.sfgtui r1,9         # digit greater than 9?
	l.cmov   r2,r2,r0
	l.add    r1,r1,r2     # add 0 (for 0...9) or 7 (for A...F)
	l.addi   r1,r1,'0'
	l.jal    _emit_char
	l.nop
	l.sfeq   r7,r0
	l.bf     2f
	l.addi   r7,r7,-4
	l.j      1b
	l.nop
2:
	l.jr     r8           # use original link register
	l.nop


##
## Main loop
##

VAR_XT tib_complete 0   # true on carriage return, reset after interpreting

CONST_XT tib          _tib_lo               # counted string
CONST_XT tib_capacity _tib_hi - _tib_lo - 1 # exclude count byte

LIST_XT handle_backspace
	.. tib cat zero equal not qbranch 1f    # don't move below zero
	.. bs_char emit space bs_char emit      # move cursor back
	.. tib cat one sub tib cstore           # decrement TIB count
1:	.. exit

LIST_XT append_to_tib   # ( c -- ) insert character into TIB
	.. tib cat tib_capacity ult qbranch 1f  # check for TIB overflow
	.. dup emit                             # echo to terminal
	.. dup tib inc tib cat plus cstore      # insert char into TIB
	.. tib cat inc tib cstore               # increment TIB count
1:	.. drop exit

XT handle_cr  true tib_complete store

# ( c -- f ) return true if 'c' denotes carriage return
XT matches_cr  dup cr_char equal swap nl_char equal or

# ( c -- f ) return true if 'c' denotes backspace
XT matches_bs  dup bs_char equal swap del_char equal or

# ( c -- f ) return true if 'c' is the shortcut for reset
XT matches_reset  reset_char equal

LIST_XT handle_uart_char
	.. dup matches_cr    qbranch 1f drop handle_cr        exit; 1:
	.. dup matches_bs    qbranch 1f drop handle_backspace exit; 1:
	.. dup matches_reset qbranch 1f drop reset            exit; 1:
	.. append_to_tib
	.. exit

LIST_XT handle_uart_input
	.. ask_key qbranch 1f
	.. handle_uart_char
1:	.. exit

LIST_XT try_exec_dentry
	.. qdup qbranch 1f dentry_xta execute
1:	.. exit

XT evaluate_one_word  eval_xt at execute

# ( ptr len -- ) evaluate up to 'len' characters starting at 'ptr'
LIST_XT evaluate
	.. parse_len store parse_ptr store
1:	.. evaluate_one_word
	.. not qbranch 1b
	.. exit

LIST_XT interpret_tib
	.. cr
	.. tib count
	.. evaluate
	.. cr
	.. exit

XT show_prompt  depth fmt_start fmt_number fmt_end type msg_prompt
XT reset_tib    zero tib_complete store zero tib cstore show_prompt

LIST_XT try_eval_tib
	.. tib_complete at qbranch 1f interpret_tib reset_tib
1:	.. exit

LIST_XT do_uart
	.. handle_uart_input
	.. try_eval_tib
	.. exit


##
## Forth application interpreted before entering the mainloop
##

_app_lo: .incbin "app.f"; _app_hi:


##
## Memory outside the image
##

.align CELL_BYTES
_text_end:
.section .bss
_bss_start:

_rstack_lo: .ds.l  32; _rstack_hi:   # return stack
_dstack_lo: .ds.l  64; _dstack_hi:   # data stack
_pad_lo:    .ds.b 128; _pad_hi:      # string-formatting buffer
_tib_lo:    .ds.b 128; _tib_hi:      # terminal-input buffer

_bss_end:

#
# Dynamic dictionary grows from here...
#
