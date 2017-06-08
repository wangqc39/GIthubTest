#include "main.h"

#define SHFSS_OUT_OF_CONTROL_TIME			(1200)
#define PWM_DEFAULT_VALUE					3000


unsigned short int ThPwm = PWM_DEFAULT_VALUE;
unsigned short int StPwm = PWM_DEFAULT_VALUE;

//0.5us一个单位，20ms一个周期，50HZ，1ms分辨率为2000
void PwmOutHwInit()
{
    TIM2_TimeBaseInit(TIM2_PRESCALER_8, 40000 - 1);
    TIM2_OC1Init(TIM2_OCMODE_PWM1, TIM2_OUTPUTSTATE_ENABLE, 0, TIM2_OCPOLARITY_HIGH);
    TIM2_OC2Init(TIM2_OCMODE_PWM1, TIM2_OUTPUTSTATE_ENABLE, 0, TIM2_OCPOLARITY_HIGH);
    //TIM2_ARRPreloadConfig(ENABLE);
    TIM2_OC1PreloadConfig(ENABLE);
    TIM2_OC2PreloadConfig(ENABLE);

    TIM2_Cmd(ENABLE);
}

unsigned int OutOfControlCnt = 0;
void SetPwmOut()
{
    static unsigned long LastRcHandleTime = 0;
    if(ActionTick <  LastRcHandleTime + 5)
        return;

    LastRcHandleTime = ActionTick;

    if(RecState == REC_RECEIVING ||
        (RecState == REC_DISCONNECTING && ActionTick < SHFSS_OUT_OF_CONTROL_TIME + ShfssConnectLastPacketTime))
    {
        TIM2_SetCompare1(StPwm);
        TIM2_SetCompare2(ThPwm);
    }
    else if(RecState == REC_DISCONNECTING && ActionTick > SHFSS_OUT_OF_CONTROL_TIME + ShfssConnectLastPacketTime)
    {
        RecState = REC_SEARCHING;
        OutOfControlCnt++;
    }
    else
    {
        ThPwm = PWM_DEFAULT_VALUE;
        StPwm = PWM_DEFAULT_VALUE;
        TIM2_SetCompare1(StPwm);
        TIM2_SetCompare2(ThPwm);
    }
}

