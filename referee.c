//  Compile with:
//  $ g++ referee.c -o referee

#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<wait.h>
#include	<unistd.h>
#include	<signal.h>
#include	<sys/types.h>

const int	MAX_NUM_PLAYERS		= 2;

const int	TEXT_LEN		= 256;

const int	NUM_POINTS_TO_WIN	= 4;

#define		PLAYER_PROG		"tennisPlayer"

const char*	pointNames[]		= {"love","15","30","40","game",
                                           "game-over","game-over"};

int		currentNumPlayers	= 0;

int		shouldPlay		= 1;

pid_t		playerArray[MAX_NUM_PLAYERS];

int		points[MAX_NUM_PLAYERS];

void		sigUsr1Handler	(int		sigNum,
				 siginfo_t*	infoPtr,
				 void*		dataPtr
				)
{
  //increment the corresponding points[] elements

  pid_t parentId = infoPtr -> si_pid;
  if(parentId  == playerArray[0]){
    points[0] = points[0] + 1;
    
  }
  if(parentId  == playerArray[1]){
    points[1] = points[1] + 1;

  }

  
  fprintf(stderr, "Referee: \"%s %s (server %d, receiver %d)\"\n",
	 pointNames[points[1]],pointNames[points[0]],
	 points[1],points[0]
	);

  if(points[0] ==  NUM_POINTS_TO_WIN  || points[1] ==  NUM_POINTS_TO_WIN){
    shouldPlay = 0;
  }
}


int		main		()
{
  struct sigaction	act;
  int			i;

  //install sigUsr1Handler() as the SIGUSR1 handler
  memset(&act, '\0', sizeof( act));
  act.sa_flags = SA_SIGINFO;
  act.sa_sigaction = sigUsr1Handler;
    sigaction(SIGUSR1, &act, NULL);

  // start both child processes
    for(int x = 0; x < 2; x++){
      pid_t childId = fork();
      if(childId != 0){
        playerArray[x] = childId;
      }

      else if(childId == 0){
	currentNumPlayers++;
	if(x == 0){
	execl(PLAYER_PROG, PLAYER_PROG, NULL);
	fprintf(stderr,"Cannot run %s\n",PLAYER_PROG);
	exit(EXIT_FAILURE);}
	else{
	  char text[TEXT_LEN];
          snprintf(text,TEXT_LEN,"%d",playerArray[0]);
          execl(PLAYER_PROG, PLAYER_PROG, text, NULL);
          fprintf(stderr,"Cannot run %s\n",PLAYER_PROG);
          exit(EXIT_FAILURE);
	}
      }
      else{
	write(STDERR_FILENO, "fork() failed\n", 14);
        exit(EXIT_FAILURE);

      }
    }

     

  // Wait for the game to be over
  while  (shouldPlay)
    sleep(1);

  //tell children to stop:
  printf("%d %d\n",points[1],points[0]);
  kill(playerArray[0], SIGTERM);
  wait(NULL);
  kill(playerArray[1], SIGTERM);
  wait(NULL);
  //  III.  Finished:
  return(EXIT_SUCCESS);
}
