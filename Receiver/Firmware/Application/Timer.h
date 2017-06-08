#ifndef __TIMER__
#define __TIMER__



extern volatile unsigned long ActionTick;


void CallBackTimerInit(void);
void CallBackTimerStart(u16 us, u16 (*cb)(void));
void CallBackTimerStop(void);
void TIM1_CC_IRQHandler(void);
void SetForceHopTimeByLastForce(unsigned short int TimeInterval);
void SetForceHopTimeByNow(unsigned short int TimeInterval);
void StartForceHopTime(unsigned short int TimeInterval);
void StopForceHopTime(void);
void ClearForceHopTimeInt(void);
void usleep2us(u32 x);
void mSleep(u32 ms);
void SysTick_Handler(void);
void ActionTickInit(void);

#endif

