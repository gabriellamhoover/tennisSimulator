/*-------------------------------------------------------------------------*
 *---									---*
 *---		tennisClient.c						---*
 *---									---*
 *---	    This file defines a C program that tells a server program 	---*
 *---	to start a virtual tennis game and report the score back.	---*
 *---	This program then prints the score.				---*
 *---									---*
 *---	----	----	----	----	----	----	----	----	---*
 *---									---*
 *---									---*
 *-------------------------------------------------------------------------*/

//	Compile with:
//	$ gcc tennisClient.c -o tennisClient

//---		Header file inclusion					---//

#include	"clientServer.h"


//---		Definition of constants:				---//

#define	DEFAULT_HOSTNAME	"localhost"


//---		Definition of global vars:				---//

const char*	pointNames[]		= {"love","15","30","40","game","game-over","game-over"};


//---		Definition of functions:				---//

//  PURPOSE:  To ask the user for the name and the port of the server.  The
//	server name is returned in 'url' up to length 'urlLen'.  The port
//	number is returned in '*portPtr'.  No return value.
void	obtainUrlAndPort	(int		urlLen,
				 char*		url,
				 int*		portPtr
				)
{
  // Application validity check:
  if  ( (url == NULL)  ||  (portPtr == NULL) )
  {
    fprintf(stderr,"BUG: NULL ptr to obtainUrlAndPort()\n");
    exit(EXIT_FAILURE);
  }

  if   (urlLen <= 1)
  {
    fprintf(stderr,"BUG: Bad string length to obtainUrlAndPort()\n");
    exit(EXIT_FAILURE);
  }

  // Get server name and port number:
  //Get server name:
  printf("Machine name [%s]? ",DEFAULT_HOSTNAME);
  fgets(url,urlLen,stdin);

  char*	cPtr	= strchr(url,'\n');

  if  (cPtr != NULL)
    *cPtr = '\0';

  if  (url[0] == '\0')
    strncpy(url,DEFAULT_HOSTNAME,urlLen);

  // Get port numbe:
  char	buffer[BUFFER_LEN];

  printf("Port number? ");
  fgets(buffer,BUFFER_LEN,stdin);

  *portPtr = strtol(buffer,NULL,10);

  //Finished:
}


//  PURPOSE:  To attempt to connect to the server named 'url' at port 'port'.
//	Returns file-descriptor of socket if successful, or '-1' otherwise.
int	attemptToConnectToServer	(const char*	url,
					 int		port
					)
{
  // Application validity check:
  if  (url == NULL)
  {
    fprintf(stderr,"BUG: NULL ptr to attemptToConnectToServer()\n");
    exit(EXIT_FAILURE);
  }


  //  Attempt to connect to server:
  //  Create a socket:
  int socketDescriptor = socket(AF_INET, // AF_INET domain
				SOCK_STREAM, // Reliable TCP
				0);

  // Ask DNS about 'url':
  struct addrinfo* hostPtr;
  int status = getaddrinfo(url,NULL,NULL,&hostPtr);

  if (status != 0)
  {
    fprintf(stderr,gai_strerror(status));
    return(-1);
  }

  // Attempt to connect to server:
  struct sockaddr_in server;
  // Clear server datastruct
  memset(&server, 0, sizeof(server));

  // Use TCP/IP
  server.sin_family = AF_INET;

  // Tell port # in proper network byte order
  server.sin_port = htons(port);

  // Copy connectivity info from info on server ("hostPtr")
  server.sin_addr.s_addr =
	((struct sockaddr_in*)hostPtr->ai_addr)->sin_addr.s_addr;

  status = connect(socketDescriptor,(struct sockaddr*)&server,sizeof(server));

  if  (status < 0)
  {
    fprintf(stderr,"Could not connect %s:%d\n",url,port);
    return(-1);
  }

  freeaddrinfo(hostPtr);

  return(socketDescriptor);
}


//  PURPOSE:  To do the work of the application.  Gets letter from user, sends
//	it to server over file-descriptor 'socketFd', and prints returned text.
//	No return value.
void		communicateWithServer
				(int		socketFd
				)
{
  //  Application validity check:

  //  Do work of application:
  //  Get letter from user:
  char	buffer[BUFFER_LEN];
  int	serverScore;
  int	receiverScore;

  printf("Press enter to tell the server to play tennis:\n");
  fgets(buffer,BUFFER_LEN,stdin);

  write(socketFd,PLAY_CMD,sizeof(PLAY_CMD));

  if  ( (read(socketFd,  &serverScore,sizeof(int)) < 0) ||
	(read(socketFd,&receiverScore,sizeof(int)) < 0)
      )
  {
    fprintf(stderr,"Error receiving score\n");
  }
  else
  {
    serverScore		= ntohl(serverScore);
    receiverScore	= ntohl(receiverScore);

    if  ( (serverScore < 0) || (receiverScore < 0) )
      fprintf(stderr,"Error receiving score\n");
    else
    {
      if  (serverScore > receiverScore)
	printf("The server won");
      else
      if  (serverScore < receiverScore)
	printf("The receiver won");
      else
	printf("A tie");

      printf(": server %s, receiver %s\n",
	     pointNames[serverScore],pointNames[receiverScore]
	    );
    }
  }

  
}


//  PURPOSE:  To do the work of the client.  Ignores command line parameters.
//	Returns 'EXIT_SUCCESS' to OS on success or 'EXIT_FAILURE' otherwise.
int	main	()
{
  char		url[BUFFER_LEN];
  int		port;
  int		socketFd;

  obtainUrlAndPort(BUFFER_LEN,url,&port);
  socketFd	= attemptToConnectToServer(url,port);

  if  (socketFd < 0)
    exit(EXIT_FAILURE);

  communicateWithServer(socketFd);
  close(socketFd);
  return(EXIT_SUCCESS);
}
	      