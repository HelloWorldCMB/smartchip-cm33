/* Linker script for Cortex-M33
 * Matches Keil scatter file Objects/cm33-freertos.sct:
 *   ROM (code) : 0x00000000, 64KB
 *   RAM (data) : 0x00010000, 64KB
 */

MEMORY
{
  FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 64K
  RAM   (rwx): ORIGIN = 0x00010000, LENGTH = 64K
}

ENTRY(Reset_Handler)

_estack = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS
{
  .text :
  {
    KEEP(*(.isr_vector))
    *(.text*)
    *(.rodata*)
    KEEP(*(.init))
    KEEP(*(.fini))
    . = ALIGN(4);
    _etext = .;
  } > FLASH

  .ARM.extab :
  {
    *(.ARM.extab* .gnu.linkonce.armextab.*)
  } > FLASH

  .ARM.exidx :
  {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } > FLASH

  _sidata = LOADADDR(.data);

  .data :
  {
    . = ALIGN(4);
    _sdata = .;
    *(.data*)
    . = ALIGN(4);
    _edata = .;
  } > RAM AT> FLASH

  .bss :
  {
    . = ALIGN(4);
    _sbss = .;
    __bss_start__ = _sbss;
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;
    __bss_end__ = _ebss;
  } > RAM

  ._user_heap_stack :
  {
    . = ALIGN(8);
    PROVIDE(end = .);
    PROVIDE(_end = .);
    . = . + 0xC00;   /* heap */
    . = . + 0x1000;  /* stack reserved in linker check */
    . = ALIGN(8);
  } > RAM

  /DISCARD/ :
  {
    *(.comment)
  }
}
