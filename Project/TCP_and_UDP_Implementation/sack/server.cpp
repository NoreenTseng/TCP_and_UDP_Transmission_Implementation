/* 
** 
** Set and print the arguments.
** Slow start & congestion avoidance delayed ACK
** loss pkt
** Slow start:
** 	1. cwnd < ssthtess and no timeout -> cwnd = cwnd*2
** 	2. timeout -> ssthress = cwnd/2, then cwnd = 1
** 	3. cwnd > ssthress -> go to congestion avoidance
** Congestion avoidence
** 	1. no timeout -> cwnd = cwnd +1
** 	2. timeout -> ssthress = cwnd/2, then cwnd = 1 and go to slow start 
*/

#include "tcp_segment.h"

/*** Variables ****/
#define connectPort 6789	//auto port setting
#define SLOWSTART 0
#define CONGESTION_AVOIDANCE 1
int sock;
char filename[1024];	//the file needed transmit
int upperTrans = 100;	//upperest transmit limits
int lowerTrans = 99;	//lowest transmit limits
int ACKnumber =  99;	//auto ACK number setting
int serverCWND = 1;	//cwnd
int dupACK = 0;		//duplicated ACK occured
int status;	//status of Slow start or Congestion avoidence
int ssth = 20;	//lower threshold
stringstream ss;
struct ddata{
	char content [1024];	//data value
	//char content[MSS];
	int value;	//data content with MSS (1 Kbytes)
};
segment recvData;	//receive data pkt
segment sendData;	//send data pkt
vector <ddata> serverPkt;	//send data buffer
struct ddata tempData;

/*** Function variables****/
//send data pkt
void firstSendPkt(uint16_t,uint16_t ,uint32_t ,uint32_t ,uint16_t ,uint16_t ,uint16_t ,uint16_t ,int );
//send "send done" message pkt
void SecondSendPkt(uint16_t ,uint16_t ,uint32_t ,uint32_t ,uint16_t ,uint16_t ,uint16_t ,uint16_t ,char* );
//set timeout:http://beej-zhtw.netdpi.net/09-man-manual/9-19-select?tmpl=%2Fsystem%2Fapp%2Ftemplates%2Fprint%2F&showPrintDialog=1
int ACKpause();
//void possion(const double, const int);
int SACK(int ,int);

int main(int argc, char **argv){
	/******** Variables **********/
	int upperBuffer;
	int inputBytes;
	int overCWND;
	int timeoutACK;
	int count = 0;
	int switchMode = 0;
	sockaddr_in server;
	sockaddr_in client;
	srand( time(NULL) );	//set time
	/**** client's info ****/
	int nlength=sizeof(client);
	/**** Build socket ****/
	sock = socket(AF_INET,SOCK_DGRAM,0);
	if(sock < 0)
	{
		printf("Socket error.\n");
		exit(1);
	}
	/**** server's info ****/
	bzero((char*)&server,sizeof(server));
	server.sin_family=AF_INET;
	server.sin_addr.s_addr=htonl(INADDR_ANY);
	server.sin_port=htons(connectPort);
	//inet_aton(argv[1], &server.sin_addr);	//set IP address
	//server.sin_port = htons(atoi(argv[2]));	//set port number
	//char* serverip = argv[1];
	//bzero((char*) &client, sizeof(client));	//initialize the sockaddr_in
	//client.sin_family = AF_INET;
	//inet_aton(argv[1], &client.sin_addr);	//set IP address
	//client.sin_port = htons(atoi(argv[2]));	//set port number
	/**** bind() ****/
	if(bind(sock,(sockaddr*)&server,sizeof(server)) < 0)
	{
		printf("Bind error.");
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
    printf("-----------Transmit Data Status-------------\n");
    printf("Status 0 : SLOW START\n");
    printf("Status 1 : CONGESTION AVOIDANCE\n");
    /************ Data transmit ****************/
	printf("-----------Start to transmit Data-------------\n");
	//read data that has to send  
	if(recvfrom(sock, &recvData, MSS, 0,(sockaddr*) &client,(socklen_t*) &nlength) < 0 )
	{
		perror("Receive data error\n");
		return 0;
	}
	printf("The data chose by client : %s\n",recvData.data_buffer);
	status=SLOWSTART;	//slow start
	//transmit in the begin
	lowerTrans=recvData.tcp_header.sequence_number+1;
	//the ack number of the transmit in the begin
	ACKnumber=lowerTrans-1;
	//open and read data
	FILE * filePtr;
	filePtr = fopen(recvData.data_buffer, "rb");
	if(!filePtr)
	{
		printf("Open data error\n");
		exit(1);
	}
	inputBytes = 0;
	while(!feof(filePtr))
	{
		fread(filename, MSS-20, 1, filePtr);
		//print data pointer pointing to 
		printf("Data pointer to %ld ",ftell(filePtr));
		printf(" with data buffer : %X\n", filename[0]);
		if( ftell(filePtr)%(MSS-20) != 0 )
			printf("----------- Read and send data done -----------\n");
		memcpy(tempData.content,filename,MSS-36);
		//increase input data slowly
		tempData.value=lowerTrans+inputBytes;
		inputBytes++;
		//push input bytes to the stack
		serverPkt.push_back(tempData);
	}
	fclose(filePtr);	//finish, close filePtr
	//the upper limits of slow start
	upperBuffer = serverPkt[serverPkt.size()-1].value;
	printf("The upper limits of data tramsit : ");
	cout << upperBuffer << endl;
	/***********Slow start and Congestion Avoidance****************/
	while(true)
	{
		/*int sack;
		for(int t = 0; t < serverPkt.size(); t++)
		{
			//ack number is same
			if(serverPkt[t].content == ACKnumber)
				sack = t;	//record the sack occured
		}
		SACK(ACKnumber, sack);*/
		for(int i = 0; i < serverCWND; i++)
		{
			char content[32];
			//string handler
			ss << lowerTrans+i ; ss >> content; ss.str(""); ss.clear();
			//overflow
			if(lowerTrans+i > upperBuffer) 
				break;
			//start to send data
			if(lowerTrans+i == upperBuffer)
				firstSendPkt(6789,0,lowerTrans+i,recvData.tcp_header.sequence_number,1281,0,0,0,lowerTrans+i);
			//flag is 1280
			else 
				firstSendPkt(6789,0,lowerTrans+i,recvData.tcp_header.sequence_number,1280,0,0,0,lowerTrans+i);
			// randan generate Poisson distribution ???
			double p_dist = 0.000001;
			int sample = rand()%60;
			//possion(p_dist,sample);
			// uncomment to use a non-deterministic generator
    		//std::random_device gen;
    		std::mt19937 gen(1701);
    		std::poisson_distribution<> distr(p_dist);
    		// generate the distribution as a histogram
    		std::map<int, int> histogram;
    		for (int i = 0; i < sample; ++i)
       			++histogram[distr(gen)];
			if(histogram[distr(gen)])
			{
				//send error
				if(sendto(sock,&sendData,MSS,0,(sockaddr*)&client,nlength) < 0)
				{
					perror("Send data error\n");
					return 0;
				}
			}
			//show loss pkt info
			else 
			{
				printf("---------Loss PKT at ");
				cout << lowerTrans+i << " ---------" << endl;
			}
			//show the status of sending pkt
			 upperTrans = lowerTrans+i;
			printf("cwnd : %d\n", serverCWND);
			printf("	send pkt : %d\n", lowerTrans + i);
		}
		//to determine whether slow start (cwnd > ssth ->) go to congestion avoidance
		overCWND = serverCWND;
		count = 0;
		switchMode = 0;
		//send data starts
		for(int j = 0; j < serverCWND; j++)
		{
			timeoutACK=ACKpause();
			if(timeoutACK==1)
			{                     
				//receive error
				if(recvfrom(sock,&recvData,MSS,0,(sockaddr*)&client,(socklen_t*)&nlength) < 0)
				{
					perror("Receive data error\n");
					return 0;
				}
				//client receive all the pkts
				if(recvData.tcp_header.ack_number == upperTrans ) 
				{  
					dupACK = 0 ;
					ACKnumber =  recvData.tcp_header.ack_number ;
					//show the info of sending pkts now
					printf("With cwnd : %d\n", serverCWND); 
					printf("	count the acks : %d %d %d %d %d %s\n",overCWND,lowerTrans,upperTrans,ACKnumber,recvData.tcp_header.ack_number,recvData.data_buffer);
					//send pkts finish
					if(recvData.tcp_header.ack_number == upperBuffer) 
					{
						//show pkt send done info
						printf("----------- Send data done -----------\n");
						printf("The upper limits of data tramsit : ");
						cout << upperBuffer << endl;
						//pack the pkt that send done
						SecondSendPkt(6789,0,10000,recvData.tcp_header.sequence_number,1280,0,0,0,"Send done");
						if(sendto(sock,&sendData,MSS,0,(sockaddr*)&client,nlength) < 0)
						{
							perror("Send data error\n");
							return 0;
						}
						printf("----------- Send data signal done -----------\n");
						return 0;
					}
					//slow start status setting
					if(status==SLOWSTART) 
						overCWND = serverCWND*2;
					else
						count = serverCWND;
					break;
				}
				/********FAST RECOVERY********/
				//duplicated ACK occured
				else if (recvData.tcp_header.ack_number == ACKnumber) 
					dupACK++; 
				//delayed ACK, not yet time out
				else 
				{  
					printf("With cwnd : %d\n", serverCWND); 
					printf("	count the acks : %d %d %d %d %d %s\n",overCWND,lowerTrans,upperTrans,ACKnumber,recvData.tcp_header.ack_number,recvData.data_buffer);
					//count duplicated ACK occured
					//ACK that late back, no timeout
					int ACKback = recvData.tcp_header.ack_number-ACKnumber;
					//update and renew ACK number
					ACKnumber =recvData.tcp_header.ack_number;
					//send data done
					if(recvData.tcp_header.ack_number==upperBuffer) 
					{
						//show pkt send done info
						printf("----------- Send data done -----------\n");
						printf("The upper limits of data tramsit : ");
						cout << upperBuffer << endl;
						//pack send done message info
						SecondSendPkt(6789,0,10000,recvData.tcp_header.sequence_number,1280,0,0,0,"Send done");
						if(sendto(sock,&sendData,MSS,0,(sockaddr*)&client,nlength) < 0)
						{
							perror("Send data error\n");
							return 0;
						}
						printf("----------- Send data signal done -----------\n");
						return 0;
					}
					//status switch
					if(status==SLOWSTART) 
						overCWND = overCWND + ACKback;
					else
						count = count + ACKback;
				}
			}
			//timeout occurred, then retransmi
			else if(timeoutACK==0)
			{ 
				//Retransmit status
				if(status == SLOWSTART) 
				{
					printf("----------- Please Retransmit immediately -----------\n");
					//change the parameter setting
					ssth = serverCWND/2;
					serverCWND = 1;
					dupACK = 0;
					switchMode = 1;
					break;
				}
				//enter slow start status
				else
				{
					printf("----------- SLOW START -----------\n");
					status = SLOWSTART;
					ssth = serverCWND/2;
					serverCWND =1;
					dupACK = 0;
					switchMode = 1;
					break;
				}
			}
			//function error
			else 
			{
				printf("Function error, please set out.\n");
				return 0;
			}
		}
		//enter congestion avoidance
		lowerTrans = ACKnumber + 1;
		//SLOWSTART to CONGESTION AVOIDANCE
		if(status == SLOWSTART ){
			if((overCWND==serverCWND*2)&&(switchMode==0)) 
				serverCWND = serverCWND*2;
			if((serverCWND>=ssth)&&(switchMode==0)) 
			{
				printf("----------- CONGESTION AVOIDANCE -----------\n");
				status = CONGESTION_AVOIDANCE;
			}
		}
		//no timeout -> cwnd = cwnd +1
		else if(count==serverCWND) 
				serverCWND =serverCWND+1;
	}
	//show the deplicated ACK info
	printf(" with Duplicated ACK : ");
	cout << dupACK << endl;
}
void firstSendPkt(uint16_t srcport,uint16_t dstport,uint32_t seqN,uint32_t ackN,uint16_t fflag,uint16_t recvwnd,uint16_t check,uint16_t offsetbit,int buf){
	sendData.tcp_header.src_port = srcport;
	sendData.tcp_header.dst_port = dstport;
	sendData.tcp_header.sequence_number = seqN++;
	sendData.tcp_header.ack_number = ackN++;
	sendData.tcp_header.flag = fflag;
	sendData.tcp_header.rwnd = recvwnd;
	sendData.tcp_header.checksum = check;
	sendData.tcp_header.offset = offsetbit;
	//copy MSS-36 characters from serverPkt[i].content to sendData.data_buffer
	//8192
	memcpy(sendData.data_buffer,serverPkt[check].content,MSS-36);
}
//send "send done" message pkt
void SecondSendPkt(uint16_t srcport,uint16_t dstport,uint32_t seqN,uint32_t ackN,uint16_t fflag,uint16_t recvwnd,uint16_t check,uint16_t offsetbit,char* buf){
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
	/******* Variables *********/
	int fd;
	struct timeval tv;
	tv.tv_sec = 0;	//seconds
	tv.tv_usec = 500000;	//500ms=0.5s
	fd_set readfds;
	//initialize readfds to zero
	FD_ZERO(&readfds);
	//set sock add to readfds
	FD_SET(sock,&readfds);
	// 在 timeout 以前會一直等待，看是否已經有資料可以接收(readable)的 socket
	fd = select(sock+1,&readfds,NULL,NULL,&tv);
	//select():檢查sockets是否有資料需要接收，或是否可以送出資料而不會發生 blocking，或是否有例外發生
	if(fd > 0) 
		return 1;
	else if(fd == 0) 
		return 0;
	else  //select() error 
		return -1;
}	
//SACK occured
/*int SACK(int ackN,int sackN){
	if(ackN == 0||sackN == 0)
		return 0;
	else if(sackN < ackN)
		return 0;
	for(int i = 0; i < serverPkt.size(); i++)
	{
		//identify sack
		if(serverPkt[i].value == ackN)
		{
			serverPkt[i].value=1;
			//clean to 1
			for(int j = 0; j <= (sackN-ackN); j++)
				serverPkt[i+j].value=1;
			break;
		}
	}
	
}*/
/*void possion(const double p, const int s){
	// uncomment to use a non-deterministic generator
    //    std::random_device gen;
    std::mt19937 gen(1701);
    std::poisson_distribution<> distr(p);
    // generate the distribution as a histogram
    std::map<int, int> histogram;
    for (int i = 0; i < s; ++i)
        ++histogram[distr(gen)];
}*/
