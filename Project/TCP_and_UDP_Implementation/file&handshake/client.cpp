#include "tcp_segment.h"

/*** Variables ****/
int sock;
sockaddr_in server;	//server info
sockaddr_in client;	//client info
segment recvData;	//receive data pkt
segment sendData;	//send data pkt
struct hostent *hostip;	//for gethostip name
segment handShake_seg = {0,0,0,0,0,0,0,0,0};
segment serverHand;
int handshakebytes;	//transfered buffer size with bytes
char handshakeBuffer[1024];
stringstream ss;
/*** Function variables****/
int Handshake(int);
void Show();

int main(int argc, char **argv){
	/*** Temp client variables ***/
	uint16_t temp_cport;
	int temp_cseq = 1;
	int temp_cack = 100;
	int temp_crwnd = 1;
	char fileName[32];
	/**** server's info ****/
	bzero((char*) &server, sizeof(server));
	server.sin_family = AF_INET;
	hostip = gethostbyname(argv[1]);
    char *srvip = argv[1];
    server.sin_port = htons(atoi(argv[2]));
    int server_length = sizeof(server);
	/***** Client info Variables *****/
	int client_leng = sizeof(client);
	bzero((char*)&client,sizeof(client));
	client.sin_family = AF_INET;
	client.sin_port = htons(3456);
	struct hostent *hostip = gethostbyname("127.0.0.1");
	//build socket
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock < 0)
	{
		perror("Socket error.\n");
		exit(1);	
	}
	if(hostip == 0) 
	{
		perror("Hostip error\n");
		exit(1);
	}
	//copy the network address to sockaddr_in structure
	bcopy(hostip -> h_addr_list[0], (caddr_t)&client.sin_addr, hostip -> h_length);
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
    printf("-----------Three-way handshake-------------\n");
    handShake_seg.tcp_header.src_port = htons(3456);
    handShake_seg.tcp_header.dst_port = atoi(argv[2]);
    handShake_seg.tcp_header.sequence_number = (rand()%100000)+1;
    handShake_seg.tcp_header.ack_number = 0;
    //no option field
    handShake_seg.tcp_header.flag = 20482; 
    handShake_seg.tcp_header.rwnd = 0;
    handShake_seg.tcp_header.checksum = 0;
    //initialize handshakeBuffer
    handshakeBuffer[handshakebytes-1] = '\0';
    //send request
    if(sendto(sock, &handShake_seg, sizeof(handShake_seg), 0, (struct sockaddr*) &server, server_length) < 0)
    {
        perror("write to server error!!\n");
        exit(1);
    }
    printf("Send a packet(SYN) to %s : %d\n",srvip,htons(server.sin_port));
	//receive permission
    if(recvfrom(sock, &serverHand, sizeof(serverHand), 0, (struct sockaddr* ) &server,(socklen_t* ) &server_length) < 0)
    {
        perror("Receive data error.\n");
        exit(1);
    }
	//flag setting:receive ok
    if((serverHand.tcp_header.flag & 0x3f) == 18)
    {
        printf("Receive a packet(SYN/ACK) from %s : %d\n",srvip,htons(server.sin_port));
        printf("        Receive a packet (seq_num = %d, ack_num = %d)\n",serverHand.tcp_header.sequence_number,serverHand.tcp_header.ack_number);
    }

    handShake_seg.tcp_header.sequence_number = serverHand.tcp_header.ack_number;
    handShake_seg.tcp_header.ack_number = serverHand.tcp_header.sequence_number + 1;
    handShake_seg.tcp_header.flag = 20496;
    //send request
    if(sendto(sock, &handShake_seg, sizeof(handShake_seg), 0, (struct sockaddr* ) &server, server_length) < 0)
    {
        perror("Send data error.\n");
        exit(1);
    }
    printf("Send a packet(ACK) to %s : %d\n",srvip,htons(server.sin_port));
	printf("-----------Three-way handshake finish successfully-------------\n");
	sendData.tcp_header.dst_port = 3456;
	sendData.tcp_header.sequence_number = temp_cseq++;
	sendData.tcp_header.ack_number = temp_cack++;
	sendData.tcp_header.rwnd = temp_crwnd;
	sendData.tcp_header.checksum = 0;
	sendData.tcp_header.offset = 0;
	sendData.tcp_header.flag = 1282;
	//send connect ok
	strcpy(sendData.data_buffer, "ConnectOK");
	//start to send and receive required data
	if(sendto(sock, &sendData, MSS, 0, (sockaddr*) &client, client_leng) < 0)
	{
		perror("Send data error.\n");
		exit(1);
	} 
	if(recvfrom(sock, &recvData, MSS, 0, (sockaddr*) &client, (socklen_t*) &client_leng) < 0)
	{
		perror("Receive data error.\n");
		exit(1);
	}
	Show();
	//Send ACK
	sendData.tcp_header.flag = 1296;
	strcpy(sendData.data_buffer, "ACK");
	//string handler : take the file asked for and recorded
	ss << recvData.data_buffer; ss >> temp_cport; ss.str(""); ss.clear();
	client.sin_port = htons(temp_cport);
	strcpy(fileName, "receive");
	strcat(fileName, recvData.data_buffer); strcat(fileName, ".mp4");
	FILE* filePtr;
	//open and write the file mp4 that asked for from server
	filePtr = fopen(fileName, "wb");
	if(!filePtr)
	{
		printf("Open data error\n");
		exit(1);
	}
	//success open data
	else
	{
		printf("Please choose whhich video you want to get (V1,V2,V3,V4) : ");
		//scanf("%c", sendData.data_buffer);
		cin >>  sendData.data_buffer;
		//add ".mp4"
		strcat(sendData.data_buffer,".mp4");
		//wait for socket builded by server
		sleep(2);
		//send data error
		if(sendto(sock, &sendData, MSS, 0, (sockaddr*) &client, client_leng) <0 )
		{
			perror("Send data error.\n");
			exit(1);
		}
		//receive data from server
		while(recvfrom(sock, &recvData, MSS, 0, (sockaddr*) &client, (socklen_t*) &client_leng))
		{
			Show();
			//ACK number = ACK number
			sendData.tcp_header.ack_number = recvData.tcp_header.ack_number;
			//check the data is ok(no Pollution)
			if((recvData.tcp_header.flag & 1) == 1)
			{
				//write in data
				fwrite(recvData.data_buffer, recvData.tcp_header.offset, 1, filePtr);
				break;
			}
			//the size is still ok(means use the last)
			else 
				fwrite(recvData.data_buffer, MSS-20, 1, filePtr);
		}
	}
	//while finishing receive, close filePtr
	fclose(filePtr);
	//return 0;
}
void Show(){
	printf("------------PKT------------\n");
	printf("source port : %d\n", recvData.tcp_header.src_port);
	printf("destination port : %d\n", recvData.tcp_header.dst_port);
	printf("Sequence number : %d\n", recvData.tcp_header.sequence_number);
	printf("ACK number : %d\n", recvData.tcp_header.ack_number);
	printf("Transmit data flag: %d\n", recvData.tcp_header.flag);
}
