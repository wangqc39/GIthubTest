/******************** (C) COPYRIGHT 2007 STMicroelectronics ********************
* File Name          : main.c
* Author             : MCD Application Team
* Version            : V1.0
* Date               : 10/08/2007
* Description        : Main program body
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "main.h"


/* Private typedef -----------------------------------------------------------*/
typedef enum { FAILED = 0, PASSED = !FAILED} TestStatus;

/* Private define ------------------------------------------------------------*/        

u8 flag;
int main(void)
{								 	
#ifdef DEBUG
    debug();
#endif

    /* System clocks configuration ---------------------------------------------*/
    //CLK_SYSCLKConfig(CLK_PRESCALER_HSIDIV1);
    CLK->CKDIVR = 0;
    //CLK->CKDIVR &= (uint8_t)(~CLK_CKDIVR_HSIDIV);
    //CLK->CKDIVR |= (uint8_t)((uint8_t)CLK_PRESCALER_HSIDIV1 & (uint8_t)CLK_CKDIVR_HSIDIV);
  
    asm("rim");
  
    ActionTickInit();
    CC2500SpiInit();
    LedHwInit();
    CallBackTimerInit();
    ShfssInit();
    PwmOutHwInit();
  
  
    
    while (1)				   
    {	
        KeyHandler();
        LedHandler();
        SetPwmOut();
        CheckAndSaveBindId();
    }
}


#ifdef  DEBUG
/*******************************************************************************
* Function Name  : assert_failed
* Description    : Reports the name of the source file and the source line number
*                  where the assert_param error has occurred.
* Input          : - file: pointer to the source file name
*                  - line: assert_param error line source number
* Output         : None
* Return         : None
*******************************************************************************/
void assert_failed(u8* file, u32 line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/
