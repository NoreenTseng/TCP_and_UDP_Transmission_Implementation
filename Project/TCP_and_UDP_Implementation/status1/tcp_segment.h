#include <stdio.h>	//c library
#include <stdlib.h>
#include <iostream>	//c++ library
#include <sstream>
#include <sys/socket.h>	//socket
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>	//string handler
#include <time.h>
#include <pthread.h>
#include <algorithm>
#include <random>
#include <iostream>
#include <iomanip>
#include <map>
using namespace std;

/***** Set the parameters *******/
#define Kb 1024
#define RTT 15
#define Threshold 64*Kb
#define MSS 1*Kb
#define RecvBffSize 512*Kb

/***** Set the protocol and TCP segment header *******/
typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct tcp_segment_header{
    uint16_t src_port;	//source port
    uint16_t dst_port;	//destination port
    uint16_t rwnd;		//receiver window size
    uint16_t checksum;	//checksum
    uint16_t flag;		//U, A, P, R, S, F
    uint16_t offset;	//last bit
    uint32_t sequence_number;//segment sequence number
    uint32_t ack_number;	//ACK number
};

/***** Set segment *******/
typedef struct segment{
	tcp_segment_header tcp_header;
	char data_buffer[MSS];	//data
};
/*

flag setting: http://www.pcnet.idv.tw/pcnet/network/network_ip_tcp.htm
U, A, P, R, S, F:1280
*/
