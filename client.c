#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <stdint.h>

#ifdef __WIN32__
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

typedef struct
{
    uint8_t li_vn_mode;     // 8-bits. Li, vn and mode.
                            //Li. 2-bits. Leap indicator.
                            //Vn. 3-bits. Version number of the protocol.
                            //Mode. 3-bits. Client will pick mode 3 for client.
    
    uint8_t stratum;        //8-bits. Stratum level of the local clock.
    uint8_t poll;           //8-bits. Maximum interval between successive messages.
    uint8_t precision;      //8-bits. Precision of the local clock.

    uint32_t rootDelay;     //32-bits. Total round trip delay time.
    uint32_t rootDispersion;//32-bits. Max error aloud from primary clock source.
    uint32_t refId;         //32-bits. Reference clock identifier.
    
    uint32_t refTm_s;       //32-bits. Reference time-stamp seconds.
    uint32_t refTm_f;       //32-bits. Reference time-stamp fraction of a second.

    uint32_t origTm_s;      //32-bits. Originate time-stamp seconds.
    uint32_t origTm_f;      //32-bits. Originate time-stamp fraction of a second.

    uint32_t rxTm_s;        //32-bits. Received time-stamp seconds.
    uint32_t rxTm_f;        //32-bits. Received time-stamp fraction of a second.

    uint32_t txTm_s;        //32-bits and the most important field the client cares about. Transmit time-stamp seconds.
    uint32_t txTm_f;        //32-bits. Transmit time-stamp fraction of a second.
} ntp_packet;               //Total: 384 bits or 48 bytes.

int main()
{

}