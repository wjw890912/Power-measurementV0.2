/*
*********************************************************************************************************
*                                     MICIRUM BOARD SUPPORT PACKAGE
*
*                             (c) Copyright 2013; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                        BOARD SUPPORT PACKAGE
*
* Filename      : bsp.c
* Version       : V1.00
* Programmer(s) : FF
*                 DC
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#define   BSP_MODULE
#include  <stdio.h>
#include  <string.h>
#include  <bsp.h>
#include  <bsp_os.h>

void bsp_printf(char * pcStr,...);

/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/
static CPU_INT32U  _cpu_freq = HIRC_HZ;
static CPU_INT32U  _pll_freq = PLL_DEFAULT_HZ;

/*
*********************************************************************************************************
*                                           LOCAL CONSTANTS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                          LOCAL DATA TYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            LOCAL TABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             REGISTERS
*********************************************************************************************************
*/

#define  BSP_REG_DEM_CR                       (*(CPU_REG32 *)0xE000EDFC)
#define  BSP_REG_DWT_CR                       (*(CPU_REG32 *)0xE0001000)
#define  BSP_REG_DWT_CYCCNT                   (*(CPU_REG32 *)0xE0001004)
#define  BSP_REG_DBGMCU_CR                    (*(CPU_REG32 *)0xE0042004)


/*
*********************************************************************************************************
*                                            REGISTER BITS
*********************************************************************************************************
*/

#define  BSP_DBGMCU_CR_TRACE_IOEN_MASK                   0x10
#define  BSP_DBGMCU_CR_TRACE_MODE_ASYNC                  0x00
#define  BSP_DBGMCU_CR_TRACE_MODE_SYNC_01                0x40
#define  BSP_DBGMCU_CR_TRACE_MODE_SYNC_02                0x80
#define  BSP_DBGMCU_CR_TRACE_MODE_SYNC_04                0xC0
#define  BSP_DBGMCU_CR_TRACE_MODE_MASK                   0xC0

#define  BSP_BIT_DEM_CR_TRCENA                    DEF_BIT_24

#define  BSP_BIT_DWT_CR_CYCCNTENA                 DEF_BIT_00


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                               BSP_Init()
*
* Description : Initialize the Board Support Package (BSP).
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) This function SHOULD be called before any other BSP function is called.
*
*               (2) CPU instruction / data tracing requires the use of the following pins :
*                   (a) (1) Aysynchronous     :  PB[3]
*                       (2) Synchronous 1-bit :  PE[3:2]
*                       (3) Synchronous 2-bit :  PE[4:2]
*                       (4) Synchronous 4-bit :  PE[6:2]
*
*                   (c) The application may wish to adjust the trace bus width depending on I/O
*                       requirements.
*********************************************************************************************************
*/

void  BSP_Init (void)
{
    BSP_IntDisAll();                                            /* Disable all interrupts.                              */

  	/* M4F FPU settings --------------------------------------------------------*/
#ifdef __TARGET_FPU_VFP
    SCB->CPACR |= ((3UL << 10*2) |                 	/* set CP10 Full Access */
                   (3UL << 11*2)  );               	/* set CP11 Full Access */
    //FPU->FPCCR &= ~(3 << 30);						/* Disable automatic FP register content (bit 31) */
    //												/* Disable Lazy context switch (bit 30) */
#endif

  	/* Enable external crystal and PLL -----------------------------------------*/
	BSP_Enable_Clock(CLK_XTL_12M);								/* Enable external external 4 ~ 22M input               */

    // Enable UART0 clock
    CLK->APBCLK0 |= CLK_APBCLK0_UART0CKEN_Msk;
    
    // Select external 12 MHz as UART clock source 
    CLK->CLKSEL1 &= ~CLK_CLKSEL1_UARTSEL_Msk;
    
    /* Set GPG multi-function pins for UART0 RXD and TXD */
    SYS->GPG_MFPL = SYS_GPG_MFPL_PG1MFP_UART0_RXD | SYS_GPG_MFPL_PG2MFP_UART0_TXD ;

    UART0->LINE |= 0x07;	
    UART0->BAUD = 0x30000066;	/* 12MHz reference clock input, for 115200 */

    BSP_IntInit();

    BSP_Tick_Init();                                            /* Start Tick Initialization                            */
}

/*
*********************************************************************************************************
*                                            BSP_CPU_ClkFreq()
*
* Description : Read CPU registers to determine the CPU clock frequency of the chip.
*
* Argument(s) : none.
*
* Return(s)   : The CPU clock frequency, in Hz.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_INT32U  BSP_CPU_ClkFreq (void)
{
    return _cpu_freq;
}


static int ClockStableCheck(CPU_INT32U clk_mask)
{
	volatile CPU_INT32U  time_out;
	
   	for (time_out = HIRC_HZ/100; time_out > 0; time_out--)
   	{
   		if ((CLK->STATUS & clk_mask) == clk_mask)
   			break;
	}
    			
	if (time_out == 0)
		return -1;
	return 0;
}


/*
*********************************************************************************************************
*                                            BSP_Enable_Clock()
*
* Description : Enable clock input
*
* Argument(s) : clk    Input clock
*
* Return(s)   : 0        Success
*               -1       Failed
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/
int BSP_Enable_Clock(E_CLK_SRC clk)
{
	switch (clk)
	{
		case CLK_XTL_12M:
			CLK->PWRCTL |= CLK_PWRCTL_HXTEN_Msk;
			if (ClockStableCheck(CLK_STATUS_HXTSTB_Msk) < 0)
				return -1;
			break; 

		case CLK_XTL_32K:
			CLK->PWRCTL |= CLK_PWRCTL_LXTEN_Msk;
			if (ClockStableCheck(CLK_STATUS_LXTSTB_Msk) < 0)
				return -1;
			break;

		case CLK_OSC_22M:
			CLK->PWRCTL |= CLK_PWRCTL_HIRCEN_Msk;
			if (ClockStableCheck(CLK_STATUS_HIRCSTB_Msk) < 0)
				return -1;
			break; 

		case CLK_OSC_10K:
			CLK->PWRCTL |= CLK_PWRCTL_LIRCEN_Msk;
			if (ClockStableCheck(CLK_STATUS_LIRCSTB_Msk) < 0)
				return -1;
			break; 

		case CLK_PLL:
			CLK->PLLCTL &= ~CLK_PLLCTL_OE_Msk;
			if (ClockStableCheck(CLK_STATUS_PLLSTB_Msk) < 0)
				return -1;
			break; 

		default:
			return -1;
	}
	return 0;
}


/*
*********************************************************************************************************
*                                            BSP_Set_System_Clock()
*
* Description : Select HCLK clock source and divider
*
* Argument(s) : clk_src  HCLK clock source. Must be type E_CLK_SRC.
*               clk_div  HCLK clock frequency = (HCLK clock source frequency) / (clk_div + 1)
*                        The valid range of clk_div is 0 <= clk_div <= 15.
*
* Return(s)   : 0        Success
*               -1       Failed
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/
int  BSP_Set_System_Clock (E_CLK_SRC clk_src, int clk_div)
{
	switch (clk_src)
	{
		case CLK_XTL_12M:
			CLK->CLKSEL0 = (CLK->CLKSEL0 & ~CLK_CLKSEL0_HCLKSEL_Msk);
			_cpu_freq = HXT_HZ / (clk_div+1);
			break; 

		case CLK_XTL_32K:
			CLK->CLKSEL0 = (CLK->CLKSEL0 & ~CLK_CLKSEL0_HCLKSEL_Msk) | 0x1;
			_cpu_freq = LXT_HZ / (clk_div+1);
			break; 

		case CLK_OSC_22M:
			CLK->CLKSEL0 = (CLK->CLKSEL0 & ~CLK_CLKSEL0_HCLKSEL_Msk) | 0x7;
			_cpu_freq = HIRC_HZ / (clk_div+1);
			break; 

		case CLK_OSC_10K:
			CLK->CLKSEL0 = (CLK->CLKSEL0 & ~CLK_CLKSEL0_HCLKSEL_Msk) | 0x3;
			_cpu_freq = LIRC_HZ / (clk_div+1);
			break; 

		case CLK_PLL:
			CLK->CLKSEL0 = (CLK->CLKSEL0 & ~CLK_CLKSEL0_HCLKSEL_Msk) | 0x2;
			_cpu_freq = _pll_freq / (clk_div+1);
			break; 
			
		default:
			return -1;
	}

	bsp_printf("Reinit system tick with HCLK %d.\n", _cpu_freq);
	BSP_Tick_Init();
	
	return 0;
}


/*
*********************************************************************************************************
*                                            BSP_Set_PLL_Freq()
*
* Description : Set PLL clock source and output frequency
*
* Argument(s) : pll_src       PLL clock source
*               pll_freq_Hz   The target PLL output frequency.
*
* Return(s)   : 0        Success
*               -1       Failed
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

int  BSP_Set_PLL_Freq (E_CLK_SRC pll_src, CPU_INT32U pll_freq_Hz)
{
	CPU_INT32U   r_out_dv, r_in_dv, r_fb_dv;
	CPU_INT32U   check, fin, fout, nr, no, nf; 
	CPU_INT32U   diff, best_diff, best_val = 0, best_freq = 0;
	
	if ((pll_src != CLK_XTL_12M) && (pll_src != CLK_OSC_22M))
		return -1;
	
	if (pll_src == CLK_XTL_12M)
		fin = HXT_HZ;
	else
		fin = HIRC_HZ;
		
	best_diff = 0x7fffffff;
	
	for (r_out_dv = 0; r_out_dv <= 3; r_out_dv++)
	{
		for (r_in_dv = 0; r_in_dv <= 0x1f; r_in_dv++)
		{
			for (r_fb_dv = 0; r_fb_dv <= 0x1ff; r_fb_dv++)
			{
				nr = r_in_dv + 2;
				no = (r_out_dv == 0) ? 1 : (r_out_dv == 3 ? 4 : 2);
				nf = r_fb_dv + 2;
				
				check = (fin/1000) / (2 * nr);
				if ((check <= 800) || (check >= 8000))
					continue;
				
				check = ((fin/1000) * nf) / nr;
				
				if ((check <= 100000) || (check >= 200000))
					continue;
				
				if (check < 120000)
					continue;

				fout = ((fin/1000) * nf) / (nr * no);
				fout *= 1000;
				
				if (fout > pll_freq_Hz)
					diff = fout - pll_freq_Hz;
				else
					diff = pll_freq_Hz - fout;
				
				if (diff < best_diff)
				{
					best_diff = diff;
					best_val = (r_out_dv << 14) | (r_in_dv << 9) | r_fb_dv;
					best_freq = fout;
					if (diff == 0)
						break;
				}
			}
		}
	} 

	if (pll_src == CLK_XTL_12M)
		CLK->PLLCTL = best_val;
	else
		CLK->PLLCTL = CLK_PLLCTL_PLLSRC_Msk | best_val;
		
	_pll_freq = best_freq;
	
	bsp_printf("PLLCTL=0x%x, FREQ=%d.%02d MHz\n", best_val, _pll_freq/1000000, (_pll_freq % 1000000) / 10000);
	return 0;
}



/*
*********************************************************************************************************
*                                            BSP_Tick_Init()
*
* Description : Initialize all the peripherals that required OS Tick services (OS initialized)
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/
void BSP_Tick_Init (void)
{
    CPU_INT32U  cpu_clk_freq;
    CPU_INT32U  cnts;
    
    cpu_clk_freq = BSP_CPU_ClkFreq();                           /* Determine SysTick reference freq.                    */
    
#if (OS_VERSION >= 30000u)
    cnts  = cpu_clk_freq / (CPU_INT32U)OSCfg_TickRate_Hz;       /* Determine nbr SysTick increments.                    */
#else
    cnts  = cpu_clk_freq / (CPU_INT32U)OS_TICKS_PER_SEC;        /* Determine nbr SysTick increments.                    */
#endif
    OS_CPU_SysTickInit(cnts);                                   /* Init uC/OS periodic time src (SysTick).              */
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                          CPU_TS_TmrInit()
*
* Description : Initialize & start CPU timestamp timer.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : CPU_TS_Init().
*
*               This function is an INTERNAL CPU module function & MUST be implemented by application/
*               BSP function(s) [see Note #1] but MUST NOT be called by application function(s).
*
* Note(s)     : (1) CPU_TS_TmrInit() is an application/BSP function that MUST be defined by the developer 
*                   if either of the following CPU features is enabled :
*
*                   (a) CPU timestamps
*                   (b) CPU interrupts disabled time measurements
*
*                   See 'cpu_cfg.h  CPU TIMESTAMP CONFIGURATION  Note #1'
*                     & 'cpu_cfg.h  CPU INTERRUPTS DISABLED TIME MEASUREMENT CONFIGURATION  Note #1a'.
*
*               (2) (a) Timer count values MUST be returned via word-size-configurable 'CPU_TS_TMR' 
*                       data type.
*
*                       (1) If timer has more bits, truncate timer values' higher-order bits greater 
*                           than the configured 'CPU_TS_TMR' timestamp timer data type word size.
*
*                       (2) Since the timer MUST NOT have less bits than the configured 'CPU_TS_TMR' 
*                           timestamp timer data type word size; 'CPU_CFG_TS_TMR_SIZE' MUST be 
*                           configured so that ALL bits in 'CPU_TS_TMR' data type are significant.
*
*                           In other words, if timer size is not a binary-multiple of 8-bit octets 
*                           (e.g. 20-bits or even 24-bits), then the next lower, binary-multiple 
*                           octet word size SHOULD be configured (e.g. to 16-bits).  However, the 
*                           minimum supported word size for CPU timestamp timers is 8-bits.
*
*                       See also 'cpu_cfg.h   CPU TIMESTAMP CONFIGURATION  Note #2'
*                              & 'cpu_core.h  CPU TIMESTAMP DATA TYPES     Note #1'.
*
*                   (b) Timer SHOULD be an 'up'  counter whose values increase with each time count.
*
*                   (c) When applicable, timer period SHOULD be less than the typical measured time 
*                       but MUST be less than the maximum measured time; otherwise, timer resolution 
*                       inadequate to measure desired times.
*
*                   See also 'CPU_TS_TmrRd()  Note #2'.
*********************************************************************************************************
*/

#if (CPU_CFG_TS_TMR_EN == DEF_ENABLED)
void  CPU_TS_TmrInit (void)
{
    CPU_INT32U  fclk_freq;


    fclk_freq = BSP_CPU_ClkFreq();

    BSP_REG_DEM_CR     |= (CPU_INT32U)BSP_BIT_DEM_CR_TRCENA;    /* Enable Cortex-M4's DWT CYCCNT reg.                   */
    BSP_REG_DWT_CYCCNT  = (CPU_INT32U)0u;
    BSP_REG_DWT_CR     |= (CPU_INT32U)BSP_BIT_DWT_CR_CYCCNTENA;

    CPU_TS_TmrFreqSet((CPU_TS_TMR_FREQ)fclk_freq);
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                           CPU_TS_TmrRd()
*
* Description : Get current CPU timestamp timer count value.
*
* Argument(s) : none.
*
* Return(s)   : Timestamp timer count (see Notes #2a & #2b).
*
* Caller(s)   : CPU_TS_Init(),
*               CPU_TS_Get32(),
*               CPU_TS_Get64(),
*               CPU_IntDisMeasStart(),
*               CPU_IntDisMeasStop().
*
*               This function is an INTERNAL CPU module function & MUST be implemented by application/
*               BSP function(s) [see Note #1] but SHOULD NOT be called by application function(s).
*
* Note(s)     : (1) CPU_TS_TmrRd() is an application/BSP function that MUST be defined by the developer 
*                   if either of the following CPU features is enabled :
*
*                   (a) CPU timestamps
*                   (b) CPU interrupts disabled time measurements
*
*                   See 'cpu_cfg.h  CPU TIMESTAMP CONFIGURATION  Note #1'
*                     & 'cpu_cfg.h  CPU INTERRUPTS DISABLED TIME MEASUREMENT CONFIGURATION  Note #1a'.
*
*               (2) (a) Timer count values MUST be returned via word-size-configurable 'CPU_TS_TMR' 
*                       data type.
*
*                       (1) If timer has more bits, truncate timer values' higher-order bits greater 
*                           than the configured 'CPU_TS_TMR' timestamp timer data type word size.
*
*                       (2) Since the timer MUST NOT have less bits than the configured 'CPU_TS_TMR' 
*                           timestamp timer data type word size; 'CPU_CFG_TS_TMR_SIZE' MUST be 
*                           configured so that ALL bits in 'CPU_TS_TMR' data type are significant.
*
*                           In other words, if timer size is not a binary-multiple of 8-bit octets 
*                           (e.g. 20-bits or even 24-bits), then the next lower, binary-multiple 
*                           octet word size SHOULD be configured (e.g. to 16-bits).  However, the 
*                           minimum supported word size for CPU timestamp timers is 8-bits.
*
*                       See also 'cpu_cfg.h   CPU TIMESTAMP CONFIGURATION  Note #2'
*                              & 'cpu_core.h  CPU TIMESTAMP DATA TYPES     Note #1'.
*
*                   (b) Timer SHOULD be an 'up'  counter whose values increase with each time count.
*
*                       (1) If timer is a 'down' counter whose values decrease with each time count,
*                           then the returned timer value MUST be ones-complemented.
*
*                   (c) (1) When applicable, the amount of time measured by CPU timestamps is 
*                           calculated by either of the following equations :
*
*                           (A) Time measured  =  Number timer counts  *  Timer period
*
*                                   where
*
*                                       Number timer counts     Number of timer counts measured 
*                                       Timer period            Timer's period in some units of 
*                                                                   (fractional) seconds
*                                       Time measured           Amount of time measured, in same 
*                                                                   units of (fractional) seconds 
*                                                                   as the Timer period
*
*                                                  Number timer counts
*                           (B) Time measured  =  ---------------------
*                                                    Timer frequency
*
*                                   where
*
*                                       Number timer counts     Number of timer counts measured
*                                       Timer frequency         Timer's frequency in some units 
*                                                                   of counts per second
*                                       Time measured           Amount of time measured, in seconds
*
*                       (2) Timer period SHOULD be less than the typical measured time but MUST be less 
*                           than the maximum measured time; otherwise, timer resolution inadequate to 
*                           measure desired times.
*********************************************************************************************************
*/

#if (CPU_CFG_TS_TMR_EN == DEF_ENABLED)
CPU_TS_TMR  CPU_TS_TmrRd (void)
{
    CPU_TS_TMR  ts_tmr_cnts;

                                                                
    ts_tmr_cnts = (CPU_TS_TMR)BSP_REG_DWT_CYCCNT;

    return (ts_tmr_cnts);
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                         CPU_TSxx_to_uSec()
*
* Description : Convert a 32-/64-bit CPU timestamp from timer counts to microseconds.
*
* Argument(s) : ts_cnts   CPU timestamp (in timestamp timer counts [see Note #2aA]).
*
* Return(s)   : Converted CPU timestamp (in microseconds           [see Note #2aD]).
*
* Caller(s)   : Application.
*
*               This function is an (optional) CPU module application programming interface (API) 
*               function which MAY be implemented by application/BSP function(s) [see Note #1] & 
*               MAY be called by application function(s).
*
* Note(s)     : (1) CPU_TS32_to_uSec()/CPU_TS64_to_uSec() are application/BSP functions that MAY be 
*                   optionally defined by the developer when either of the following CPU features is 
*                   enabled :
*
*                   (a) CPU timestamps
*                   (b) CPU interrupts disabled time measurements
*
*                   See 'cpu_cfg.h  CPU TIMESTAMP CONFIGURATION  Note #1'
*                     & 'cpu_cfg.h  CPU INTERRUPTS DISABLED TIME MEASUREMENT CONFIGURATION  Note #1a'.
*
*               (2) (a) The amount of time measured by CPU timestamps is calculated by either of 
*                       the following equations :
*
*                                                                        10^6 microseconds
*                       (1) Time measured  =   Number timer counts   *  -------------------  *  Timer period
*                                                                            1 second
*
*                                              Number timer counts       10^6 microseconds
*                       (2) Time measured  =  ---------------------  *  -------------------
*                                                Timer frequency             1 second
*
*                               where
*
*                                   (A) Number timer counts     Number of timer counts measured
*                                   (B) Timer frequency         Timer's frequency in some units 
*                                                                   of counts per second
*                                   (C) Timer period            Timer's period in some units of 
*                                                                   (fractional)  seconds
*                                   (D) Time measured           Amount of time measured, 
*                                                                   in microseconds
*
*                   (b) Timer period SHOULD be less than the typical measured time but MUST be less 
*                       than the maximum measured time; otherwise, timer resolution inadequate to 
*                       measure desired times.
*
*                   (c) Specific implementations may convert any number of CPU_TS32 or CPU_TS64 bits 
*                       -- up to 32 or 64, respectively -- into microseconds.
*********************************************************************************************************
*/

#if (CPU_CFG_TS_32_EN == DEF_ENABLED)
CPU_INT64U  CPU_TS32_to_uSec (CPU_TS32  ts_cnts)
{
    CPU_INT64U  ts_us;
    CPU_INT64U  fclk_freq;

    
    fclk_freq = BSP_CPU_ClkFreq();    
    ts_us     = ts_cnts / (fclk_freq / DEF_TIME_NBR_uS_PER_SEC);

    return (ts_us);
}
#endif


#if (CPU_CFG_TS_64_EN == DEF_ENABLED)
CPU_INT64U  CPU_TS64_to_uSec (CPU_TS64  ts_cnts)
{
    CPU_INT64U  ts_us;
    CPU_INT64U  fclk_freq;
    

    fclk_freq = BSP_CPU_ClkFreq();    
    ts_us     = ts_cnts / (fclk_freq / DEF_TIME_NBR_uS_PER_SEC);

    return (ts_us);
}
#endif


char bsp_getchar()
{
	while (1)
	{
    	if ((UART0->FIFOSTS & 0x00004000) == 0 )
        {
        	return (UART0->DAT);
        }
	}
}


void bsp_putchar(unsigned char ch)
{
	while ((UART0->FIFOSTS & 0x00800000)); //waits for TX_FULL bit is clear
    
    UART0->DAT = ch;
    if (ch == '\n')
    {
    	while((UART0->FIFOSTS & 0x00800000)); //waits for TX_FULL bit is clear
        UART0->DAT = '\r';
    }
}


void bsp_put_string(char *string)
{
    while (*string != '\0')
    {
        bsp_putchar(*string);
        string++;
    }
}


static void bsp_put_rep_char(char c, int count)
{
    while (count--)
        bsp_putchar(c);
}


static void put_string_reverse(char *s, int index)
{
    while ((index--) > 0)
        bsp_putchar(s[index]);
}


static void bsp_put_number(int value, int radix, int width, char fill)
{
    char    buffer[24];
    int     bi = 0;
    unsigned int  uvalue;
    unsigned short  digit;
    unsigned short  left = 0;
    unsigned short  negative = 0;

    if (fill == 0)
        fill = ' ';

    if (width < 0)
    {
        width = -width;
        left = 1;
    }

    if (width < 0 || width > 80)
        width = 0;

    if (radix < 0)
    {
        radix = -radix;
        if (value < 0)
        {
            negative = 1;
            value = -value;
        }
    }

    uvalue = value;

    do
    {
        if (radix != 16)
        {
            digit = uvalue % radix;
            uvalue = uvalue / radix;
        }
        else
        {
            digit = uvalue & 0xf;
            uvalue = uvalue >> 4;
        }
        buffer[bi] = digit + ((digit <= 9) ? '0' : ('A' - 10));
        bi++;

        if (uvalue != 0)
        {
            if ((radix == 10)
                && ((bi == 3) || (bi == 7) || (bi == 11) | (bi == 15)))
            {
                buffer[bi++] = ',';
            }
        }
    }
    while (uvalue != 0);

    if (negative)
    {
        buffer[bi] = '-';
        bi += 1;
    }

    if (width <= bi)
        put_string_reverse(buffer, bi);
    else
    {
        width -= bi;
        if (!left)
            bsp_put_rep_char(fill, width);
        put_string_reverse(buffer, bi);
        if (left)
            bsp_put_rep_char(fill, width);
    }
}


static char *format_item(char *f, int a)
{
    char   c;
    int    fieldwidth = 0;
    int    leftjust = 0;
    int    radix = 0;
    char   fill = ' ';

    if (*f == '0')
        fill = '0';

    while ((c = *f++) != 0)
    {
        if (c >= '0' && c <= '9')
        {
            fieldwidth = (fieldwidth * 10) + (c - '0');
        }
        else
            switch (c)
            {
                case '\000':
                    return (--f);
                case '%':
                    bsp_putchar('%');
                    return (f);
                case '-':
                    leftjust = 1;
                    break;
                case 'c':
                    {
                        if (leftjust)
                            bsp_putchar(a & 0x7f);

                        if (fieldwidth > 0)
                            bsp_put_rep_char(fill, fieldwidth - 1);

                        if (!leftjust)
                            bsp_putchar(a & 0x7f);
                        return (f);
                    }
                case 's':
                    {
                        if (leftjust)
                            bsp_put_string((char *)a);

                        if (fieldwidth > strlen((char *)a))
                            bsp_put_rep_char(fill, fieldwidth - strlen((char *)a));

                        if (!leftjust)
                            bsp_put_string((char *)a);
                        return (f);
                    }
                case 'd':
                case 'i':
                    radix = -10;
                    break;
                case 'u':
                    radix = 10;
                    break;
                case 'x':
                    radix = 16;
                    break;
                case 'X':
                    radix = 16;
                    break;
                case 'o':
                    radix = 8;
                    break;
                default:
                    radix = 3;
                    break;      /* unknown switch! */
            }
        if (radix)
            break;
    }

    if (leftjust)
        fieldwidth = -fieldwidth;

    bsp_put_number(a, radix, fieldwidth, fill);

    return (f);
}


#define vaStart(list, param) list = (char*)((int)&param + sizeof(param))
#define vaArg(list, type) ((type *)(list += sizeof(type)))[-1]


void bsp_printf(char * pcStr,...)
{
    char  *argP;

    vaStart(argP, pcStr);       /* point at the end of the format string */
    while (*pcStr)
    {                       /* this works because args are all ints */
        if (*pcStr == '%')
            pcStr = format_item(pcStr + 1, vaArg(argP, int));
        else
            bsp_putchar(*pcStr++);
    }
}

