#***************************************
#Name: Sean White
#Project 2
#File: ftclient.py
#Description: Client of the file transfer
#system that takes commands via the
#command line to list directory contents
#or transfer a specific file
#***************************************
from socket import *
import sys
import os.path


#***************************************
#startUpListenSocket()
#sets up and returns the listen socket
#***************************************
def startUpListenSocket(dataPort):
    dataSocket = socket(AF_INET,SOCK_STREAM)
    dataSocket.bind(('', dataPort))
    dataSocket.listen(1)
    return dataSocket;
    
    
#***************************************
#validateArguments()
#validates all command line arguments
#***************************************
def validateArguments(arguments):
    try:
        int(arguments[2])
    except ValueError:
        print("Server port number not valid")
        return -1;
    
    if (int(arguments[2]) < 40000) or (int(arguments[2]) > 65535):
        print("Server port number out of range")
        return -1;
    
    if arguments[3] != "-l" and arguments[3] != "-g":
        print("command not supported")
        return -1;
        
    if len(arguments) == 5:
        if arguments[3] != "-l":
            print ("Arguments not valid")
            return -1;
            
        try:
            int(arguments[4])
        except ValueError:
            print("Client port number not valid")
            return -1;
    
        if (int(arguments[4]) < 40000) or (int(arguments[4]) > 65535):
            print("Client port number out of range")
            return -1;
    
    elif len(arguments) == 6:
        if arguments[3] != "-g":
            print ("Arguments not valid")
            return -1;
        try:
            int(arguments[5])
        except ValueError:
            print("Client port number not valid")
            return -1;
    
        if (int(arguments[5]) < 40000) or (int(arguments[5]) > 65535):
            print("Client port number out of range")
            return -1;
    else:  
        print ("Arguments not valid")
        return -1;
        
    return;
    
    
#***************************************
#createSocket()
#creates and returns socket
#***************************************
def createSocket(serverName, serverPort):
    newSocket = socket(AF_INET,SOCK_STREAM)
    newSocket.connect((serverName, serverPort))
    return newSocket;

#****************************************
#receiveMessage()
#recieves and prints the servers's status
#messages and prints directory contents
#****************************************   
def receiveMessage(connectionSocket, serverName, serverPort, flag):
    message = connectionSocket.recv(1024).decode()
    if message == "ok":
		#print "That's all"
		return 0;
    #if recieving directory listing
    if flag:
        print "\nReceiving directory structure from", serverName, ":", serverPort
        print message
    else:
        print "\n", serverName, ":", serverPort, "says", message, "\n"
    return 1;
    
#****************************************
#downloadFile()
#downloads file from server
#****************************************   
def downloadFile(connectionSocket, file):
    line = connectionSocket.recv(1024).decode()
    if line == '':
		#print "That's all"
		return 0;
    file.write(line)
    return 1;
    
#****************************************
#createDuplicateFileName()
#creates and returns duplicate file name
#or orignial name if duplicate is not 
#needed
#****************************************   
def createDuplicateFileName(fileName):
    if os.path.exists(fileName):
        try:
            int(fileName[len(fileName)-5])
        except ValueError:
            #convert to list because python is dumb
            fileNameList = list(fileName)
            fileNameList.insert(len(fileName)-4, '1')
        else:
            newVersionNumber = str(int(fileName[len(fileName)-5]) + 1)
            fileNameList = list(fileName)
            fileNameList[len(fileName)-5] = newVersionNumber
            
        newFileName = ''.join(fileNameList)
        return createDuplicateFileName(newFileName)
            
    else:
        return fileName;

def main():
   
    #validate parameters
    if validateArguments(sys.argv) == -1:
        return -1;
    
    #assign parameters to variables
    serverName = sys.argv[1]    
    serverPort = int(sys.argv[2])
    command = sys.argv[3]
    
    if len(sys.argv) == 5:
        dataPort = int(sys.argv[4])
        command += ' ' + str(dataPort)
    
    if len(sys.argv) == 6:
        fileName = sys.argv[4]
        dataPort = int(sys.argv[5])
        command += ' ' + fileName
        command += ' ' + str(dataPort)
        
    #add && to let server know the end has been reached.
    command += '&&'
    
    #setup conrol socket for commands/status messages
    controlSocket = createSocket(serverName, serverPort)
    controlSocket.send(command)
    
    #set up data socket for data transfer
    serverSocket = startUpListenSocket(dataPort)
    dataSocket, addr = serverSocket.accept()
    
    #recieve control messsage and if there is am error message, quit
    if receiveMessage(controlSocket, serverName, serverPort, 0):
        return;
    
    #if command was -l, recieve data from server    
    elif len(sys.argv) == 5:     
        receiveMessage(dataSocket, serverName, serverPort, 1)
    
    #if command was -g, download file form server
    elif len(sys.argv) == 6:
        print "\nReceiving", fileName, "from", serverName, ":", serverPort
        fileName = createDuplicateFileName(fileName)
        file = open(fileName, 'w+')
        while downloadFile(dataSocket, file):
            continue
        file.close()
        print "Transfer of", sys.argv[4], "is complete."
        print "File saved as", fileName, "\n"
        
    
    controlSocket.close()
    
main()