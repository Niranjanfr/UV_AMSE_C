/*=================================================*/
/* programme d'utilisation des "alarmes cycliques" */
/*-------------------------------------------------*/
/* Jacques BOONAERT / cours SEMBA et AMSE          */
/*_________________________________________________*/
/* simulation d'un systeme de type "premier ordre" */
/* (le signal d'entree est genere en "interne")    */
/*_________________________________________________*/
/* dans cette version, l'entree est stockee dans   */
/* une zone de memoire partagee                    */
/*=================================================*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>             /* NOUVEAU */
#include <sys/mman.h>          /* NOUVEAU */
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>          /* ->INDISPENSABLE pour les types tempo. */
/*....................*/
/* variables globales */
/*....................*/
double  z0,                       /* ->recurrence */
        a0,                       /* ->recurrence */
        Tu,                       /* ->periode du signal carre */
        duree,                    /* ->duree de la simulation (s) */
        Te;                       /* ->periode d'echantillonnage */
double  *u;                       /* ->u : pointeur sur la zone partagee   */
int     GoOn = 1;                 /* ->controle d'execution                */
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
  printf("%s <gain> <cste de temps> <periode ech.> <periode> <duree>\n", pgm_name );
  printf("simule un premier ordre soumis a un signal carre\n");
  printf("<gain> : gain du premier ordre\n");
  printf("<cste de temps> : constante de temps du premier ordre\n");
  printf("<periode ech.> : periode d'echantillonnage (s)\n");
  printf("<periode> : periode du signal carre (s)\n");
  printf("<duree> : duree de la simulation (s)\n");
  printf("\n");
  printf("exemple : \n");
  printf("%s 1.0 0.15 0.01 0.5 10\n", pgm_name );
  printf("realise la simulation sur 10s, avec un premier ordre de\n");
  printf("fonction de transfert 1.0/(1 + 0.15*p) en utilisant une\n");
  printf("periode d'echantillonnage de 0.01 s\n");
}
/*&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&*/
/* gestionnaire de l'alarme cyclique */
/*&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&*/
void cycl_alm_handler( int signal )
{
    static double y0     = 0.0;     /* ->y(k)                       */
    static double y1;               /* ->y(k+1)                     */
    static double t      = 0.0;     /* ->duree intercalaire         */
    static double Ttotal = 0.0;     /* ->duree totale de simulation */
    /*...............................*/
    /* mise a jour entree / sortie : */
    /*...............................*/
    if( signal == SIGALRM)
    {
        y1 = z0 * y0 + a0 * (*u);
        /* MAL... : printf a eviter dans les signal handler... */
        printf("%lf\t%lf\t%lf\n", Ttotal, *u, y1);
        /* preparation pour l'occurence suivante */
        t+=Te;
        Ttotal+=Te;
        y0 = y1;
        if( Ttotal > duree)
        {
            GoOn = 0;
        };
        /* changement d'etat eventuel de l'entree */
        /*
        if( t > Tu )
        {
            if( *u == 0.0)
            {
                *u = 1.0;
            }
            else
            {
                *u = 0.0;
            };
            t = 0.0;
        };
        */
    };
}
/*#####################*/
/* programme principal */
/*#####################*/
int main( int argc, char *argv[])
{
  double                k,       /* ->gain statique */
                        tau;     /* ->constante de temps */
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
  if( (sscanf(argv[1],"%lf", &k     ) == 0)||
      (sscanf(argv[2],"%lf", &tau   ) == 0)||
      (sscanf(argv[3],"%lf", &Te    ) == 0)||  
      (sscanf(argv[4],"%lf", &Tu    ) == 0)||
      (sscanf(argv[5],"%lf", &duree ) == 0)   )
  {
    printf("ERREUR : probleme de format des arguments\n");
    printf("         passe en ligne de commande.\n");
    usage( argv[0] );
    return( 0 );
  };
  /*................*/
  /* initialisation */
  /*................*/
  /* creation de la zone partagee */
  int fd;   /* ->"file descriptor" correspondant a la zone partagee */
  fd = shm_open("ENTREE", O_RDWR | O_CREAT, 0600);
  if( fd < 0)
  {
    fprintf(stderr,"ERREUR : main() ---> appel a shm_open()\n");
    fprintf(stderr,"        code d'erreur %d (%s)\n", 
                            errno, 
                            (char *)(strerror(errno)));
    return( -errno );
  };
  ftruncate( fd, sizeof(double));
  u =  (double *)mmap(NULL, 
                      sizeof(double), 
                      PROT_READ | PROT_WRITE, 
                      MAP_SHARED, 
                      fd, 
                      0                         );
  sigemptyset( &blocked );
  memset( &sa, 0, sizeof( sigaction )); /* ->precaution utile... */
  sa.sa_handler = cycl_alm_handler;
  sa.sa_flags   = 0;
  sa.sa_mask    = blocked;
  z0 = exp(-Te/tau);
  a0 = k*(1.0 - z0);
  /* installation du gestionnaire de signal */
  sigaction(SIGALRM, &sa, NULL );
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
  }
  while( GoOn == 1 );
  /* fini */
  printf("FIN DU DECOMPTE.\n");
  return( 0 );  /* ->on n'arrive pas jusque la en pratique */
}

  
