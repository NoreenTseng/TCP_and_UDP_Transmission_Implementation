all:server client

server:server.o
	g++ server.cpp -o server
	
client:client.o
	g++ client.cpp -o client

clean:
	rm -f server client *.o

