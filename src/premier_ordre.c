/*=================================================*/
/* programme d'utilisation des "alarmes cycliques" */
/*-------------------------------------------------*/
/* Jacques BOONAERT / cours SEMBA et AMSE          */
/*=================================================*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>          /* ->INDISPENSABLE pour les types tempo. */
/*....................*/
/* variables globales */
/*....................*/
float K,                      /* ->gain statique                       */
      tau,                    /* ->cste de temps                       */
      Te,                     /* ->periode d'echantillonage            */
      T,                      /* ->periode du signal carré             */
      duree;                  /* ->durée totale de simulation          */
float temps = 0;              /* ->durée simulée                       */


float y = 0;
float z0, a0;

/*...................*/
/* prototypes locaux */
/*...................*/
void usage( char *);           /* ->aide de ce programme                */
void cycl_alm_handler( int );  /* ->gestionnaire pour l'alarme cyclique */
/*&&&&&&&&&&&&&&&&&&&&&&*/
/* aide de ce programme */
/*&&&&&&&&&&&&&&&&&&&&&&*/
void usage( char *pgm_name )
{
  if( pgm_name == NULL )
  {
    exit( -1 );
  };
  printf("%s <K> <tau> <Te> <T> <duree>\n", pgm_name );
  printf("Calcule les valeurs d'un système du premier ordre\n");
  printf("exemple : \n");
  printf("%s 2.0 0.2 0.01 1.0 10.0\n", pgm_name );
}

/*&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&*/
/* gestionnaire de l'alarme cyclique */
/*&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&*/
void cycl_alm_handler( int signal )
{
    //On met à jour le temps écoulé
    temps += Te;

    //On calcule quelle est la valeur de l'entrée
    float tempsDansPeriode = temps;
    while(tempsDansPeriode >T){
        tempsDansPeriode -= T;
    }
    int u = 1;
    if(tempsDansPeriode > T/2.0){
        u = 0;
    }

    //On calcule la nouvelle sortie à partir de la précédente
    y = y*z0 + a0 * u;
    
    printf("%.4f\t%d\t%.3f\n", temps, u, y );
    
}
/*#####################*/
/* programme principal */
/*#####################*/
int main( int argc, char *argv[])
{
  struct sigaction      sa,      /* ->configuration de la gestion de l'alarme */
                        sa_old;  /* ->ancienne config de gestion d'alarme     */
  sigset_t              blocked; /* ->liste des signaux bloques               */
  struct itimerval      period;  /* ->periode de l'alarme cyclique            */

  /* verification des arguments */
  if( argc != 6 )
  {
    usage( argv[0] );
    return( 0 );
  };
  /* recuperation des arguments */
  if( (sscanf(argv[1],"%f", &K) == 0)||
      (sscanf(argv[2],"%f", &tau) == 0)||
      (sscanf(argv[3],"%f", &Te) == 0) ||
      (sscanf(argv[4],"%f", &T) == 0)||
      (sscanf(argv[5],"%f", &duree) == 0) )
  {
    printf("ERREUR : probleme de format des arguments\n");
    printf("         passe en ligne de commande.\n");
    usage( argv[0] );
    return( 0 );
  };

  z0 = exp(-Te/tau);
  a0 = K * (1-z0);
  
  /* initialisation */
  sigemptyset( &blocked );
  memset( &sa, 0, sizeof( sigaction )); /* ->precaution utile... */
  sa.sa_handler = cycl_alm_handler;
  sa.sa_flags   = 0;
  sa.sa_mask    = blocked;
  
  /* installation du gestionnaire de signal */
  sigaction(SIGALRM, &sa, NULL );
  
  /* initialisation de l'alarme  */
  period.it_interval.tv_sec  = (int)Te;// Période de l'alarme en s
  period.it_interval.tv_usec = (int)(Te*1000000);// Partie décimale de la période en micro secondes
  period.it_value.tv_sec     = (int)Te;
  period.it_value.tv_usec    = (int)(Te*1000000);
  
  /* demarrage de l'alarme */
  setitimer( ITIMER_REAL, &period, NULL );

  /* on ne fait desormais plus rien d'autre que */
  /* d'attendre les signaux                     */
  do
  {
    pause();
  }
  while( temps < duree );
  /* fini */
  printf("FIN DU PROGRAMME.\n");
  return( 0 );  /* ->on n'arrive pas jusque la en pratique */
}

  
