#include "main.h"
#include "string.h"

enum {
    SFHSS_START = 0x101,
    SFHSS_CAL   = 0x102,
    SFHSS_TUNE  = 0x103,
    SFHSS_DATA1 = 0x02,
    SFHSS_DATA2 = 0x05
};

typedef enum {INT_NORMAL = 0, INT_FORCE_HOP = 1, INT_AFTER_HOP = 2} IntStateInfo;


#define RC_TIMEOUT			200
#define SHFSS_PACKET_SIZE			11

unsigned short int RcData[4];
RecStateInfo RecState;
u8 rf_chan =  0;
unsigned char TxFhssCode;
unsigned char TxId[2] = {0xFF, 0xFF};
unsigned char stat;
unsigned long ShfssConnectLastPacketTime;
unsigned char Channel;
IntStateInfo IntState;
unsigned long DisconnectCnt;
unsigned long ForceHopCnt;
short int RSSI;

unsigned short int TxIdSaved;

unsigned short int ShfssSearchData(void);


void SetRcData()
{
    ThPwm = RcData[0] + 2000;
    StPwm = RcData[1] + 2000;
}

void SetSavedTxId()
{
    TxIdSaved = (unsigned int)TxId[0];
    TxIdSaved |= ((unsigned int)TxId[1] << 8);
}

void GetShfssTxConfig()
{
/*for(i = 0; i < 2; i++)
    {
        TxId[i] = *(unsigned char *)(FLASH_DATA_START_PHYSICAL_ADDRESS + i);
    }*/
    memcpy(TxId, (void const *)FLASH_DATA_START_PHYSICAL_ADDRESS, 2);
    SetSavedTxId();
}

void CheckAndSaveBindId()
{
    unsigned short int NowTxId;
    NowTxId = (unsigned int)TxId[0];
    NowTxId |= ((unsigned int)TxId[1] << 8);
    if(NowTxId != TxIdSaved)
    {
        FLASH_Unlock(FLASH_MEMTYPE_DATA);
        memcpy((void *)(FLASH_DATA_START_PHYSICAL_ADDRESS), (void const *)TxId, 2);
        FLASH_Lock(FLASH_MEMTYPE_DATA);
        SetSavedTxId();
    }
}

static void calc_next_chan()
{
    rf_chan += TxFhssCode + 2;
    if (rf_chan > 29) 
    {
        if (rf_chan < 31) 
        {
            rf_chan += TxFhssCode + 2;
        }
        rf_chan -= 31;
    }

    Channel = rf_chan * 5;
}

void TuneChan(unsigned char Channel)
{ 
    CC2500_Strobe(CC2500_SIDLE);
    CC2500_Strobe(CC2500_SFRX);
    CC2500_WriteReg(CC2500_0A_CHANNR, Channel);
    CC2500_Strobe(CC2500_SCAL);
    //uDelay(850);
}

void TuneChan0()
{
    Channel = 0;
    TuneChan(Channel); 
}

static void tune_chan()
{
    calc_next_chan();
    TuneChan(Channel); 
}

void StartRecData()
{
    CC2500_Strobe(CC2500_SIDLE);
    CC2500_Strobe(CC2500_SFRX);
    CC2500_Strobe(CC2500_SRX);
    CC2500_Strobe(CC2500_SRX);
}

void ShfssInit()
{
    unsigned char data;
    CC2500_Strobe(CC2500_SRES);
    CC2500_WriteReg(0x0A, 0x00);
    CC2500_WriteReg(0x00, 0x06);
    CC2500_WriteReg(0x01, 0x2E);
    CC2500_WriteReg(0x02, 0x32);
    CC2500_WriteReg(0x03, 0x07);
    CC2500_WriteReg(0x04, 0xD3);
    CC2500_WriteReg(0x05, 0x91);
    CC2500_WriteReg(0x06, SHFSS_PACKET_SIZE);
    CC2500_WriteReg(0x07, 0x0C);
    CC2500_WriteReg(0x09, 0x80);
    CC2500_WriteReg(0x0B, 0x1F);
    CC2500_WriteReg(0x0C, 0x00);
    CC2500_WriteReg(0x10, 0x7C);
    //CC2500_WriteReg(0x10, 0x7B);
    CC2500_WriteReg(0x11, 0x43);
    CC2500_WriteReg(0x12, 0x83);
    CC2500_WriteReg(0x13, 0x23);
    CC2500_WriteReg(0x14, 0x7A);
    CC2500_WriteReg(0x15, 0x44);
    CC2500_WriteReg(0x16, 0x07);
    CC2500_WriteReg(0x17, 0x3C);
    CC2500_WriteReg(0x18, 0x08);
    CC2500_WriteReg(0x19, 0x1D);
    CC2500_WriteReg(0x1A, 0x1C);
    CC2500_WriteReg(0x1B, 0xC7);
    CC2500_WriteReg(0x1C, 0x00);
    CC2500_WriteReg(0x1D, 0xB0);
    CC2500_WriteReg(0x1E, 0x57);
    CC2500_WriteReg(0x1F, 0x6B);
    CC2500_WriteReg(0x20, 0xF8);
    CC2500_WriteReg(0x21, 0xB6);
    CC2500_WriteReg(0x22, 0x10);
    CC2500_WriteReg(0x23, 0xEA);
    CC2500_WriteReg(0x24, 0x0A);
    CC2500_WriteReg(0x25, 0x17);
    CC2500_WriteReg(0x26, 0x11);
    CC2500_WriteReg(0x2C, 0x88);
    CC2500_WriteReg(0x2D, 0x31);
    CC2500_WriteReg(0x2E, 0x0B);
    CC2500_WriteReg(0x08, 0x0C);

    CC2500_Strobe(CC2500_SIDLE);
    CC2500_WriteReg(0x0D, 0x5C);
    CC2500_WriteReg(0x0E, 0x76);
    CC2500_WriteReg(0x0F, 0x2E);
    CC2500_WriteReg(CC2500_3E_PATABLE,  0xAE);
    CC2500_Strobe(CC2500_SIDLE);
    CC2500_Strobe(CC2500_SFRX);
    CC2500_Strobe(CC2500_SCAL);
    CC2500_ReadRegisterMulti(CC2500_35_MARCSTATE, &data, 1);
    CC2500_Strobe(CC2500_SIDLE);
    CC2500_WriteReg(0x09, 0x00);
    CC2500_Strobe(CC2500_SRX);
    CC2500_Strobe(CC2500_SRX);

    GetShfssTxConfig();
    RecState = REC_SEARCHING;

    TuneChan0();
    mSleep(2);
    StartRecData();

    CallBackTimerStart(30, ShfssSearchData);


    //StartRecData();

    //ShfssSearchInit();
}


#define spacer1 0x02 //0b10
#define spacer2 (spacer1 << 4)
int AnalysisPacket(unsigned char *buff)
{
    unsigned char check = 0;
    int i;
    if(buff[0] != 0x81)
        return -1;

    for(i = 0; i < SHFSS_PACKET_SIZE - 1; i++)
    {
        check += buff[i];
    }
    if(buff[10] != check)
    {
        return -8;
    }

    if(RecState != REC_BIND)
    {
        if(buff[1] != TxId[0] || buff[2] != TxId[1])
            return -2;
    }

    if((buff[3] & spacer1) == 0)
        return -4;
    if((buff[5] & spacer2) == 0)
        return -5;
    if((buff[6] & spacer1) == 0)
        return -6;
    if((buff[8] & spacer2) == 0)
        return -7;
    

    stat = buff[9] & 0x7;
    if(stat != SFHSS_DATA1 && stat != SFHSS_DATA2)
        return -6;

    rf_chan = buff[3] >> 3;
    TxFhssCode = buff[9] >> 3;
    
    return 0;
}

void AnalysisRcData(unsigned char *buff)
{
    RcData[0] = ((buff[3] & 1) << 10) | (buff[4] << 2) | (buff[5] >> 6);
    RcData[1] = ((buff[5] & 0x1F) << 6) | (buff[6] >> 2);
    RcData[2] = ((buff[6] & 1) << 10) | (buff[7] << 2) | (buff[8] >> 6);
    RcData[3] = buff[8] & 0x1F;

    SetRcData();

}

#define RSSI_OFFSET			72
void CalRssi()
{
    unsigned char ReadRssi;
    ReadRssi = CC2500_GetRssi();
    if(ReadRssi >= 128)
    {
        RSSI = ((ReadRssi - 256) >> 1) - RSSI_OFFSET;
    }
    else
    {
        RSSI = (ReadRssi >> 1) - RSSI_OFFSET;
    }
}

unsigned char RfStatus;
void GetStatus()
{
    CC2500_ReadRegisterMulti(CC2500_35_MARCSTATE, &RfStatus, 1);
}

unsigned char buff[SHFSS_PACKET_SIZE];
unsigned long PacketCnt;
unsigned long PacketErrorCnt;
unsigned long LostPacketCnt;
unsigned short int ShfssSearchData()
{
    unsigned char DataCnt;
    int PacketRet;

    if(IntState == INT_NORMAL)
    {
        GetStatus();
        DataCnt = CC2500_GetRxCnt();
        if(DataCnt == 5)
        {
            CalRssi();
        }
        if(DataCnt >= SHFSS_PACKET_SIZE)
        {
            CC2500_ReadData(buff, SHFSS_PACKET_SIZE);
            PacketRet = AnalysisPacket(buff);
            if(PacketRet == 0)
            {
                
                if(RecState == REC_SEARCHING || RecState == REC_DISCONNECTING)
                {
                    RecState = REC_RECEIVING;
                }
                else if(RecState == REC_BIND)
                {
                    RecState = REC_RECEIVING;
                    TxId[0] = buff[1];
                    TxId[1] = buff[2];
                }
                PacketCnt++;

                AnalysisRcData(buff);
    
                ShfssConnectLastPacketTime = ActionTick;
                if(stat == SFHSS_DATA1)
                {
                    StartForceHopTime(1650 + 500);
                    StartRecData();
                    return 500;
                }
                else
                {
                    StartForceHopTime(6800 + 500);
                    tune_chan();
                    IntState = INT_AFTER_HOP;
                    //return 1200;
                    return 1000;
                }
            }
            else
            {
                PacketErrorCnt++;
            }
        }
        return 400;//return 200;
    }
    else if(IntState == INT_AFTER_HOP)
    {
        StartRecData();
        IntState = INT_NORMAL;
        return 300;//return 100;
    }
    else
    {
        if(ActionTick >= ShfssConnectLastPacketTime + RC_TIMEOUT)
        {
            DisconnectCnt++; 
            StopForceHopTime();
            TuneChan0(); 
            IntState = INT_AFTER_HOP;
           
            RecState = REC_DISCONNECTING;
            return 1000;
        }
        else
        {
            SetForceHopTimeByLastForce(6800);
            tune_chan();
            IntState = INT_AFTER_HOP;
            
            return 1000;
        }
    }
}

void ForceHopChannel()
{
    ForceHopCnt++;
    IntState = INT_FORCE_HOP;
}

void ShfssBindInit()
{
    RecState = REC_BIND;
    StopForceHopTime();
    CallBackTimerStop();
    

    TuneChan0();
    mSleep(2);
    StartRecData();
    
    CallBackTimerStart(100, ShfssSearchData);
}


void ShfssSearchInit()
{
    RecState = REC_SEARCHING;
    StopForceHopTime();
    CallBackTimerStop();
    

    TuneChan0();
    mSleep(2);
    StartRecData();
    
    CallBackTimerStart(100, ShfssSearchData);
}



