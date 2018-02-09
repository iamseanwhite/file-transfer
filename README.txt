To run the server program:
	"gcc -o ftserver ftserver.c"
	"ftserver <server port #>"

To compile and run the client program:
	"python ftclient.py <server name> <server port #> <-l> <client port #>"
	or
	"python ftclient.py <server name> <server port #> <-g> <filename> <client port #>"

Note:
	To avoid well-known port numbers, my input validation excludes port numbers smaller than 
	40000 and greater than 65535.

Sources Cited:
	Outline of client code taken from textbook/lecture.
	Outline of server code taken from lecture from previous class.
	Code for reading directory contents referenced from gnu.org
	Various references to StackOverflow and Piazza for miscelaneous problems