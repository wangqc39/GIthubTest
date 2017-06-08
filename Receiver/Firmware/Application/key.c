#include "main.h"

#define GET_KEY			GPIO_ReadInputPin(GPIOD, GPIO_PIN_6)

void KeyHwInit()
{
    GPIO_Init(GPIOD, GPIO_PIN_6, GPIO_MODE_IN_PU_NO_IT);
}

unsigned long LastKeyUpTime;
void KeyHandler()
{
    if(GET_KEY != RESET)
    {
        LastKeyUpTime = ActionTick;
    }
    else
    {
        //°´¼ü°´ÏÂ
        if(ActionTick > LastKeyUpTime + 500 && RecState != REC_BIND)
        {
            ShfssBindInit();
            LastKeyUpTime = ActionTick;
        }
    }
}

