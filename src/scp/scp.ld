ENTRY(_start)

MEMORY {
	VECTORS (rwx): ORIGIN = 0x0,     LENGTH = 0x200
	RAM     (rwx): ORIGIN = 0x10000, LENGTH = 0x4000
}

SECTIONS
{
	.vectors : ALIGN(4) { *(.text.vectors) } >VECTORS
	.main    : ALIGN(4) { *(.text.main)    } >RAM
	.bss     :          { *(.bss)          } >RAM
}
