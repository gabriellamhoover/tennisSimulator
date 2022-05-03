#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<wait.h>
#include	<unistd.h>
#include	<signal.h>
#include	<sys/types.h>

const char*	playerNameCPtr	= "";

pid_t		opponentPid	= -1;

int		isServer	= 0;

int		shouldPlay	= 1;

int		isMyTurn	= 0;

void		swing		()
{
  sleep(1);
  isMyTurn		= 0;

  if  ((rand() % 4) == 0)
  {
    //  send SIGUSR2 to opponentPid and
    //  to print a message of disappointment
    fprintf(stderr,"%s : Darn it\n", playerNameCPtr);
    isMyTurn = isServer;
    kill(opponentPid, SIGUSR2);

  }
  else
  {
    // send SIGUSR1 to opponentPid and
    // to print the sound of the racket hitting the ball
    fprintf(stderr,"%s : fwack\n", playerNameCPtr);
    kill(opponentPid, SIGUSR1);
  }

}


void		sigTermHandler	(int		sigNum
				)
{
  //set shouldPlay to 0
  shouldPlay = 0;
  exit(EXIT_SUCCESS);
}


void		sigUsr1Handler	(int		sigNum,
				 siginfo_t*	infoPtr,
				 void*		dataPtr
				)
{
  //set isMyTurn and opponentPid
  isMyTurn = 1;
  opponentPid = infoPtr -> si_pid;
}


void		sigUsr2Handler	(int		sigNum,
				 siginfo_t*	infoPtr,
				 void*		dataPtr
				)
{
  //  set isMyTurn to isServer,
  //  and send SIGUSR1 to the parent process.
  isMyTurn = isServer;
  kill(getppid(),SIGUSR1);
}


int		main		(int		argc,
				 char*		argv[]
				)
{
  // Reset the random number generator:
  srand(getpid());

  // set up either as the server or receiver player:
  int len = sizeof(argv) / sizeof(argv[0]);
  
  if(argc == 2){
    char *cPtr;
    opponentPid = strtol(argv[1],&cPtr, 10);
    playerNameCPtr = "server";
    isServer = 1;
     isMyTurn = 1;
  }
  else{
    playerNameCPtr = "receiver";
  }
  //  install the signal handlers
  struct sigaction	act;
  memset(&act, '\0', sizeof(act));
  act.sa_flags = SA_SIGINFO;
  act.sa_sigaction = sigUsr1Handler;
  sigaction(SIGUSR1, &act, NULL);
  act.sa_flags = SA_SIGINFO;
  act.sa_sigaction = sigUsr2Handler;
  sigaction(SIGUSR2, &act, NULL);
  act.sa_flags = SA_SIGINFO;
  act.sa_handler = sigTermHandler;
  sigaction(SIGTERM, &act, NULL);


  //  Play the game:
  while  (shouldPlay)
  {
    while  (!isMyTurn)
      sleep(1);

    swing();
  }


  return(EXIT_SUCCESS);
}
