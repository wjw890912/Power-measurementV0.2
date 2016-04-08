;/*
;********************************************************************************************************
;                                    EXCEPTION VECTORS & STARTUP CODE
;
; File      : cstartup.s
; For       : ARMv7M Cortex-M4
; Mode      : Thumb2
; Toolchain : RealView Development Suite
;             RealView Microcontroller Development Kit (MDK)
;             ARM Developer Suite (ADS)
;             Keil uVision
;********************************************************************************************************
;*/

;/*
;********************************************************************************************************
;*                           <<< Use Configuration Wizard in Context Menu >>>
;*
;* Note(s) : (1) The µVision4 Configuration Wizard enables menu driven configuration of assembler, 
;*               C/C++, or debugger initialization files. The Configuration Wizard uses control items 
;*               that are embedded into the comments of the configuration file.
;*
;********************************************************************************************************
;*/

;/*
;********************************************************************************************************
;*                                              STACK DEFINITIONS
;*
;* Configuration Wizard Menu:
;* // <h> Stack Configuration
;* //   <o> Stack Size (in Bytes) <0x0-0xFFFFFFFF:8>
;* // </h>;
;*********************************************************************************************************
;*/

STACK_SIZE      EQU     0x00000200
                AREA    STACK, NOINIT, READWRITE, ALIGN=3
Stack_Mem       SPACE   STACK_SIZE
__initial_sp


;/*
;********************************************************************************************************
;*                                              STACK DEFINITIONS
; <h> Heap Configuration
;   <o>  Heap Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>
;*********************************************************************************************************
;*/

HEAP_SIZE       EQU     0x00000000
                AREA    HEAP, NOINIT, READWRITE, ALIGN=3
__heap_base
Heap_Mem        SPACE   HEAP_SIZE
__heap_limit


                PRESERVE8
                THUMB


                                                      ; Vector Table Mapped to Address 0 at Reset

                AREA    RESET, DATA, READONLY
                EXPORT  __Vectors
                IMPORT  BSP_IntHandlerBOD
                IMPORT  BSP_IntHandlerIRC 
                IMPORT  BSP_IntHandlerPWRWU
                IMPORT  BSP_IntHandlerSRAMF
                IMPORT  BSP_IntHandlerCLKF
                IMPORT  BSP_IntHandlerRTC
                IMPORT  BSP_IntHandlerTAMPER
                IMPORT  BSP_IntHandlerEINT0
                IMPORT  BSP_IntHandlerEINT1
                IMPORT  BSP_IntHandlerEINT2
                IMPORT  BSP_IntHandlerEINT3
                IMPORT  BSP_IntHandlerEINT4
                IMPORT  BSP_IntHandlerEINT5
                IMPORT  BSP_IntHandlerEINT6
                IMPORT  BSP_IntHandlerEINT7
                IMPORT  BSP_IntHandlerGPIOA
                IMPORT  BSP_IntHandlerGPIOB
                IMPORT  BSP_IntHandlerGPIOC
                IMPORT  BSP_IntHandlerGPIOD
                IMPORT  BSP_IntHandlerGPIOE
                IMPORT  BSP_IntHandlerGPIOF
                IMPORT  BSP_IntHandlerGPIOG
                IMPORT  BSP_IntHandlerGPIOH
                IMPORT  BSP_IntHandlerGPIOI
                IMPORT  BSP_IntHandlerGPIOJ
                IMPORT  BSP_IntHandlerTIMER0
                IMPORT  BSP_IntHandlerTIMER1
                IMPORT  BSP_IntHandlerTIMER2
                IMPORT  BSP_IntHandlerTIMER3
                IMPORT  BSP_IntHandlerPDMA
                IMPORT  BSP_IntHandlerADC
                IMPORT  BSP_IntHandlerWDT
                IMPORT  BSP_IntHandlerWWDT
                IMPORT  BSP_IntHandlerEADC0
                IMPORT  BSP_IntHandlerEADC1
                IMPORT  BSP_IntHandlerEADC2
                IMPORT  BSP_IntHandlerEADC3
                IMPORT  BSP_IntHandlerACMP0
                IMPORT  BSP_IntHandlerOPA0
                IMPORT  BSP_IntHandlerOPA1
                IMPORT  BSP_IntHandlerICAP0
                IMPORT  BSP_IntHandlerICAP1
                IMPORT  BSP_IntHandlerPWMA0
                IMPORT  BSP_IntHandlerPWMA1
                IMPORT  BSP_IntHandlerPWMA2
                IMPORT  BSP_IntHandlerPWMA3
                IMPORT  BSP_IntHandlerPWMA4
                IMPORT  BSP_IntHandlerPWMA5
                IMPORT  BSP_IntHandlerPWMABRK
                IMPORT  BSP_IntHandlerQEI0
                IMPORT  BSP_IntHandlerPWMB0
                IMPORT  BSP_IntHandlerPWMB1
                IMPORT  BSP_IntHandlerPWMB2
                IMPORT  BSP_IntHandlerPWMB3
                IMPORT  BSP_IntHandlerPWMB4
                IMPORT  BSP_IntHandlerPWMB5
                IMPORT  BSP_IntHandlerPWMBBRK
                IMPORT  BSP_IntHandlerQEI1
                IMPORT  BSP_IntHandlerEPWMA
                IMPORT  BSP_IntHandlerEPWMABRK
                IMPORT  BSP_IntHandlerEPWMB
                IMPORT  BSP_IntHandlerEPWMBBRK
                IMPORT  BSP_IntHandlerUSBD
                IMPORT  BSP_IntHandlerUSBH
                IMPORT  BSP_IntHandlerUSB_OTG
                IMPORT  BSP_IntHandlerEMAC_TX
                IMPORT  BSP_IntHandlerEMAC_RX
                IMPORT  BSP_IntHandlerSPI0
                IMPORT  BSP_IntHandlerSPI1
                IMPORT  BSP_IntHandlerSPI2
                IMPORT  BSP_IntHandlerSPI3
                IMPORT  BSP_IntHandlerUART0
                IMPORT  BSP_IntHandlerUART1
                IMPORT  BSP_IntHandlerUART2
                IMPORT  BSP_IntHandlerUART3
                IMPORT  BSP_IntHandlerUART4
                IMPORT  BSP_IntHandlerUART5
                IMPORT  BSP_IntHandlerI2C0
                IMPORT  BSP_IntHandlerI2C1
                IMPORT  BSP_IntHandlerI2C2
                IMPORT  BSP_IntHandlerI2C3
                IMPORT  BSP_IntHandlerI2C4
                IMPORT  BSP_IntHandlerSC0
                IMPORT  BSP_IntHandlerSC1
                IMPORT  BSP_IntHandlerSC2
                IMPORT  BSP_IntHandlerSC3
                IMPORT  BSP_IntHandlerSC4
                IMPORT  BSP_IntHandlerSC5
                IMPORT  BSP_IntHandlerCAN0
                IMPORT  BSP_IntHandlerCAN1
                IMPORT  BSP_IntHandlerI2S0
                IMPORT  BSP_IntHandlerI2S1
                IMPORT  BSP_IntHandlerSD
                IMPORT  BSP_IntHandlerPS2D
                IMPORT  BSP_IntHandlerCRYPTO
                IMPORT  BSP_IntHandlerCRC
                IMPORT  BSP_IntHandlerDefault

                IMPORT  OS_CPU_PendSVHandler
                IMPORT  OS_CPU_SysTickHandler 

__Vectors       DCD     __initial_sp                      ; Top of Stack
                DCD     Reset_Handler                     ; Reset Handler
                DCD     App_NMI_ISR                       ; NMI Handler
                DCD     App_Fault_ISR                     ; Hard Fault Handler
                DCD     App_MemFault_ISR                  ; MPU Fault Handler
                DCD     App_BusFault_ISR                  ; Bus Fault Handler
                DCD     App_UsageFault_ISR                ; Usage Fault Handler
                DCD     App_Spurious_ISR                  ; Reserved
                DCD     App_Spurious_ISR                  ; Reserved
                DCD     App_Spurious_ISR                  ; Reserved
                DCD     App_Spurious_ISR                  ; Reserved
                DCD     App_Spurious_ISR                  ; SVCall Handler
                DCD     App_Spurious_ISR                  ; Debug Monitor Handler
                DCD     App_Spurious_ISR                  ; Reserved
                DCD     OS_CPU_PendSVHandler              ; PendSV Handler
                DCD     OS_CPU_SysTickHandler             ; SysTick Handler

                                                          ;  External Interrupts
                DCD     BSP_IntHandlerBOD                 ;   0: Brown Out detection
                DCD     BSP_IntHandlerIRC                 ;   1: Internal RC
                DCD     BSP_IntHandlerPWRWU               ;   2: Power Down Wake Up
                DCD     BSP_IntHandlerSRAMF               ;   3: SRAM parity error
                DCD     BSP_IntHandlerCLKF                ;   4: Clock failed
                DCD     BSP_IntHandlerDefault             ;   5: Reserved
                DCD     BSP_IntHandlerRTC                 ;   6: Real Time Clock
                DCD     BSP_IntHandlerTAMPER              ;   7: Tamper detection
                DCD     BSP_IntHandlerEINT0               ;   8: External Input 0
                DCD     BSP_IntHandlerEINT1               ;   9: External Input 1
                DCD     BSP_IntHandlerEINT2               ;  10: External Input 2
                DCD     BSP_IntHandlerEINT3               ;  11: External Input 3
                DCD     BSP_IntHandlerEINT4               ;  12: External Input 4
                DCD     BSP_IntHandlerEINT5               ;  13: External Input 5
                DCD     BSP_IntHandlerEINT6               ;  14: External Input 6
                DCD     BSP_IntHandlerEINT7               ;  15: External Input 7
                DCD     BSP_IntHandlerGPIOA               ;  16: GPIO Port A
                DCD     BSP_IntHandlerGPIOB               ;  17: GPIO Port B
                DCD     BSP_IntHandlerGPIOC               ;  18: GPIO Port C
                DCD     BSP_IntHandlerGPIOD               ;  19: GPIO Port D
                DCD     BSP_IntHandlerGPIOE               ;  20: GPIO Port E
                DCD     BSP_IntHandlerGPIOF               ;  21: GPIO Port F
                DCD     BSP_IntHandlerGPIOG               ;  22: GPIO Port G
                DCD     BSP_IntHandlerGPIOH               ;  23: GPIO Port H
                DCD     BSP_IntHandlerGPIOI               ;  24: GPIO Port I
                DCD     BSP_IntHandlerGPIOJ               ;  25: GPIO Port J
                DCD     BSP_IntHandlerDefault             ;  26: Reserved
                DCD     BSP_IntHandlerDefault             ;  27: Reserved
                DCD     BSP_IntHandlerDefault             ;  28: Reserved
                DCD     BSP_IntHandlerDefault             ;  29: Reserved
                DCD     BSP_IntHandlerDefault             ;  30: Reserved
                DCD     BSP_IntHandlerDefault             ;  31: Reserved
                DCD     BSP_IntHandlerTIMER0              ;  32: Timer 0
                DCD     BSP_IntHandlerTIMER1              ;  33: Timer 1
                DCD     BSP_IntHandlerTIMER2              ;  34: Timer 2
                DCD     BSP_IntHandlerTIMER3              ;  35: Timer 3
                DCD     BSP_IntHandlerDefault             ;  36: Reserved
                DCD     BSP_IntHandlerDefault             ;  37: Reserved
                DCD     BSP_IntHandlerDefault             ;  38: Reserved
                DCD     BSP_IntHandlerDefault             ;  39: Reserved
                DCD     BSP_IntHandlerPDMA                ;  40: Peripheral DMA
                DCD     BSP_IntHandlerDefault             ;  41: Reserved
                DCD     BSP_IntHandlerADC                 ;  42: ADC
                DCD     BSP_IntHandlerDefault             ;  43: Reserved
                DCD     BSP_IntHandlerDefault             ;  44: Reserved
                DCD     BSP_IntHandlerDefault             ;  45: Reserved
                DCD     BSP_IntHandlerWDT                 ;  46: Watch Dog Timer
                DCD     BSP_IntHandlerWWDT                ;  47: Window Watch Dog Timer
                DCD     BSP_IntHandlerEADC0               ;  48: EADC0
                DCD     BSP_IntHandlerEADC1               ;  49: EADC1
                DCD     BSP_IntHandlerEADC2               ;  50: EADC2
                DCD     BSP_IntHandlerEADC3               ;  51: EADC3
                DCD     BSP_IntHandlerDefault             ;  52: Reserved
                DCD     BSP_IntHandlerDefault             ;  53: Reserved
                DCD     BSP_IntHandlerDefault             ;  54: Reserved
                DCD     BSP_IntHandlerDefault             ;  55: Reserved
                DCD     BSP_IntHandlerACMP0               ;  56: Analog Comparator
                DCD     BSP_IntHandlerDefault             ;  57: Reserved
                DCD     BSP_IntHandlerDefault             ;  58: Reserved
                DCD     BSP_IntHandlerDefault             ;  59: Reserved
                DCD     BSP_IntHandlerOPA0                ;  60: OPA0
                DCD     BSP_IntHandlerOPA1                ;  61: OPA1
                DCD     BSP_IntHandlerICAP0               ;  62: ICAP0
                DCD     BSP_IntHandlerICAP1               ;  63: ICAP1
                DCD     BSP_IntHandlerPWMA0               ;  64: PWM A channel 0
                DCD     BSP_IntHandlerPWMA1               ;  65: PWM A channel 1
                DCD     BSP_IntHandlerPWMA2               ;  66: PWM A channel 2
                DCD     BSP_IntHandlerPWMA3               ;  67: PWM A channel 3
                DCD     BSP_IntHandlerPWMA4               ;  68: PWM A channel 4
                DCD     BSP_IntHandlerPWMA5               ;  69: PWM A channel 5
                DCD     BSP_IntHandlerPWMABRK             ;  70: 
                DCD     BSP_IntHandlerQEI0                ;  71:
                DCD     BSP_IntHandlerPWMB0               ;  72: PWM B channel 0
                DCD     BSP_IntHandlerPWMB1               ;  73: PWM B channel 1
                DCD     BSP_IntHandlerPWMB2               ;  74: PWM B channel 2
                DCD     BSP_IntHandlerPWMB3               ;  75: PWM B channel 3
                DCD     BSP_IntHandlerPWMB4               ;  76: PWM B channel 4
                DCD     BSP_IntHandlerPWMB5               ;  77: PWM B channel 5
                DCD     BSP_IntHandlerPWMBBRK             ;  78: 
                DCD     BSP_IntHandlerQEI1                ;  79:
                DCD     BSP_IntHandlerEPWMA               ;  80:
                DCD     BSP_IntHandlerEPWMABRK            ;  81:
                DCD     BSP_IntHandlerEPWMB               ;  82:
                DCD     BSP_IntHandlerEPWMBBRK            ;  83:
                DCD     BSP_IntHandlerDefault             ;  84: Reserved
                DCD     BSP_IntHandlerDefault             ;  85: Reserved
                DCD     BSP_IntHandlerDefault             ;  86: Reserved
                DCD     BSP_IntHandlerDefault             ;  87: Reserved
                DCD     BSP_IntHandlerUSBD                ;  88: USB Device
                DCD     BSP_IntHandlerUSBH                ;  89: USB Host
                DCD     BSP_IntHandlerUSB_OTG             ;  90: USB On-The-Go
                DCD     BSP_IntHandlerDefault             ;  91: Reserved
                DCD     BSP_IntHandlerEMAC_TX             ;  92: Ethernet MAC TX
                DCD     BSP_IntHandlerEMAC_RX             ;  93: Ethernet MAC RX
                DCD     BSP_IntHandlerDefault             ;  94: Reserved
                DCD     BSP_IntHandlerDefault             ;  95: Reserved
                DCD     BSP_IntHandlerSPI0                ;  96: SPI0
                DCD     BSP_IntHandlerSPI1                ;  97: SPI1
                DCD     BSP_IntHandlerSPI2                ;  98: SPI2
                DCD     BSP_IntHandlerSPI3                ;  99: SPI3
                DCD     BSP_IntHandlerDefault             ; 100: Reserved
                DCD     BSP_IntHandlerDefault             ; 101: Reserved
                DCD     BSP_IntHandlerDefault             ; 102: Reserved
                DCD     BSP_IntHandlerDefault             ; 103: Reserved
                DCD     BSP_IntHandlerUART0               ; 104: UART0
                DCD     BSP_IntHandlerUART1               ; 105: UART1
                DCD     BSP_IntHandlerUART2               ; 106: UART2
                DCD     BSP_IntHandlerUART3               ; 107: UART3
                DCD     BSP_IntHandlerUART4               ; 108: UART4
                DCD     BSP_IntHandlerUART5               ; 109: UART5
                DCD     BSP_IntHandlerDefault             ; 110: Reserved
                DCD     BSP_IntHandlerDefault             ; 111: Reserved
                DCD     BSP_IntHandlerI2C0                ; 112: I2C 0
                DCD     BSP_IntHandlerI2C1                ; 113: I2C 1
                DCD     BSP_IntHandlerI2C2                ; 114: I2C 2
                DCD     BSP_IntHandlerI2C3                ; 115: I2C 3
                DCD     BSP_IntHandlerI2C4                ; 116: I2C 4
                DCD     BSP_IntHandlerDefault             ; 117: Reserved
                DCD     BSP_IntHandlerDefault             ; 118: Reserved
                DCD     BSP_IntHandlerDefault             ; 119: Reserved
                DCD     BSP_IntHandlerSC0                 ; 120: Smart Card 0
                DCD     BSP_IntHandlerSC1                 ; 121: Smart Card 1
                DCD     BSP_IntHandlerSC2                 ; 122: Smart Card 2
                DCD     BSP_IntHandlerSC3                 ; 123: Smart Card 3
                DCD     BSP_IntHandlerSC4                 ; 124: Smart Card 4
                DCD     BSP_IntHandlerSC5                 ; 125: Smart Card 5
                DCD     BSP_IntHandlerDefault             ; 126: Reserved
                DCD     BSP_IntHandlerDefault             ; 127: Reserved
                DCD     BSP_IntHandlerCAN0                ; 128: CAN 0
                DCD     BSP_IntHandlerCAN1                ; 129: CAN 1
                DCD     BSP_IntHandlerDefault             ; 130: Reserved
                DCD     BSP_IntHandlerDefault             ; 131: Reserved
                DCD     BSP_IntHandlerI2S0                ; 132: I2S0
                DCD     BSP_IntHandlerI2S1                ; 133: I2S1
                DCD     BSP_IntHandlerDefault             ; 134: Reserved
                DCD     BSP_IntHandlerDefault             ; 135: Reserved
                DCD     BSP_IntHandlerSD                  ; 136: SD Card
                DCD     BSP_IntHandlerDefault             ; 137: Reserved
                DCD     BSP_IntHandlerPS2D                ; 138: PS2 Device
                DCD     BSP_IntHandlerDefault             ; 139: Reserved
                DCD     BSP_IntHandlerCRYPTO              ; 140: Cryptographic Engine
                DCD     BSP_IntHandlerCRC                 ; 141: CRC

                AREA    |.text|, CODE, READONLY


; Reset Handler

Reset_Handler   PROC    
                EXPORT  Reset_Handler                 [WEAK]
                IMPORT  __main
				;===================================================
				; for M4 FPU         
				;===================================================       
                IF     :DEF:__TARGET_FPU_VFP
                ; Enable access to Floating-point coprocessor.
        		MOV     r0,#0xe000e000
        		MOV     r1,#0xf00000
        		STR     r1,[r0,#0xd88]
        		LDR     r1,[r0,#0xf34]
        		BIC     r1,r1,#0x80000000
        		STR     r1,[r0,#0xf34]
        		LDR     r1,[r0,#0xf34]
        		BIC     r1,r1,#0x40000000
        		STR     r1,[r0,#0xf34]
                ENDIF
                LDR     R0, =__main
                BX      R0
                ENDP

; Dummy Exception Handlers (infinite loops which can be modified)                

App_NMI_ISR      PROC
                EXPORT  App_NMI_ISR                   [WEAK]
                B       .
                ENDP
App_Fault_ISR\
                PROC
                EXPORT  App_Fault_ISR                 [WEAK]
                B       .
                ENDP
App_MemFault_ISR\
                PROC
                EXPORT  App_MemFault_ISR             [WEAK]
                B       .
                ENDP
App_BusFault_ISR\
                PROC
                EXPORT  App_BusFault_ISR             [WEAK]
                B       .
                ENDP
App_UsageFault_ISR\
                PROC
                EXPORT  App_UsageFault_ISR            [WEAK]
                B       .
                ENDP
App_Spurious_ISR\
                PROC
                EXPORT  App_Spurious_ISR                [WEAK]
                B       .
                ENDP
App_Reserved_ISR\
                PROC
                EXPORT  App_Reserved_ISR                [WEAK]
                B       .
                ENDP


                ALIGN


                                                                ; User Initial Stack & Heap

                IF      :DEF:__MICROLIB
                
                EXPORT  __initial_sp
                EXPORT  __heap_base
                EXPORT  __heap_limit
                
                ELSE
                
                IMPORT  __use_two_region_memory
                EXPORT  __user_initial_stackheap

__user_initial_stackheap

                LDR     R0, =  Heap_Mem
                LDR     R1, =(Stack_Mem + STACK_SIZE)
                LDR     R2, =(Heap_Mem  +  HEAP_SIZE)
                LDR     R3, = Stack_Mem
                BX      LR

                ALIGN

                ENDIF


                END
