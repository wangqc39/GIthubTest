#include "main.h"

#define RC_CALL_BACK_TIMER			TIM1

volatile unsigned long ActionTick;
u16 (*SysTimerCallBackFunc)(void);

void CallBackTimerInit()
{
    
    //TIM1_TimeBaseInit(15,
    //                                  TIM1_COUNTERMODE_UP, 
    //                                  0xFFFF, 
    //                                  0);
    //TIM1->ARRH = (uint8_t)(TIM1_Period >> 8);
    //TIM1->ARRL = (uint8_t)(TIM1_Period);

    //TIM1->PSCRH = (uint8_t)(TIM1_Prescaler >> 8);
    TIM1->PSCRL = 15;

    //TIM1->CR1 = (uint8_t)((uint8_t)(TIM1->CR1 & (uint8_t)(~(TIM1_CR1_CMS | TIM1_CR1_DIR)))
    //                       | (uint8_t)(TIM1_CounterMode));
    //TIM1->RCR = TIM1_RepetitionCounter;
                                      
    //CallBackTimerStop();
    

    //TIM1_OC1Init(TIM1_OCMODE_TIMING, TIM1_OUTPUTSTATE_DISABLE, TIM1_OUTPUTNSTATE_DISABLE, 0, 
    //                        TIM1_OCPOLARITY_HIGH, TIM1_OCNPOLARITY_HIGH, TIM1_OCIDLESTATE_RESET, TIM1_OCNIDLESTATE_RESET);
    /*TIM1->CCER1 &= (uint8_t)(~( TIM1_CCER1_CC1E | TIM1_CCER1_CC1NE 
                               | TIM1_CCER1_CC1P | TIM1_CCER1_CC1NP));
    TIM1->CCER1 |= (uint8_t)((uint8_t)((uint8_t)(TIM1_OutputState & TIM1_CCER1_CC1E)
                                     | (uint8_t)(TIM1_OutputNState & TIM1_CCER1_CC1NE))
                           | (uint8_t)( (uint8_t)(TIM1_OCPolarity  & TIM1_CCER1_CC1P)
                                        | (uint8_t)(TIM1_OCNPolarity & TIM1_CCER1_CC1NP)));*/
    //TIM1->CCMR1 = (uint8_t)((uint8_t)(TIM1->CCMR1 & (uint8_t)(~TIM1_CCMR_OCM)) | 
    //                        (uint8_t)TIM1_OCMode);

    //TIM1->OISR &= (uint8_t)(~(TIM1_OISR_OIS1 | TIM1_OISR_OIS1N));
    //TIM1->OISR |= (uint8_t)((uint8_t)( TIM1_OCIdleState & TIM1_OISR_OIS1 ) | 
    //                        (uint8_t)( TIM1_OCNIdleState & TIM1_OISR_OIS1N ));

    /* Set the Pulse value */
    //TIM1->CCR1H = (uint8_t)(TIM1_Pulse >> 8);
    //TIM1->CCR1L = (uint8_t)(TIM1_Pulse);
    //ITC_SetSoftwarePriority(ITC_IRQ_TIM1_CAPCOM, ITC_PRIORITYLEVEL_3);
    
    //TIM1_Cmd(ENABLE);
    TIM1->CR1 = 1;
}

void CallBackTimerStart(u16 us, u16 (*cb)(void))
{
    u16 t;

    if(! cb)       return;
    SysTimerCallBackFunc = cb;
    
    //t =  TIM1->CNTRL | ((u16)TIM1->CNTRH << 8);
    t = TIM1_GetCounter();

    //TIM1_SetCompare1(us + t);
    t = us + t;
    TIM1->CCR1H = (uint8_t)(t >> 8);
    TIM1->CCR1L = (uint8_t)(t);
    //TIM1_ClearITPendingBit(TIM1_IT_CC1);
    //TIM1->SR1 = (uint8_t)(~(uint8_t)TIM1_IT_CC1);
    //TIM1_ITConfig(TIM1_IT_CC1, ENABLE);
    TIM1->IER |= (uint8_t)TIM1_IT_CC1;
}

void CallBackTimerStop(void) 
{
    //TIM1_ITConfig(TIM1_IT_CC1, DISABLE);
    TIM1->IER &= (uint8_t)(~(uint8_t)TIM1_IT_CC1);
    SysTimerCallBackFunc = NULL;
}

void TIM1_CC_IRQHandler()
{
    u16 us;
    uint16_t tmpccr1 = 0;
    uint8_t tmpccr1l=0, tmpccr1h=0;
    if(TIM1_GetITStatus(TIM1_IT_CC1) != RESET)
    {
        if(SysTimerCallBackFunc)
        {
            us = SysTimerCallBackFunc();
            //TIM1_ClearITPendingBit(TIM1_IT_CC1);
            TIM1->SR1 = (uint8_t)(~(uint8_t)TIM1_IT_CC1);
            if (us)
            {
                tmpccr1h = TIM1->CCR1H;
                tmpccr1l = TIM1->CCR1L;
                tmpccr1 = (uint16_t)(tmpccr1l);
                tmpccr1 |= (uint16_t)((uint16_t)tmpccr1h << 8);
                tmpccr1 += us;
                TIM1->CCR1H = (uint8_t)(tmpccr1 >> 8);
                TIM1->CCR1L = (uint8_t)(tmpccr1);
    
                //TIM1_SetCompare1(us + TIM1_GetCapture1());
                return;
            }
        }
        CallBackTimerStop();
    }
    else
    {
        ForceHopChannel();
	TIM1_ClearITPendingBit(TIM1_IT_CC2);		
    }   
}

void SetForceHopTimeByLastForce(unsigned short int TimeInterval)
{
    TIM1_SetCompare2(TIM1_GetCapture2() + TimeInterval);
}

void SetForceHopTimeByNow(unsigned short int TimeInterval)
{
    uint16_t NowCnt;
    //NowCnt = (uint16_t)(RC_CALL_BACK_TIMER->CNTRL) | (uint16_t)((uint16_t)RC_CALL_BACK_TIMER->CNTRH << 8);
    NowCnt = TIM1_GetCounter();
    TIM1_SetCompare2(NowCnt + TimeInterval);
}

void StartForceHopTime(unsigned short int TimeInterval)
{
    SetForceHopTimeByNow(TimeInterval);
    TIM1_ITConfig(TIM1_IT_CC2, ENABLE);
    TIM1_ClearITPendingBit(TIM1_IT_CC2);
}

void StopForceHopTime()
{
    TIM1_ITConfig(TIM1_IT_CC2, DISABLE);
}

void ClearForceHopTimeInt()
{
    TIM1_ClearITPendingBit(TIM1_IT_CC2);
}




void usleep2us(u32 x)
{
    while(x > 0)
    {
        x--;
    }
}

void mSleep(u32 ms)
{
    unsigned long DistTick;
    DistTick = ActionTick + ms;
    while(ActionTick < DistTick);
}

void ActionTickInit()
{
    //TIM4_TimeBaseInit(TIM4_PRESCALER_128, 125 - 1);
    TIM4->PSCR = TIM4_PRESCALER_128;
    TIM4->ARR = 125 - 1;
    //ITC_SetSoftwarePriority(ITC_IRQ_TIM4_OVF, ITC_PRIORITYLEVEL_1);
    ITC->ISPR6 = 0x7F;
    //TIM4_ITConfig(TIM4_IT_UPDATE, ENABLE);
    TIM4->IER |= (uint8_t)TIM4_IT_UPDATE;
    //TIM4_Cmd(ENABLE);
    TIM4->CR1 |= TIM4_CR1_CEN;
}

void SysTick_Handler(void)
{
    TIM4->SR1 = ~TIM4_FLAG_UPDATE;
    ActionTick++;
}


