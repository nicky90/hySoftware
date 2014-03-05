#ifndef __ADAPTER_H
#define __ADAPTER_H


typedef struct __YKMsg_Head {
    int       msgLevel;
    int       msgType;
    short int jobNum;
    short int satNum;
    int       orbNum;
    short int msgFrom;
    short int msgTo;
    short int time[7];
    int       packetLen;
} YKMsg_Head;

typedef struct __Msg_to_YK {
    YKMsg_Head msgHead;
    int        stateCode;
    short int  paramNum;
    char       params[10][150];
} MsgToYK;

typedef struct __Msg_from_YK {i
    YKMsg_Head msgHead;
    int        argc;
    char       msg[8][200];
} MsgFromYK;

/* return 0 if filelist fetched successful or return -1 */
int h2a_fetch_filelist(const char *filelist);
/* return 0 if all files in filelist exist and size right, or return -1 */
int h2a_check_filelist(const char *filelist);

#endif
