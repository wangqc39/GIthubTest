#include "main.h"

#define CS_PORT			GPIOC
#define CS_PIN			GPIO_PIN_4

#define CS_HI() 	GPIO_WriteHigh(CS_PORT, CS_PIN)//CS_PORT->ODR |= (uint8_t)CS_PIN;//GPIO_WriteHigh(GPIOE, GPIO_PIN_5)//gpio_set(GPIOB, GPIO12)   
#define CS_LO() 	GPIO_WriteLow(CS_PORT, CS_PIN)//CS_PORT->ODR &= (uint8_t)(~CS_PIN);//GPIO_WriteLow(GPIOE, GPIO_PIN_5)//gpio_clear(GPIOB, GPIO12)
#define PROTOSPI_xfer(byte) spi_xfer(byte)


  
void AmpControl()
{
    GPIO_Init(GPIOA, GPIO_PIN_2, GPIO_MODE_OUT_PP_LOW_SLOW);//TX
    GPIO_Init(GPIOA, GPIO_PIN_3, GPIO_MODE_IN_PU_NO_IT);
}

void CC2500SpiInit()
{
    GPIO_Init(CS_PORT, CS_PIN, GPIO_MODE_OUT_PP_HIGH_FAST);
    SPI_Init(SPI_FIRSTBIT_MSB, SPI_BAUDRATEPRESCALER_16, SPI_MODE_MASTER, SPI_CLOCKPOLARITY_HIGH, SPI_CLOCKPHASE_2EDGE,
                   SPI_DATADIRECTION_2LINES_FULLDUPLEX, SPI_NSS_SOFT, 7);
    SPI_Cmd(ENABLE);


    AmpControl();
}

unsigned char spi_xfer(u8 data)
{
    //SPI_SendData(data);
    SPI->DR = data;

    //while((SPI->SR & (uint8_t)SPI_FLAG_RXNE) == RESET);
    while(!(SPI->SR & (uint8_t)SPI_FLAG_RXNE));

    return SPI->DR;
}

void CC2500_Strobe(u8 state)
{
    CS_LO();
    PROTOSPI_xfer(state);
    CS_HI();
}


void CC2500_ReadRegisterMulti(u8 address, u8 data[], u8 length)
{
    unsigned char i;

    CS_LO();
    PROTOSPI_xfer(CC2500_READ_BURST | address);
    for(i = 0; i < length; i++)
    {
        data[i] = PROTOSPI_xfer(0);
    }
    CS_HI();
}

void CC2500_WriteRegisterMulti(u8 address, const u8 data[], u8 length)
{
    int i;
    CS_LO();
    PROTOSPI_xfer(CC2500_WRITE_BURST | address);
    for(i = 0; i < length; i++)
    {
        PROTOSPI_xfer(data[i]);
    }
    CS_HI();
}

void CC2500_ReadData(u8 *dpbuffer, int len)
{
    CC2500_ReadRegisterMulti(CC2500_3F_RXFIFO, dpbuffer, len);
}

void CC2500_WriteData(u8 *dpbuffer, u8 len)
{
    CC2500_Strobe(CC2500_SFTX);
    CC2500_WriteRegisterMulti(CC2500_3F_TXFIFO, dpbuffer, len);
    CC2500_Strobe(CC2500_STX);
}

void CC2500_WriteReg(u8 address, u8 data)
{
    CS_LO();
    PROTOSPI_xfer(address);
    PROTOSPI_xfer(data);
    CS_HI();
}

u8 CC2500_ReadReg(u8 address)
{
    u8 data;
    CS_LO();
    PROTOSPI_xfer(CC2500_READ_SINGLE | address);
    data = PROTOSPI_xfer(0);
    CS_HI();
    return data;
}

void CC2500_SetPower(int power)
{
    const unsigned char patable[8]=
    {
        0xC6,  // -12dbm
        0x97, // -10dbm
        0x6E, // -8dbm
        0x7F, // -6dbm
        0xA9, // -4dbm
        0xBB, // -2dbm
        0xFE, // 0dbm
        0xFF // 1.5dbm
    };
    if (power > 7)
        power = 7;
    CC2500_WriteReg(CC2500_3E_PATABLE,  patable[power]);
}

void CC2500_SetTxRxMode(enum TXRX_State mode)
{
    // config-cc2500 = 0x01 for swapping GDO0 and GDO2.
    int R0 = CC2500_02_IOCFG0;
    int R2 = CC2500_00_IOCFG2;
    /*if (Transmitter.module_config[CC2500] == CC2500_REVERSE_GD02) {
      R0 = CC2500_00_IOCFG2;
      R2 = CC2500_02_IOCFG0;
    }*/

    if(mode == TX_EN) {
        CC2500_WriteReg(R2, 0x2F);
        CC2500_WriteReg(R0, 0x2F | 0x40);
    } else if (mode == RX_EN) {
        CC2500_WriteReg(R0, 0x2F);
        CC2500_WriteReg(R2, 0x2F | 0x40);
    } else {
        CC2500_WriteReg(R0, 0x2F);
        CC2500_WriteReg(R2, 0x2F);
    }
}

int CC2500_Reset()
{
    CC2500_Strobe(CC2500_SRES);
    usleep2us(500);
    CC2500_SetTxRxMode(TXRX_OFF);
    return CC2500_ReadReg(CC2500_0E_FREQ1) == 0xC4;
}

unsigned char CC2500_GetRxCnt()
{
    unsigned char DataCnt;
    CC2500_ReadRegisterMulti(CC2500_3B_RXBYTES, &DataCnt, 1);
    return DataCnt;
}

unsigned char CC2500_GetRssi()
{
    unsigned char DataCnt;
    CC2500_ReadRegisterMulti(CC2500_34_RSSI, &DataCnt, 1);
    return DataCnt;
}

unsigned char CC2500_GetState()
{
    unsigned char DataCnt;
    CC2500_ReadRegisterMulti(CC2500_35_MARCSTATE, &DataCnt, 1);
    return DataCnt;
}


