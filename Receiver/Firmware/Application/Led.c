#include "main.h"

#define ON_CHIP_LED_ON 			GPIO_WriteHigh(GPIOA, GPIO_PIN_1)
#define ON_CHIP_LED_OFF 		GPIO_WriteLow(GPIOA, GPIO_PIN_1)

#define BIND_STATUS_LED_FLICKER_INTERVAL	75

void LedHwInit()
{
    GPIO_Init(GPIOA, GPIO_PIN_1, GPIO_MODE_OUT_PP_LOW_SLOW);
}

u8 LedStatus = 0;
u32 StartBindBlinkTime = 0;
u32 LastFlickTime;
void LedHandler()
{
    if(ActionTick < 1000)
    {
        //ÉÏµçÁÁ1Ãë
        if(LedStatus == 0)
        {
            ON_CHIP_LED_ON;
            LedStatus = 1;
        }
    }
    else
    {
        if(RecState == REC_BIND)
        {
            if(StartBindBlinkTime == 0)
            {
                StartBindBlinkTime = ActionTick;
            }
            
            if(ActionTick > LastFlickTime + BIND_STATUS_LED_FLICKER_INTERVAL)
            {
                LastFlickTime = ActionTick;
                if(LedStatus == 0)
                {
                    LedStatus = 1;
                    ON_CHIP_LED_ON;
                }
                else
                {
                    LedStatus = 0;
                    ON_CHIP_LED_OFF;
                }
            }
        }
        else if(RecState == REC_DISCONNECTING || RecState == REC_SEARCHING)
        {
            LedStatus = 0;
            ON_CHIP_LED_OFF;
        }
        else
        {
            //REC_RECEIVING
            if((StartBindBlinkTime != 0) && (ActionTick < StartBindBlinkTime + 1000))
            {
                if(ActionTick > LastFlickTime + BIND_STATUS_LED_FLICKER_INTERVAL)
                {
                    //BIND×´Ì¬ÉÁË¸Âú1Ãë
                    LastFlickTime = ActionTick;
                    if(LedStatus == 0)
                    {
                        LedStatus = 1;
                        ON_CHIP_LED_ON;
                    }
                    else
                    {
                        LedStatus = 0;
                        ON_CHIP_LED_OFF;
                    }
                }
            }
            else
            {
                //if(LedStatus == 0)
                {
                    LedStatus = 1;
                    ON_CHIP_LED_ON;
                }
            }
        }
    }
}


