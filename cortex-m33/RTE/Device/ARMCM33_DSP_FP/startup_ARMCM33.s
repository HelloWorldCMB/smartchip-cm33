;/**************************************************************************//**
; * @file     startup_ARMCM33.s
; * @brief    CMSIS Core Device Startup File for
; *           ARMCM33 Device
; * @version  V5.4.0
; * @date     12. December 2018
; ******************************************************************************/
;/*
; * Copyright (c) 2009-2018 Arm Limited. All rights reserved.
; *
; * SPDX-License-Identifier: Apache-2.0
; *
; * Licensed under the Apache License, Version 2.0 (the License); you may
; * not use this file except in compliance with the License.
; * You may obtain a copy of the License at
; *
; * www.apache.org/licenses/LICENSE-2.0
; *
; * Unless required by applicable law or agreed to in writing, software
; * distributed under the License is distributed on an AS IS BASIS, WITHOUT
; * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
; * See the License for the specific language governing permissions and
; * limitations under the License.
; */

;//-------- <<< Use Configuration Wizard in Context Menu >>> ------------------


;<h> Stack Configuration
;  <o> Stack Size (in Bytes) <0x0-0xFFFFFFFF:8>
;</h>

Stack_Size      EQU      0x00001000

                AREA     STACK, NOINIT, READWRITE, ALIGN=3
__stack_limit
Stack_Mem       SPACE    Stack_Size
__initial_sp


;<h> Heap Configuration
;  <o> Heap Size (in Bytes) <0x0-0xFFFFFFFF:8>
;</h>

Heap_Size       EQU      0x00000C00

                IF       Heap_Size != 0                      ; Heap is provided
                AREA     HEAP, NOINIT, READWRITE, ALIGN=3
__heap_base
Heap_Mem        SPACE    Heap_Size
__heap_limit
                ENDIF


                PRESERVE8
                THUMB


; Vector Table Mapped to Address 0 at Reset

                AREA     RESET, DATA, READONLY
                EXPORT   __Vectors
                EXPORT   __Vectors_End
                EXPORT   __Vectors_Size

__Vectors       DCD      __initial_sp                        ;     Top of Stack
                DCD      Reset_Handler                       ;     Reset Handler
                DCD      NMI_Handler                         ; -14 NMI Handler
                DCD      HardFault_Handler                   ; -13 Hard Fault Handler
                DCD      MemManage_Handler                   ; -12 MPU Fault Handler
                DCD      BusFault_Handler                    ; -11 Bus Fault Handler
                DCD      UsageFault_Handler                  ; -10 Usage Fault Handler
                DCD      SecureFault_Handler                 ;  -9 Secure Fault Handler
                DCD      0                                   ;     Reserved
                DCD      0                                   ;     Reserved
                DCD      0                                   ;     Reserved
                DCD      SVC_Handler                         ;  -5 SVCall Handler
                DCD      DebugMon_Handler                    ;  -4 Debug Monitor Handler
                DCD      0                                   ;     Reserved
                DCD      PendSV_Handler                      ;  -2 PendSV Handler
                DCD      SysTick_Handler                     ;  -1 SysTick Handler

                ; Interrupts
                DCD      GPIO010_IRQHandler                  ;   0 Interrupt 0
                DCD      Interrupt1_Handler                  ;   1 Interrupt 1
                DCD      ftwdt011_interrupt                  ;   2 Interrupt 2
                DCD      ftwdt011_3_interrupt                ;   3 Interrupt 3
                DCD      Interrupt4_Handler                  ;   4 Interrupt 4
                DCD      Interrupt5_Handler                  ;   5 Interrupt 5
                DCD      Interrupt6_Handler                  ;   6 Interrupt 6
                DCD      Interrupt7_Handler                  ;   7 Interrupt 7
                DCD      Interrupt8_Handler                  ;   8 Interrupt 8
                DCD      Interrupt9_Handler                  ;   9 Interrupt 9
				DCD		 Interrupt10_Handler  ;10
                DCD      Interrupt11_Handler  ;11
                DCD      Interrupt12_Handler  ;12
                DCD      PWMTMR_IRQHandler ;13			
                DCD      PWMTMR1_IRQHandler ; //14
                DCD      PWMTMR2_IRQHandler ;//15
                DCD      PWMTMR3_IRQHandler ;/16
                DCD      PWMTMR4_IRQHandler ;/17					
                DCD      PWMTMR_1_IRQHandler ;18			
                DCD      PWMTMR_1_1_IRQHandler ; //19
                DCD      PWMTMR_1_2_IRQHandler ;//20
                DCD      PWMTMR_1_3_IRQHandler ;/21
                DCD      PWMTMR_1_4_IRQHandler ;/22					
                DCD      PWMTMR_2_IRQHandler ;23		
                DCD      PWMTMR_2_1_IRQHandler ; //24
                DCD      PWMTMR_2_2_IRQHandler ;//25
                DCD      PWMTMR_2_3_IRQHandler ;/26
                DCD      PWMTMR_2_4_IRQHandler ;/27					
				DCD      PWMTMR_3_IRQHandler ;28
                DCD      PWMTMR_3_1_IRQHandler ; //29
                DCD      PWMTMR_3_2_IRQHandler ;//30
                DCD      PWMTMR_3_3_IRQHandler ;/31
                DCD      PWMTMR_3_4_IRQHandler ;/32					                
				DCD      IIC0_IRQHandler                 ; 33					
                DCD      Interrupt34_Handler  ;/34
                DCD      Interrupt35_Handler  ;/35
                DCD      Interrupt36_Handler  ;/36
                DCD      Interrupt37_Handler  ;/37
                DCD      UART0_IRQHandler  ;/38
                DCD      Interrupt39_Handler  ;/39					
                DCD      Interrupt40_Handler  ;/40
                DCD      Interrupt41_Handler  ;/41					
                DCD      Interrupt42_Handler  ;/42					
                DCD      Interrupt43_Handler  ;/43					
                DCD      Interrupt44_Handler  ;/44					
                DCD      Interrupt45_Handler  ;/45										
                DCD      Interrupt46_Handler  ;/46					
                DCD      Interrupt47_Handler  ;/47					
                DCD      Interrupt48_Handler  ;/48					
                DCD      Interrupt49_Handler  ;/49					
                DCD      Interrupt50_Handler  ;/50					
                DCD      Interrupt51_Handler  ;/51					
                DCD      Interrupt52_Handler  ;/52										
                DCD      Interrupt53_Handler  ;/53					
                DCD      Interrupt54_Handler  ;/54					
                DCD      Interrupt55_Handler  ;/55					
                DCD      Interrupt56_Handler  ;/56					
                DCD      Interrupt57_Handler  ;/57					
                DCD      CAN010_0_interrupt  ;/58					
                DCD      CAN010_1_interrupt  ;/59										
                DCD      ftkbc010_interrupt  ;/60					
                DCD      Interrupt61_Handler  ;/61					
                DCD      Interrupt62_Handler  ;/62					
                DCD      Interrupt63_Handler  ;/63					
                DCD      Interrupt64_Handler  ;/64					
                DCD      Interrupt65_Handler  ;/65					
                DCD      AHB_DMA_IRQHandler  ;/66										
                DCD      AHB_DMA_TC_IRQHandler  ;/67					
                DCD      Interrupt68_Handler  ;/68					
                DCD      Semaphore_Handler  ;/69					
                DCD      CTI_Handler  ;/70					
                SPACE    (409 * 4)                           ; Interrupts 10 .. 480 are left out
__Vectors_End
__Vectors_Size  EQU      __Vectors_End - __Vectors


                AREA     |.text|, CODE, READONLY

; Reset Handler

Reset_Handler   PROC
                EXPORT   Reset_Handler             [WEAK]
                IMPORT   SystemInit
                IMPORT   __main

                 LDR      R0, =__stack_limit
                MSR      MSPLIM, R0                          ; Non-secure version of MSPLIM is RAZ/WI

                LDR      R0, =SystemInit
                BLX      R0
                LDR      R0, =__main
                BX       R0
                ENDP


; Macro to define default exception/interrupt handlers.
; Default handler are weak symbols with an endless loop.
; They can be overwritten by real handlers.
                MACRO
                Set_Default_Handler  $Handler_Name
$Handler_Name   PROC
                EXPORT   $Handler_Name             [WEAK]
                B        .
                ENDP
                MEND


; Default exception/interrupt handler

                Set_Default_Handler  NMI_Handler
                Set_Default_Handler  HardFault_Handler
                Set_Default_Handler  MemManage_Handler
                Set_Default_Handler  BusFault_Handler
                Set_Default_Handler  UsageFault_Handler
                Set_Default_Handler  SecureFault_Handler
                Set_Default_Handler  SVC_Handler
                Set_Default_Handler  DebugMon_Handler
                Set_Default_Handler  PendSV_Handler
                Set_Default_Handler  SysTick_Handler

                Set_Default_Handler  GPIO010_IRQHandler
                Set_Default_Handler  Interrupt1_Handler
                Set_Default_Handler  ftwdt011_interrupt
                Set_Default_Handler  ftwdt011_3_interrupt
                Set_Default_Handler  Interrupt4_Handler
                Set_Default_Handler  Interrupt5_Handler
                Set_Default_Handler  Interrupt6_Handler
                Set_Default_Handler  Interrupt7_Handler
                Set_Default_Handler  Interrupt8_Handler
                Set_Default_Handler  Interrupt9_Handler
                Set_Default_Handler  Interrupt10_Handler  ;/10
                Set_Default_Handler  Interrupt11_Handler  ;/11
                Set_Default_Handler  Interrupt12_Handler  ;/12
                Set_Default_Handler  PWMTMR_IRQHandler  ;/13
                Set_Default_Handler  PWMTMR1_IRQHandler  ;/14
                Set_Default_Handler  PWMTMR2_IRQHandler  ;/15
                Set_Default_Handler  PWMTMR3_IRQHandler  ;/16
                Set_Default_Handler  PWMTMR4_IRQHandler  ;/17
                Set_Default_Handler  PWMTMR_1_IRQHandler  ;/18
                Set_Default_Handler  PWMTMR_1_1_IRQHandler  ;/19
                Set_Default_Handler  PWMTMR_1_2_IRQHandler  ;/20
                Set_Default_Handler  PWMTMR_1_3_IRQHandler  ;/21
                Set_Default_Handler  PWMTMR_1_4_IRQHandler  ;/22
                Set_Default_Handler  PWMTMR_2_IRQHandler ;/23
                Set_Default_Handler  PWMTMR_2_1_IRQHandler  ;/24
                Set_Default_Handler  PWMTMR_2_2_IRQHandler  ;/25
                Set_Default_Handler  PWMTMR_2_3_IRQHandler  ;/26
                Set_Default_Handler  PWMTMR_2_4_IRQHandler  ;/27
                Set_Default_Handler  PWMTMR_3_IRQHandler ;/28
                Set_Default_Handler  PWMTMR_3_1_IRQHandler  ;/29
                Set_Default_Handler  PWMTMR_3_2_IRQHandler  ;/30
                Set_Default_Handler  PWMTMR_3_3_IRQHandler  ;/31
                Set_Default_Handler  PWMTMR_3_4_IRQHandler  ;/32
                Set_Default_Handler  IIC0_IRQHandler  ;/33
                Set_Default_Handler  Interrupt34_Handler  ;/34
                Set_Default_Handler  Interrupt35_Handler  ;/35
                Set_Default_Handler  Interrupt36_Handler  ;/36
                Set_Default_Handler  Interrupt37_Handler  ;/37
                Set_Default_Handler  UART0_IRQHandler  ;/38
                Set_Default_Handler  Interrupt39_Handler  ;/39			
                Set_Default_Handler  Interrupt40_Handler  ;/40
                Set_Default_Handler  Interrupt41_Handler  ;/41
                Set_Default_Handler  Interrupt42_Handler  ;/42
                Set_Default_Handler  Interrupt43_Handler  ;/43
                Set_Default_Handler  Interrupt44_Handler  ;/44
                Set_Default_Handler  Interrupt45_Handler  ;/45
                Set_Default_Handler  Interrupt46_Handler  ;/46
                Set_Default_Handler  Interrupt47_Handler  ;/47
                Set_Default_Handler  Interrupt48_Handler  ;/48
                Set_Default_Handler  Interrupt49_Handler  ;/49
                Set_Default_Handler  Interrupt50_Handler  ;/50
                Set_Default_Handler  Interrupt51_Handler  ;/51
                Set_Default_Handler  Interrupt52_Handler  ;/52
                Set_Default_Handler  Interrupt53_Handler  ;/53
                Set_Default_Handler  Interrupt54_Handler  ;/54
                Set_Default_Handler  Interrupt55_Handler  ;/55
                Set_Default_Handler  Interrupt56_Handler  ;/56
                Set_Default_Handler  Interrupt57_Handler  ;/57
                Set_Default_Handler  CAN010_0_interrupt  ;/58
                Set_Default_Handler  CAN010_1_interrupt  ;/59
                Set_Default_Handler  ftkbc010_interrupt  ;/60
                Set_Default_Handler  Interrupt61_Handler  ;/61
                Set_Default_Handler  Interrupt62_Handler  ;/62
                Set_Default_Handler  Interrupt63_Handler  ;/63
                Set_Default_Handler  Interrupt64_Handler  ;/64
                Set_Default_Handler  Interrupt65_Handler  ;/65
                Set_Default_Handler  AHB_DMA_IRQHandler  ;/66
                Set_Default_Handler  AHB_DMA_TC_IRQHandler  ;/67
                Set_Default_Handler  Interrupt68_Handler  ;/68
                Set_Default_Handler  Semaphore_Handler  ;/69
                Set_Default_Handler  CTI_Handler  ;/70				
                ALIGN


; User setup Stack & Heap

                IF       :LNOT::DEF:__MICROLIB
                IMPORT   __use_two_region_memory
                ENDIF

                EXPORT   __stack_limit
                EXPORT   __initial_sp
                IF       Heap_Size != 0                      ; Heap is provided
                EXPORT   __heap_base
                EXPORT   __heap_limit
                ENDIF

                END
