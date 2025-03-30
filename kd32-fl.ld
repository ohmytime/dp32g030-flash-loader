/* Entry Point */
ENTRY(Reset_Handler)

/* Highest address of the user mode stack */
_estack = ORIGIN(RAM) + LENGTH(RAM); /* end of "RAM" Ram type memory */

_Min_Heap_Size = 0x200; /* required amount of heap */
_Min_Stack_Size = 0x400; /* required amount of stack */

/* Memories definition */
MEMORY
{
  RAM    (xrw)    : ORIGIN = 0x20000000,   LENGTH = 24K
  FLASH    (rx)    : ORIGIN = 0x8000000,   LENGTH = 128K
}

/*
	RAM    (xrw)    : ORIGIN = 0x20000000,   LENGTH = 21856
    FLASH    (rx)    : ORIGIN = 0x8002000,   LENGTH = 120K

 */
/* Sections */
SECTIONS
{
	/* 向量表段 */
  .isr_vector : {
    . = ALIGN(512);  /* 512 字节对齐 */
    KEEP(*(.isr_vector))
    . = ALIGN(4);
  } >FLASH

	/* 代码段 */
  .text : {
    *(.text.Reset_Handler)
    *(.text*)
    *(.rodata*)
  } >FLASH

	PrgCode :
	{
		. = ALIGN(4);
		*(.vectors)       /* 添加向量表段 */
		*(.PrgCode)
		*(.PrgCode*)
		*(.text)
		*(.text*)
		. = ALIGN(4);
	} >FLASH

	PrgData :
	{
		. = ALIGN(4);
		*(.PrgData)
		*(.PrgData*)
		*(.data)
		*(.data*)
		*(.rodata)
		*(.rodata*)
		_sbss = .;
		*(.bss)
		*(.bss*)
		. = ALIGN(4);
	} >RAM

/* 只读数据存放在 Flash */
  .rodata : {
    *(.rodata*)
  } >FLASH


	/* 数据段：VMA在RAM，LMA在Flash */
  .data : {
    _sdata = .;
    *(.data*)
    _edata = .;
  } >RAM AT>FLASH

  /* BSS 段 */
  .bss : {
    _sbss = .;
    *(.bss*)
    _ebss = .;
  } >RAM

  /* Flash算法关键段 */
  DevDscr : {
    *(.DevDscr*)
  } >FLASH
}
