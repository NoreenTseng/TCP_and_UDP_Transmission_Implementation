#include "tcp_segment.h"

/*** Variables ****/
segment recvData;	//receive data pkt
segment sendData;	//send data pkt
int sock;
int tempSeqence_number=100;
/*** Function variables****/
//pack pkt ready to send
void sendPkt(uint16_t ,uint16_t ,uint32_t ,uint32_t ,uint16_t ,uint16_t ,uint16_t ,uint16_t,char*);
//set timeout:http://beej-zhtw.netdpi.net/09-man-manual/9-19-select?tmpl=%2Fsystem%2Fapp%2Ftemplates%2Fprint%2F&showPrintDialog=1
int ACKpause();

int main(int argc, char **argv){
	/*** Variables ***/
	sockaddr_in server;
 	sockaddr_in client;
 	int timeoutACK;
 	int ACKpkt;
 	//build socket
 	/**** Build socket ****/
	sock = socket(AF_INET,SOCK_DGRAM,0);
	if(sock < 0)
	{
		printf("Socket error.\n");
		exit(1);
	}
	/***** Client info Variables *****/
	int client_leng = sizeof(client);
	bzero((char*)&client,sizeof(client));	//initialize and clean up
	client.sin_family=AF_INET;
	client.sin_port=htons(6789);
	struct hostent *hostip = gethostbyname("127.0.0.1");
	if(hostip == 0) 
	{
		perror("Hostip error\n");
		exit(1);
	}
	//copy the network address to sockaddr_in structure
	bcopy(hostip -> h_addr_list[0], (caddr_t) &client.sin_addr, hostip -> h_length);
	/******* Three-way handshake **********/
    /*printf("-----------Three-way handshake-------------\n");
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
	printf("-----------Three-way handshake finish successfully-------------\n");*/
	//test V1.mp4 transmit
	sendPkt(0,6789,100,0,1282,32,0,0,"V1.mp4");
	if(sendto(sock,&sendData,MSS,0,(sockaddr*)&client,client_leng) < 0)
	{
		perror("Send data error.\n");
		return 0;
	}
	FILE*filePtr;
	//open and write in file
	filePtr = fopen("resultFromServer.mp4", "wb");
	if(!filePtr)
	{
		printf("Open data error\n");
		exit(1);
	}
	while(recvfrom(sock,&recvData,MSS,0,(sockaddr*)&client,(socklen_t*)&client_leng))
	{
		//show the pkt received info
		printf("Receive pkt at sequence number : ");
		cout << recvData.tcp_header.sequence_number;
		printf(" with flag : ");
		cout << recvData.tcp_header.flag << endl;
		//start receiving, sequence number one by one
		if(recvData.tcp_header.sequence_number==tempSeqence_number+1)
		{
			tempSeqence_number=tempSeqence_number+1;
			//show the First receive pkt info
			printf("---------- First receive-----------------\n");
			printf("Receive pkt at sequence number :");
			cout << tempSeqence_number;
			printf(" with data : ");
			printf("%X\n",recvData.data_buffer[0]);
			//write in file
			fwrite(recvData.data_buffer,MSS-20,1,filePtr);
			//check the data is ok(no Pollution)
			if((recvData.tcp_header.flag & 1) == 1) 
			{
				sendPkt(0,6789,recvData.tcp_header.sequence_number+1,tempSeqence_number,1282,32,0,0,"ACK");
				//send ACK
				if(sendto(sock,&sendData,MSS,0,(sockaddr*)&client,client_leng) < 0)
				{
					perror("Send data error.\n");
					return 0;
				}
				printf("Seqence number : ");
				cout << tempSeqence_number << endl;
				fclose(filePtr);	//while send data done, close filePtr
				break;
			}
			//setting timeout info including AVK, sequence number
			timeoutACK =ACKpause();
			//time out not yet occured, there are two times transmit
			if(timeoutACK==1)
			{
				//receiveing error
				if(recvfrom(sock,&recvData,MSS,0,(sockaddr*)&client,(socklen_t*)&client_leng) < 0)
				{
					perror("Receive data error.\n");
					return 0;
				}
				else
				{
					//not yet time out, receive the second pkt one by one
					if(recvData.tcp_header.sequence_number==tempSeqence_number+1)
					{
						tempSeqence_number=tempSeqence_number+1;
						//show the First receive pkt info
						printf("---------- First receive-----------------\n");
						printf("Receive pkt at sequence number :");
						cout << tempSeqence_number;
						printf(" with data : ");
						printf("%X\n",recvData.data_buffer[0]);
						//write in file
						fwrite(recvData.data_buffer,MSS-20,1,filePtr);
						sendPkt(0,6789,recvData.tcp_header.sequence_number+1,tempSeqence_number,1282,32,0,0,"ACK");
						//send ACK
						if(sendto(sock,&sendData,MSS,0,(sockaddr*)&client,client_leng) < 0)
						{
					 		perror("Send data error.\n");
							return 0;
						}
						//show the send receive pkt info
						printf("Send pkt at sequence number :");
						cout << tempSeqence_number;
						printf(" with flag : ");
						cout << recvData.tcp_header.flag << endl;
						//receive data done, close filePtr
						if((recvData.tcp_header.flag & 1) == 1) 
						{
							fclose(filePtr);
							break;
						}
					}
					//normally send ACK back to the server
					else
					{
						sendPkt(0,6789,recvData.tcp_header.sequence_number+1,tempSeqence_number,1282,32,0,0,"ACK");
						//send ACK
						if(sendto(sock,&sendData,MSS,0,(sockaddr*)&client,client_leng) < 0)
						{
							perror("Send data error.\n");
							return 0;
						}
						//show the send receive pkt info
						printf("Send pkt at sequence number :");
						cout << tempSeqence_number;
						printf(" with flag : ");
						cout << recvData.tcp_header.flag << endl;
					}
				}
			}
			//time out occured
			if(timeoutACK==0)
			{
				printf("----------TIME OUT----------\n");
				sendPkt(0,6789,recvData.tcp_header.sequence_number+1,tempSeqence_number,1282,32,0,0,"ACK");
				//send ACK
				if(sendto(sock,&sendData,MSS,0,(sockaddr*)&client,client_leng) < 0){
					perror("Send data error.\n");
					return 0;
				}
				//show the send receive pkt info
				printf("Send pkt at sequence number :");
				cout << tempSeqence_number;
				printf(" with flag : ");
				cout << recvData.tcp_header.flag << endl;
				//receive data done, close filePtr
				if((recvData.tcp_header.flag & 1)==1) 
				{
					fclose(filePtr);
					break;
				}
			}
			//function error
			if(timeoutACK==-1)
			{
				perror("Function error, please set out.\n");
				return 0;
			}
		}
		//send ACK ok
		else
		{
			//pack ACK message
			sendPkt(0,6789,recvData.tcp_header.sequence_number+1,tempSeqence_number,1282,32,0,0,"ACK");
			//send ACK
			if(sendto(sock,&sendData,MSS,0,(sockaddr*)&client,client_leng) < 0)
			{
				perror("Send data error\n");
				return 0;
			}
		}
	}
	//wait for pkt and then ACK
	ACKpkt = 0;
	//for the timeout setting
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 200000;
	setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv));
	while(true)
	{
		if((recvfrom(sock,&recvData,MSS,0,(sockaddr*)&client,(socklen_t*)&client_leng))!=-1)
		{
			//show the send receive pkt info
			printf("Send pkt at sequence number :");
			cout << recvData.tcp_header.sequence_number;
			//send data successfilly
			if(strcmp(recvData.data_buffer,"Send done")==0) 
			{
				printf("\n------------ Send data done ------------\n");
				return 0;
			}
		}
		//time out
		if(ACKpkt==10000) 
		{
			printf("------------ Time out, Send fail, please set out ------------\n");
			return 0;
		}
		//pack ACK message
		sendPkt(0,6789,recvData.tcp_header.sequence_number+1,tempSeqence_number,1282,32,0,0,"ACK");
		//send ACK
		if(sendto(sock,&sendData,MSS,0,(sockaddr*)&client,client_leng) < 0)
		{
					perror("Send data error.\n");
					return 0;
		}
		//show waiting info
		printf("\nThe waiting %d pkt at sequence number : ", ACKpkt);
		cout << tempSeqence_number << endl;
		ACKpkt++;
	}
}
void sendPkt(uint16_t srcport,uint16_t dstport,uint32_t seqN,uint32_t ackN,uint16_t fflag,uint16_t recvwnd,uint16_t check,uint16_t offsetbit,char* buf){
	sendData.tcp_header.src_port = srcport;
	sendData.tcp_header.dst_port = dstport;
	sendData.tcp_header.sequence_number = seqN++;
	sendData.tcp_header.ack_number = ackN++;
	sendData.tcp_header.flag = fflag;
	sendData.tcp_header.rwnd = recvwnd;
	sendData.tcp_header.checksum = check;
	sendData.tcp_header.offset = offsetbit;
	strcpy(sendData.data_buffer, buf);
}
//set timeout:http://beej-zhtw.netdpi.net/09-man-manual/9-19-select?tmpl=%2Fsystem%2Fapp%2Ftemplates%2Fprint%2F&showPrintDialog=1
int ACKpause(){
	int fb;
	struct timeval tv;
	tv.tv_sec = 0;	//seconds
	tv.tv_usec = 500000;	//500ms=0.5s
	fd_set readfds;
	//initialize readfds to zero
	FD_ZERO(&readfds);
	//set sock add to readfds
	FD_SET(sock,&readfds);
	// 在 timeout 以前會一直等待，看是否已經有資料可以接收(readable)的 socket
	fb = select(sock+1,&readfds,NULL,NULL,&tv);
	//select():檢查sockets是否有資料需要接收，或是否可以送出資料而不會發生 blocking，或是否有例外發生
	if(fb > 0) 
		return 1;
	else if(fb == 0) 
		return 0;
	else //select() error 
		return -1;
}	
