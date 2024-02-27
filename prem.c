#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>  



int main( int argc, char *argv[])
{
//   struct sigaction      sa,      /* ->configuration de la gestion de l'alarme */
//                         sa_old;  /* ->ancienne config de gestion d'alarme     */
//   sigset_t              blocked; /* ->liste des signaux bloques               */
//   struct itimerval      period;  /* ->periode de l'alarme cyclique            */
  /* verification des arguments */
  if( argc != 4 )
  {
    usage( argv[0] );
    return( 0 );
  };
  /* recuperation des arguments */
  if( (sscanf(argv[1],"%d", &hh) == 0)||
      (sscanf(argv[2],"%d", &mm) == 0)||
      (sscanf(argv[3],"%d", &ss) == 0)  )
  {
    printf("ERREUR : probleme de format des arguments\n");
    printf("         passe en ligne de commande.\n");
    usage( argv[0] );
    return( 0 );
  };
}