/**
 * GCC startup for ARMCM33 - IRQ table matches Keil startup_ARMCM33.s
 */
  .syntax unified
  .cpu cortex-m33
  .fpu fpv5-sp-d16
  .thumb

  .global g_pfnVectors
  .global __Vectors
  .global __VECTOR_TABLE
  .global Default_Handler
  .global Reset_Handler

  .word _sidata
  .word _sdata
  .word _edata
  .word _sbss
  .word _ebss

  .section .text.Reset_Handler
  .weak Reset_Handler
  .type Reset_Handler, %function
Reset_Handler:
  ldr  r0, =_estack
  msr  msp, r0

  /* MSPLIM for Cortex-M33 (non-secure may be RAZ/WI) */
  ldr  r0, =_sbss
  msr  msplim, r0

  /* Copy .data */
  ldr  r0, =_sdata
  ldr  r1, =_edata
  ldr  r2, =_sidata
  movs r3, #0
  b    2f
1:
  ldr  r4, [r2, r3]
  str  r4, [r0, r3]
  adds r3, r3, #4
2:
  adds r4, r0, r3
  cmp  r4, r1
  bcc  1b

  /* Zero .bss */
  ldr  r2, =_sbss
  ldr  r4, =_ebss
  movs r3, #0
  b    4f
3:
  str  r3, [r2]
  adds r2, r2, #4
4:
  cmp  r2, r4
  bcc  3b

  bl   SystemInit
  bl   main

  b    .

  .size Reset_Handler, .-Reset_Handler

  .section .text.Default_Handler,"ax",%progbits
Default_Handler:
  b    .
  .size Default_Handler, .-Default_Handler

/* Weak aliases for exception/IRQ handlers */
  .macro def_irq_handler handler_name
  .weak \handler_name
  .thumb_set \handler_name, Default_Handler
  .endm

  def_irq_handler NMI_Handler
  def_irq_handler HardFault_Handler
  def_irq_handler MemManage_Handler
  def_irq_handler BusFault_Handler
  def_irq_handler UsageFault_Handler
  def_irq_handler SecureFault_Handler
  def_irq_handler SVC_Handler
  def_irq_handler DebugMon_Handler
  def_irq_handler PendSV_Handler
  def_irq_handler SysTick_Handler

  def_irq_handler GPIO010_IRQHandler
  def_irq_handler Interrupt1_Handler
  def_irq_handler ftwdt011_interrupt
  def_irq_handler ftwdt011_3_interrupt
  def_irq_handler Interrupt4_Handler
  def_irq_handler Interrupt5_Handler
  def_irq_handler Interrupt6_Handler
  def_irq_handler Interrupt7_Handler
  def_irq_handler Interrupt8_Handler
  def_irq_handler Interrupt9_Handler
  def_irq_handler Interrupt10_Handler
  def_irq_handler Interrupt11_Handler
  def_irq_handler Interrupt12_Handler
  def_irq_handler PWMTMR_IRQHandler
  def_irq_handler PWMTMR1_IRQHandler
  def_irq_handler PWMTMR2_IRQHandler
  def_irq_handler PWMTMR3_IRQHandler
  def_irq_handler PWMTMR4_IRQHandler
  def_irq_handler PWMTMR_1_IRQHandler
  def_irq_handler PWMTMR_1_1_IRQHandler
  def_irq_handler PWMTMR_1_2_IRQHandler
  def_irq_handler PWMTMR_1_3_IRQHandler
  def_irq_handler PWMTMR_1_4_IRQHandler
  def_irq_handler PWMTMR_2_IRQHandler
  def_irq_handler PWMTMR_2_1_IRQHandler
  def_irq_handler PWMTMR_2_2_IRQHandler
  def_irq_handler PWMTMR_2_3_IRQHandler
  def_irq_handler PWMTMR_2_4_IRQHandler
  def_irq_handler PWMTMR_3_IRQHandler
  def_irq_handler PWMTMR_3_1_IRQHandler
  def_irq_handler PWMTMR_3_2_IRQHandler
  def_irq_handler PWMTMR_3_3_IRQHandler
  def_irq_handler PWMTMR_3_4_IRQHandler
  def_irq_handler IIC0_IRQHandler
  def_irq_handler Interrupt34_Handler
  def_irq_handler Interrupt35_Handler
  def_irq_handler Interrupt36_Handler
  def_irq_handler Interrupt37_Handler
  def_irq_handler UART0_IRQHandler
  def_irq_handler Interrupt39_Handler
  def_irq_handler Interrupt40_Handler
  def_irq_handler Interrupt41_Handler
  def_irq_handler Interrupt42_Handler
  def_irq_handler Interrupt43_Handler
  def_irq_handler Interrupt44_Handler
  def_irq_handler Interrupt45_Handler
  def_irq_handler Interrupt46_Handler
  def_irq_handler Interrupt47_Handler
  def_irq_handler Interrupt48_Handler
  def_irq_handler Interrupt49_Handler
  def_irq_handler Interrupt50_Handler
  def_irq_handler Interrupt51_Handler
  def_irq_handler Interrupt52_Handler
  def_irq_handler Interrupt53_Handler
  def_irq_handler Interrupt54_Handler
  def_irq_handler Interrupt55_Handler
  def_irq_handler Interrupt56_Handler
  def_irq_handler Interrupt57_Handler
  def_irq_handler CAN010_0_interrupt
  def_irq_handler CAN010_1_interrupt
  def_irq_handler ftkbc010_interrupt
  def_irq_handler Interrupt61_Handler
  def_irq_handler Interrupt62_Handler
  def_irq_handler Interrupt63_Handler
  def_irq_handler Interrupt64_Handler
  def_irq_handler Interrupt65_Handler
  def_irq_handler AHB_DMA_IRQHandler
  def_irq_handler AHB_DMA_TC_IRQHandler
  def_irq_handler Interrupt68_Handler
  def_irq_handler Semaphore_Handler
  def_irq_handler CTI_Handler

  .section .isr_vector,"a",%progbits
  .type g_pfnVectors, %object
  .size g_pfnVectors, .-g_pfnVectors

g_pfnVectors:
__Vectors:
__VECTOR_TABLE:
  .word _estack
  .word Reset_Handler
  .word NMI_Handler
  .word HardFault_Handler
  .word MemManage_Handler
  .word BusFault_Handler
  .word UsageFault_Handler
  .word SecureFault_Handler
  .word 0
  .word 0
  .word 0
  .word SVC_Handler
  .word DebugMon_Handler
  .word 0
  .word PendSV_Handler
  .word SysTick_Handler
  /* External interrupts 0..70 */
  .word GPIO010_IRQHandler
  .word Interrupt1_Handler
  .word ftwdt011_interrupt
  .word ftwdt011_3_interrupt
  .word Interrupt4_Handler
  .word Interrupt5_Handler
  .word Interrupt6_Handler
  .word Interrupt7_Handler
  .word Interrupt8_Handler
  .word Interrupt9_Handler
  .word Interrupt10_Handler
  .word Interrupt11_Handler
  .word Interrupt12_Handler
  .word PWMTMR_IRQHandler
  .word PWMTMR1_IRQHandler
  .word PWMTMR2_IRQHandler
  .word PWMTMR3_IRQHandler
  .word PWMTMR4_IRQHandler
  .word PWMTMR_1_IRQHandler
  .word PWMTMR_1_1_IRQHandler
  .word PWMTMR_1_2_IRQHandler
  .word PWMTMR_1_3_IRQHandler
  .word PWMTMR_1_4_IRQHandler
  .word PWMTMR_2_IRQHandler
  .word PWMTMR_2_1_IRQHandler
  .word PWMTMR_2_2_IRQHandler
  .word PWMTMR_2_3_IRQHandler
  .word PWMTMR_2_4_IRQHandler
  .word PWMTMR_3_IRQHandler
  .word PWMTMR_3_1_IRQHandler
  .word PWMTMR_3_2_IRQHandler
  .word PWMTMR_3_3_IRQHandler
  .word PWMTMR_3_4_IRQHandler
  .word IIC0_IRQHandler
  .word Interrupt34_Handler
  .word Interrupt35_Handler
  .word Interrupt36_Handler
  .word Interrupt37_Handler
  .word UART0_IRQHandler
  .word Interrupt39_Handler
  .word Interrupt40_Handler
  .word Interrupt41_Handler
  .word Interrupt42_Handler
  .word Interrupt43_Handler
  .word Interrupt44_Handler
  .word Interrupt45_Handler
  .word Interrupt46_Handler
  .word Interrupt47_Handler
  .word Interrupt48_Handler
  .word Interrupt49_Handler
  .word Interrupt50_Handler
  .word Interrupt51_Handler
  .word Interrupt52_Handler
  .word Interrupt53_Handler
  .word Interrupt54_Handler
  .word Interrupt55_Handler
  .word Interrupt56_Handler
  .word Interrupt57_Handler
  .word CAN010_0_interrupt
  .word CAN010_1_interrupt
  .word ftkbc010_interrupt
  .word Interrupt61_Handler
  .word Interrupt62_Handler
  .word Interrupt63_Handler
  .word Interrupt64_Handler
  .word Interrupt65_Handler
  .word AHB_DMA_IRQHandler
  .word AHB_DMA_TC_IRQHandler
  .word Interrupt68_Handler
  .word Semaphore_Handler
  .word CTI_Handler
