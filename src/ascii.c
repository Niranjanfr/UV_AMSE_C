/*=============================*/
/* modele dynamique d'une cuve */
/*_____________________________*/
/* Jacques BOONAERT-LEPERS     */
/* evaluation AMSE 2023-2024   */
/*=============================*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
/*.............................*/
/* constantes de l'application */
/*.............................*/
#define     VOLUME      "VOLUME"
#define     NB_ASCII    50
/*....................*/
/* variables globales */
/*....................*/
double  Te,                       /* ->periode d'echantillonnage          */
        V_MAX;                    /* ->volume maximum de la cuve          */
double  *v;                       /* ->qe : pointeur sur la zone partagee */
int     GoOn = 1;                 /* ->controle d'execution               */
/*...................*/
/* prototypes locaux */
/*...................*/
void usage( char *);           /* ->aide de ce programme                  */
void cycl_alm_handler( int );  /* ->gestionnaire pour l'alarme cyclique   */
/*&&&&&&&&&&&&&&&&&&&&&&*/
/* aide de ce programme */
/*&&&&&&&&&&&&&&&&&&&&&&*/
void usage( char *pgm_name )
{
  if( pgm_name == NULL )
  {
    exit( -1 );
  };
  printf("%s <V_MAX> <periode d'ech.> \n", pgm_name );
  printf("Affiche une jauge de la cuve.\n");
  printf("<V_MAX>          : volume max en m³.\n");
  printf("<periode d'ech.> : periode d'echantillonnage en s.\n");
  printf("\n");
  printf("exemple : \n");
  printf("%s 100 0.01\n", pgm_name );
  printf("affichage de la cuve avec une periode");
  printf("d'echantillonnage de 0.01 s et un\n");
  printf("volume maximum de 100m³\n");
}
/*&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&*/
/* gestionnaire de l'alarme cyclique */
/*&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&*/
void cycl_alm_handler( int signal )
{
    /*...............................*/
    /* mise a jour entree / sortie : */
    /*...............................*/
    if( signal == SIGALRM)
    {
        
    };

    /*................................*/
    /* arret du processus a reception */
    /* de SIGUSR1                     */
    /*................................*/
    if( signal == SIGUSR1)
    {
        GoOn = 0;
    };
}
/*#####################*/
/* programme principal */
/*#####################*/
int main( int argc, char *argv[])
{
  struct sigaction      sa,         /* ->configuration de la gestion de l'alarme */
                        sa_old;     /* ->ancienne config de gestion d'alarme     */
  sigset_t              blocked;    /* ->liste des signaux bloques               */
  struct itimerval      period;     /* ->periode de l'alarme cyclique            */
  int                   fd_volume;  /* ->zone partagee VOLUME */

  /* verification des arguments */
  if( argc != 3 )
  {
    usage( argv[0] );
    return( 0 );
  };
  /* recuperation des arguments */
  if( (sscanf(argv[1],"%lf", &V_MAX ) == 0)||
      (sscanf(argv[2],"%lf", &Te    ) == 0)   )  
  {
    printf("ERREUR : probleme de format des arguments\n");
    printf("         passe en ligne de commande.\n");
    usage( argv[0] );
    return( 0 );
  };


  //Vérification des arguments
  if(V_MAX<=0){
    printf("Le volume max ne peut pas être inférieur ou égal à 0.\n");
    return 0;
  }
  if(Te<=0){
    printf("La période d'échantillonage ne peut pas être inférieure ou égale à 0.\n");
    return 0;
  }


  /*................*/
  /* initialisation */
  /*................*/
  /* creation des zones partagees */
  /*         --->VOLUME<---            */
  printf("zone VOLUME\n");
  fd_volume = shm_open(VOLUME, O_RDWR | O_CREAT, 0600);
  if( fd_volume < 0)
  {
    fprintf(stderr,"ERREUR : main() ---> appel a shm_open() VOLUME\n");
    fprintf(stderr,"        code d'erreur %d (%s)\n", 
                            errno, 
                            (char *)(strerror(errno)));
    return( -errno );
  };
  ftruncate( fd_volume, sizeof(double));
  v =  (double *)mmap(NULL, 
                      sizeof(double), 
                      PROT_READ | PROT_WRITE, 
                      MAP_SHARED, 
                      fd_volume, 
                      0                         );

  sigemptyset( &blocked );
  memset( &sa, 0, sizeof( sigaction )); /* ->precaution utile... */
  sa.sa_handler = cycl_alm_handler;
  sa.sa_flags   = 0;
  sa.sa_mask    = blocked;
  
  /* installation du gestionnaire de signal */
  sigaction(SIGALRM, &sa, NULL );
  sigaction(SIGUSR1, &sa, NULL );

  /* initialisation de l'alarme  */
  period.it_interval.tv_sec  = (int)(Te);  
  period.it_interval.tv_usec = (int)((Te - (int)(Te))*1e6);
  period.it_value.tv_sec     = (int)(Te);
  period.it_value.tv_usec    = (int)((Te - (int)(Te))*1e6);
  /* demarrage de l'alarme */
  setitimer( ITIMER_REAL, &period, NULL );
  
  /* on ne fait desormais plus rien d'autre que */
  /* d'attendre les signaux                     */
  do
  {
    pause();
    //Clear screen
    system("clear");
    //Print cuve
    int nbRempli = (int)((*v) / V_MAX * NB_ASCII);
    for(int i=0; i< nbRempli; i++){
        printf("#");
    }
    for(int i=0; i<NB_ASCII-nbRempli; i++){
        printf("_");
    }
    printf("\n");
  }
  while( GoOn == 1 );
  /* menage */
  close(fd_volume);
  /* fini */
  printf("FIN.\n");
  return( 0 );  /* ->on n'arrive pas jusque la en pratique */
}

  
