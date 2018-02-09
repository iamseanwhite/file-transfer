//***************************************
//Name: Sean White
//Project 2
//File: ftserver.c
//Description: Server for the file transfer
//system that waits for incomming connections
//and processes requests based on client
//commands.
//***************************************
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <dirent.h>
#include <errno.h>
#include <ctype.h>
#include <netdb.h>


//***************************************
//error()
//prints error messages
//***************************************
void error(const char *msg) {	
	perror(msg); 
	exit(1); 
}


//***************************************
//setUpAddressStruct()
//sets up the addres struct that is used
//for reating the listen socket
//***************************************
struct sockaddr_in setUpAddressStruct(int portNumber){
	// Set up the address struct for this process (the server)
	struct sockaddr_in serverAddress;
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	//portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	//portNumber = 12000; // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	//serverAddress.sin_port = htons(atoi(argv[1])); // Store the port number
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process
	return serverAddress;
}


//***************************************
//startUpListenSocket()
//sets up and returns the listen socket
//***************************************
int startUpListenSocket(struct sockaddr_in serverAddress) {
	// Set up the socket
	int listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) 
		error("ERROR opening socket");
    //printf("4");

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections
	//printf("3");
	return listenSocketFD;
}


//***************************************
//initiateContact()
//Sets up the client address struct, the 
//socket and creates the data connection
//with the client
//***************************************

void initiateContact(struct sockaddr_in serverAddress, struct hostent* serverHostInfo, char* hostName, int portNumber, int* socketFD) {
	
    memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname(hostName); // Convert the machine name into a special form of address
	
	if (serverHostInfo == NULL) { 
	    fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
	    exit(0);    
	}
	
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr_list[0], serverHostInfo->h_length); // Copy in the address
    	
	// Set up the socket
	*socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (*socketFD < 0) error("CLIENT: ERROR opening socket");
	
	// Connect to server
	if (connect(*socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting");
}


//***************************************
//sendData()
//handles the client's request by sending
//pertinent data on data connection or 
//pertient error/ok messages on the conrrol
//connection
//***************************************
void sendData(int socket, char* message) {
	// Send data to server
	int charsWritten = send(socket, message, strlen(message), 0); // Write to the client
	//send(dataSocketFD, newLine, strlen(newLine), 0); // Write to the client
	if (charsWritten < 0) 
		error("Server: ERROR writing to socket");
	if (charsWritten < strlen(message)) 
		printf("Server: WARNING: Not all data written to socket!\n");	
}


//***************************************
//validatePortNumber()
//validates that the port number is in range 
//and consists of only numbers
//***************************************
int validatePortNumber(char* portNumberString){
	int i, portNumber;
	int foundNaN = 0;

	for(i = 0; i < strlen(portNumberString); i++) {
		if (isdigit(portNumberString[i]) == 0) {
			foundNaN = 1;
			//printf("foundNaN...tempstring is %i\n", portNumberString[i]);
		}
	}
	
	if (foundNaN != 0) {
		printf("Not a valid port number\n");
		exit(1);
	}
	
	portNumber = atoi(portNumberString);
	
	if (portNumber < 40000 || portNumber > 65535) {
		printf("Port number out of range\n");
		exit(1);
	}
	
	return portNumber;
}


int main(int argc, char *argv[]) {
	
	int establishedConnectionFD, portNumber, charsRead;
	socklen_t sizeOfClientInfo;
	struct sockaddr_in clientAddress;
	struct hostent* clientHostInfo;
	char buffer[140000];
	char partial[70000];
	char* localhost = "localhost";
	char* portNumberString = argv[1];
	//char* portNumberString = "12001";

	
	portNumber = validatePortNumber(portNumberString);

	struct sockaddr_in serverAddress = setUpAddressStruct(portNumber);
   
	int listenSocketFD = startUpListenSocket(serverAddress);
	
	
	while(1){
		// Accept a connection, blocking if one is not available until one connects
		sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr*) &clientAddress, &sizeOfClientInfo); // Accept
		if (establishedConnectionFD < 0) error("ERROR on accept");
		
		// Get the message from the client and display it
		memset(buffer, '\0', 140000);
		while(strstr(buffer, "&&") == NULL){
			memset(partial, '\0', 70000);
			charsRead = recv(establishedConnectionFD, partial, 70000 - 1, 0); // Read the client's message from the socket
			strcat(buffer, partial);
			if (charsRead < 0) error("ERROR reading from socket");
		}
		
		//remove &&
		buffer[strlen(buffer) - 1] = '\0';
		buffer[strlen(buffer) - 1] = '\0';
		
		
		//extract client's port number to set up data connection
		char subbuffer[5];
		memset(subbuffer, '\0', 5);
		memcpy(subbuffer, &buffer[strlen(buffer)-5], 5); //substring of length 5 starting from 3rd character
		
		
		//set up info for connecting to client
		int dataPort = atoi(subbuffer);
		int dataSocketFD;
		char clientHostName[1024];
		memset(clientHostName, '\0', 1024);
		char service[20];
		memset(service, '\0', 20);
		
		getnameinfo((struct sockaddr *)&clientAddress, sizeOfClientInfo, clientHostName, 1024, service, 20, NI_NOFQDN);
		
		initiateContact(clientAddress, clientHostInfo, clientHostName, dataPort, &dataSocketFD);
		
		
		//if the client wants the directory contents
		if (buffer[1] == 'l') {
			
			printf("\nConnection from %s\nList directory requested on port %i.\nSending directory contents to %s:%i\n", clientHostName, dataPort, clientHostName, dataPort);
			
			//open the current directory and read all the entries
			DIR *directory;
			struct dirent *entry;
			char allEntriesInDirectory[256];
			memset(allEntriesInDirectory, '\0', 256);
			
			directory = opendir ("./");
		    if (directory != NULL)
			{
				while (entry = readdir (directory)) {
					
					strcat(allEntriesInDirectory, entry->d_name);
					strcat(allEntriesInDirectory, "\n");
			        // printf ("%s", entry->d_name);
			    	
				}
			    (void) closedir (directory);
			}
			else
				perror ("Couldn't open the directory");
				
			char* okMessage = "ok";
			
			sendData(establishedConnectionFD, okMessage);	
				
			sendData(dataSocketFD, allEntriesInDirectory);
		}
		
		
		//if the client whats to transfer a file
		if (buffer[1] == 'g') {
			
			char* testMessage = "Your command is -g.";
			char fileToBeTransferred[256];
			
			memset(fileToBeTransferred, '\0', 256);
			memcpy(fileToBeTransferred, &buffer[3], (strlen(buffer)-9)); //substring that is the extracted file to be sent
			
			printf("\nConnection from %s\nFile %s requested on port %i.\n", clientHostName, fileToBeTransferred, dataPort);
			
			FILE *filePointer;
		    char fileBuffer[1000];
		    memset(fileBuffer, '\0', 1000);
		
		    filePointer = fopen(fileToBeTransferred, "r");
		    
		    if (filePointer == NULL) {
		    	//perror("Server: Error opening file");
		    	char* errorMessage = strerror(errno);
		    	
		    	printf("File not Found. Sending error message to %s:%i\n", clientHostName, dataPort);
		    	
				sendData(establishedConnectionFD, errorMessage);
		    }
		    
		    else {
		    	printf("Sending %s to %s:%i\n", fileToBeTransferred, clientHostName, dataPort);
		    	
		    	char* okMessage = "ok";
		    	
				sendData(establishedConnectionFD, okMessage);
		    	
		    	while (fgets(fileBuffer, 999, (FILE*)filePointer)) {
			    	
					sendData(dataSocketFD, fileBuffer);
					
					if (feof(filePointer)) break;
				}	
		
		    	fclose(filePointer);
		    }
		}
		close(dataSocketFD);
	}
}