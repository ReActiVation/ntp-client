#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif


#define NTP_TIMESTAMP_DELTA 2208988800ull
#define h_addr h_addr_list[0] /* hostent backward compatibility */

#define LI(packet) (uint8_t) ((packet.li_vn_mode & 0xC0) >> 6) // (li   & 11 000 000) >> 6
#define VN(packet) (uint8_t) ((packet.li_vn_mode & 0x38) >> 3) // (vn   & 00 111 000) >> 3
#define MODE(packet) (uint8_t) ((packet.li_vn_mode & 0x07) >> 0) // (mode & 00 000 111) >> 0

//Throw error message function
void error(char * message)
{
    perror(message);

    exit(0);
}

int main(int argc, char* argv[ ])
{
    int socket_file_descriptor, n; //Socket file descriptor and the n return result from writing/reading from the socket.
    
    int port_number = 123; //NTP UDP port number.

    char* host_name = "ru.pool.ntp.org"; //NTP-server hots name.

    //Structure that defines 48 bytes ntp packet.
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

    //Create and zero out the packet. All 48 bytes worth.
    ntp_packet packet = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    memset( &packet, 0, sizeof( ntp_packet ));

    // Set the first byte's bits to 00,011,011 for li = 0, vn = 3, and mode = 3. The rest will be left set to zero.
    *( ( char * ) &packet + 0) = 0x1b; // Represents 27 in base 10 or 00011011 in base 2.

    //Create a UDP socket, convert the host-name to an IP address, set port number,
    //connect to the server, send the packet, and then read the return packet.

    struct sockaddr_in server_address; //Server address data structure.
    struct hostent *server; //Server data structure.

    socket_file_descriptor = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); //Create UDP socket.

    if(socket_file_descriptor < 0)
        error("ERROR opening socket");
    
    server = gethostbyname(host_name); //Convert URL to IP.

    if(server == NULL)
        error("ERROR not such host");
    
    //Zero out the server address structure.
    bzero((char*) &server_address, sizeof(server_address));

    server_address.sin_family = AF_INET;

    //Copying the server's IP address to the server address structure.
    bcopy((char*)server->h_addr, (char*)&server_address.sin_addr.s_addr, server->h_length );

    //Convert the port number integer to network big-endian style and save it to the server address structure.
    server_address.sin_port = htons(port_number);

    //Call up the server using IP address and port number.
    if(connect(socket_file_descriptor, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
        error("ERROR connecting");
    
    //Send it the NTP packet it wants. If n == -1, it failed.
    n = write(socket_file_descriptor, (char*) &packet, sizeof(ntp_packet));

    if(n < 0)
        error("ERROR writing to socket");

    //Wait and receive the packet back from the server. If n == -1, it failed.
    n = read(socket_file_descriptor, (char *) &packet, sizeof(ntp_packet));

    if(n < 0)
        error("ERROR reading from socket");

    //These two fields contain the time-stamp seconds as the packet left NTP server.
    //The number of seconds correspond to the seconds passed since 1900.
    //ntohl() converts the bit/byte order from the network's to host's "endianness".

    packet.txTm_s = ntohl(packet.txTm_s); //Time-stamp seconds.
    packet.txTm_f = ntohl(packet.txTm_f); //Time-stamp fraction of a second.

    // Extract the 32 bits that represent the time-stamp seconds (since NTP epoch) from when the packet left the server.
    // Subtract 70 years worth of seconds from the seconds since 1900.
    // This leaves the seconds since the UNIX epoch of 1970.
    // (1900)------------------(1970)**************************************(Time Packet Left the Server)

    time_t txTm = ( time_t ) ( packet.txTm_s - NTP_TIMESTAMP_DELTA );

    // Print the time we got from the server, accounting for local timezone and conversion from UTC time.

    printf( "Time: %s", ctime( ( const time_t* ) &txTm ) );

    return 0;
}