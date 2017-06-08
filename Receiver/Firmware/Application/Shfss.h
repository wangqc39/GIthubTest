#ifndef __SHFSS__
#define __SHFSS__

typedef enum {REC_SEARCHING = 0, REC_RECEIVING = 1, REC_BIND = 2, REC_DISCONNECTING = 3} RecStateInfo;


extern RecStateInfo RecState;
extern unsigned long ShfssConnectLastPacketTime;


void ShfssInit(void);
void ForceHopChannel(void);
void ShfssBindInit(void);
void CheckAndSaveBindId(void);

#endif

