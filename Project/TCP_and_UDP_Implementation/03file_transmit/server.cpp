/* 
** Step 1:
** Set and print the arguments.
** Three way handshake.((another))
** One server to one client.
** One server to four or more client, also, record their index.
*/

#include "tcp_segment.h"

/*** Variables ****/
int sock;
sockaddr_in server;	//server info
sockaddr_in client;	//client info
segment recvData;	//receive data pkt
segment sendData;	//send data pkt
segment handShake_seg = {0,0,0,0,0,0,0,0,0};	//initialize hand shake segment
segment clientHand;
int nbytes;	//transfered buffer size with bytes
int nlength;	//buffer length
stringstream ss;

/*** Function variables****/
//int Handshake(int);
void Show();
void sendPkt(uint16_t,uint16_t,uint32_t,uint32_t,uint16_t,uint16_t,uint16_t,uint16_t,char*);

int main(int argc, char **argv){
	/******** Variables **********/
	int tempBytes;
	int tempPort = 3456;
	/**** Build socket ****/
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(sock < 0)
	{
		perror("Socket error.\n");
		exit(1);	
	}
	/**** server's info ****/
	bzero((char*) &server, sizeof(server));	//initialize the sockaddr_in
	server.sin_family = AF_INET;
	
	//inet_aton(argv[1], &server.sin_addr);	//set IP address
	//server.sin_port = htons(atoi(argv[2]));	//set port number
	//char* serverip = argv[1];
	/**** client's info ****/
	bzero((char*) &client, sizeof(client));	//initialize the sockaddr_in
	client.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(tempPort);
	//inet_aton(argv[1], &client.sin_addr);	//set IP address
	//client.sin_port = htons(atoi(argv[2]));	//set port number
	nlength = sizeof(client);
	/**** bind() ****/
	if(bind(sock, (sockaddr* )&server, sizeof(server)) < 0)
    {
        perror("Bind error.\n");
        exit(1);
    }
    /******* Show the parameter**********/
    printf("-----------Parameter-------------\n");
    printf("Port number : %d\n", ntohs(server.sin_port));
    printf("The RTT delay : %d ms\n", RTT);
    printf("Threshold : %d bytes\n", Threshold);
    printf("MSS : %d bytes\n", MSS);
    printf("Receive buffer size : %d bytes\n", RecvBffSize);
    printf("Server's IP address : %s\n", inet_ntoa(server.sin_addr));
    printf("Server's port number : %d\n", ntohs(server.sin_port));
    /******* Three-way handshake **********/
	/*printf("-----------Three-way handshake-------------\n");
	handShake_seg.tcp_header.src_port = atoi(argv[2]);
	handShake_seg.tcp_header.sequence_number = (rand()%100000)+1;
	handShake_seg.tcp_header.ack_number = 0;
	//no option field
    handShake_seg.tcp_header.flag = 20498; 
    handShake_seg.tcp_header.rwnd = 0;
    handShake_seg.tcp_header.checksum = 0;
    handShake_seg.tcp_header.offset = 0;
    memset(handShake_seg.data_buffer,'\0',512);
    //read handshake data from client
	if((nbytes = recvfrom(sock, &clientHand, sizeof(clientHand), 0, (sockaddr*)&client, (socklen_t*)&nlength)) < 0)
    {
        perror("Receive data error\n");
        exit(1);
    }
    //set flags to change mode
    if((clientHand.tcp_header.flag & 0x3f) == 2)
    {
        printf("Receive a packet(SYN) from %s : %d\n",inet_ntoa(client.sin_addr),htons(client.sin_port));
        printf("        Receive a packet (seq_num = %d, ack_num = %d)\n",clientHand.tcp_header.sequence_number,clientHand.tcp_header.ack_number);
    }
    handShake_seg.tcp_header.ack_number = clientHand.tcp_header.sequence_number + 1;
    //sand handshake data to client
    if(sendto(sock, &handShake_seg, sizeof(handShake_seg), 0, (struct sockaddr *)&client, nlength) < 0)
    {
        perror("Send data error\n");
        exit(1);
    }
    printf("Send a packet(SYN/ACK) to %s : %d\n",inet_ntoa(client.sin_addr),htons(client.sin_port));
    //ACK from client
    if((nbytes = recvfrom(sock, &clientHand, sizeof(clientHand), 0, (sockaddr* )&client, (socklen_t* )&nlength)) < 0)
    {
        perror("Receive data error\n");
        exit(1);
    }
	//success to receive ACK
    if((clientHand.tcp_header.flag & 0x3f) == 16)
    {
        printf("Receive a packet(ACK) from %s : %d\n",inet_ntoa(client.sin_addr),htons(client.sin_port));
        printf("        Receive a packet (seq_num = %d, ack_num = %d)\n", clientHand.tcp_header.sequence_number, clientHand.tcp_header.ack_number);
    }
    printf("-----------Three-way handshake finish successfully-------------\n");
	*/
	/************ Data transmit ****************/
	printf("-----------Start to transmit Data-------------\n");
	for(int i = 0; i < 30; i++)
	{
		//receive from client
		if(recvfrom(sock, &recvData, MSS, 0, (sockaddr* )&client, (socklen_t* ) &nlength) < 0 )
		{              
				perror("Receive data error\n");
				return 0;
		}
		printf("client index : %d\n", i+1);
		Show();
		//connect ok with client, then send data
		if(strcmp(recvData.data_buffer,"ConnectOK") == 0) 
		{
			//dataPort is the port to transmit data with clients' order
			int dataPort = tempPort + 1 + i*2;
			char tempBytes[8];
			//string stream hanndler
			ss << dataPort; ss >> tempBytes; ss.str(""); ss.clear();
			//the pkt ready for sending
			sendPkt(recvData.tcp_header.dst_port, client.sin_port, 1, 0, 1298,  recvData.tcp_header.rwnd, 0, 0, tempBytes);
			//send dataPort :sendto()
			if(sendto(sock, &sendData, MSS, 0, (sockaddr* )&client, nlength) < 0) 
			{
				perror("Send data error\n");
				return 0;
			}
			//Create threads
			int pkthread = fork();
			if(pkthread == 0)
			{
				sockaddr_in serverKID;
				int sockKID = socket(AF_INET,SOCK_DGRAM,0);
				//build kid socket
				if(sockKID < 0)
				{
					perror("KID socket error.\n");
					return 0;
				}
				//initialize and create ip address and port number
				bzero((char* )&serverKID, sizeof(serverKID));
				serverKID.sin_family = AF_INET;
				serverKID.sin_addr.s_addr = htonl(INADDR_ANY);
				serverKID.sin_port = htons(dataPort);
				//start bind()
				if(bind(sockKID, (sockaddr*)&serverKID, sizeof(serverKID)) < 0)
				{
					printf("KID bind error\n");
					exit(1);
				}
				printf("KID socket created at port : %d\n", dataPort);
				if(recvfrom(sockKID, &recvData ,MSS, 0,(sockaddr* )&client , (socklen_t* ) &nlength) < 0) 
				{
					perror("KID receive error.\n");
					return 0;
				}
				Show();
				//open and read the file mp4 that client asked for
				FILE* filePtr;
				filePtr = fopen(recvData.data_buffer, "rb");
				if(!filePtr)
				{
					printf("Open data error\n");
					exit(1);
				}
				//send the parameters with open mode
				sendData.tcp_header.sequence_number = 1;
				sendData.tcp_header.flag = 1280;
				sendData.tcp_header.rwnd = 1024;
				sendData.tcp_header.offset = (MSS-20);
				//read in data(determine:whether eof)
				while(!feof(filePtr))
				{
					fread(sendData.data_buffer, MSS-20, 1, filePtr);
					//print data pointer pointing to 
					printf("Data pointer to %ld ",ftell(filePtr));
					printf("with data buffer : %X\n", sendData.data_buffer[0]);
					//send data finish
					if(feof(filePtr))
					{
						sendData.tcp_header.flag = 1281;
						sendData.tcp_header.offset = ftell(filePtr)%(MSS-20);
						printf("----------- Send data done -----------\n");
					}
					
					if(sendto(sockKID, &sendData, MSS, 0, (sockaddr* )&client, nlength) < 0) 
					{
						perror("Send data error.\n");
						exit(1); 
					} 
					//send pkt  
					printf("Send Pkt : %d\n", sendData.tcp_header.sequence_number);
					sendData.tcp_header.sequence_number++;
					//1ms = 1000us
					usleep(1000);
				}
				//close filePtr
				fclose(filePtr);
				exit(0);
			}
		}
	}
	return 0;
}

void Show(){
	printf("source port : %d\n", client.sin_port);
	printf("destination port : %d\n", recvData.tcp_header.dst_port);
	printf("Sequence number : %d\n", recvData.tcp_header.sequence_number);
	printf("ACK number : %d\n", recvData.tcp_header.ack_number);
	printf("Transmit data : %s\n", recvData.data_buffer);
}
void sendPkt(uint16_t srcport,uint16_t dstport,uint32_t seqN,uint32_t ackN,uint16_t fflag,uint16_t recvwnd,uint16_t check,uint16_t offsetbit,char* buf){
	sendData.tcp_header.src_port = srcport;
	sendData.tcp_header.dst_port = dstport;
	sendData.tcp_header.sequence_number = seqN++;
	sendData.tcp_header.ack_number = ackN++;
	sendData.tcp_header.checksum = check;
	sendData.tcp_header.flag = fflag;
	sendData.tcp_header.rwnd = recvwnd;
	sendData.tcp_header.offset = offsetbit;
	strcpy(sendData.data_buffer, buf);
}
