ENTRY(Reset_Handler)

MEMORY
{
  FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 512K
  SRAM  (rw) : ORIGIN = 0x20000000, LENGTH = 112K
  RAM2  (rw) : ORIGIN = 0x2001C000, LENGTH = 16K
  CCMRAM(rw) : ORIGIN = 0x10000000, LENGTH = 64K
}

PHDRS
{
  text PT_LOAD FLAGS(5);
  data PT_LOAD FLAGS(6);
}

_estack = ORIGIN(SRAM) + LENGTH(SRAM);

SECTIONS
{
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))
    . = ALIGN(4);
  } > FLASH :text

  .text :
  {
    . = ALIGN(4);
    _stext = .;
    *(.text)
    *(.text*)
    *(.rodata)
    *(.rodata*)
    *(.glue_7)
    *(.glue_7t)
    KEEP(*(.init))
    KEEP(*(.fini))
    . = ALIGN(4);
    _etext = .;
  } > FLASH :text

  .ARM.extab :
  {
    . = ALIGN(4);
    *(.ARM.extab)
    *(.gnu.linkonce.armextab.*)
    . = ALIGN(4);
  } > FLASH :text

  .ARM.exidx :
  {
    . = ALIGN(4);
    PROVIDE(__exidx_start = .);
    *(.ARM.exidx*)
    . = ALIGN(4);
    PROVIDE(__exidx_end = .);
  } > FLASH :text

  .preinit_array :
  {
    PROVIDE_HIDDEN(__preinit_array_start = .);
    KEEP(*(.preinit_array*))
    PROVIDE_HIDDEN(__preinit_array_end = .);
  } > FLASH :text

  .init_array :
  {
    PROVIDE_HIDDEN(__init_array_start = .);
    KEEP(*(SORT(.init_array.*)))
    KEEP(*(.init_array*))
    PROVIDE_HIDDEN(__init_array_end = .);
  } > FLASH :text

  .fini_array :
  {
    PROVIDE_HIDDEN(__fini_array_start = .);
    KEEP(*(SORT(.fini_array.*)))
    KEEP(*(.fini_array*))
    PROVIDE_HIDDEN(__fini_array_end = .);
  } > FLASH :text

  _sidata = LOADADDR(.data);
  .data :
  {
    . = ALIGN(4);
    _sdata = .;
    PROVIDE(__data_start__ = _sdata);
    *(.data)
    *(.data*)
    . = ALIGN(4);
    _edata = .;
    PROVIDE(__data_end__ = _edata);
  } > SRAM AT > FLASH :data

  .bss :
  {
    . = ALIGN(4);
    _sbss = .;
    PROVIDE(__bss_start__ = _sbss);
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;
    PROVIDE(__bss_end__ = _ebss);
  } > SRAM :NONE

  .heap (NOLOAD) :
  {
    . = ALIGN(8);
    PROVIDE(__heap_start__ = .);
    . = . + 0x400;
    . = ALIGN(8);
    PROVIDE(__heap_end__ = .);
  } > SRAM :NONE

  .reserved_for_stack (NOLOAD) :
  {
    . = ALIGN(8);
    PROVIDE(__reserved_for_stack_start__ = .);
    . = . + 0x1000;
    . = ALIGN(8);
    PROVIDE(__reserved_for_stack_end__ = .);
  } > SRAM :NONE

  PROVIDE(end = .);
  PROVIDE(_end = .);

  /DISCARD/ :
  {
    libnosys.a ( * )
  }
}
