To run the server program:
	"gcc -o ftserver ftserver.c"
	"ftserver <server port #>"

To compile and run the client program:
	to list files:
		"python ftclient.py <server name> <server port #> <-l> <client port #>"
	to retrieve a text file:
		"python ftclient.py <server name> <server port #> <-g> <filename> <client port #>"

Note:
	To avoid well-known port numbers, input validation excludes port numbers smaller than 
	40000 and greater than 65535.
