/*=============================*/
/* modele de régulation du     */
/* volume d'une cuve           */
/*_____________________________*/
/* Niranjan KULKARNI           */
/* Lucas NAURY                 */
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
#define     DEBIT       "DEBIT"
#define     NIVEAU      "NIVEAU"
#define     CONSIGNE    "CONSIGNE"
#define     STOP        "STOP"
/*....................*/
/* variables globales */
/*....................*/
double  *y,                       /* ->hauteur de fluide dans la cuve         */
        *yc,                      /* ->consigne : hauteur du fluide souhaitée */
        K,                        /* ->gain du régulateur                     */
        Te;                       /* ->periode d'echantillonnage              */
double  *qe;                      /* ->qe : pointeur sur la zone partagee     */
int     *stop;                    /* ->stop : pointeur sur la zone partagee   */
int     GoOn = 1;                 /* ->controle d'execution                   */
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
  printf("%s <K> <Te>\n", pgm_name );
  printf("Régule le niveau d'une cuve avec un gain statique K.\n");
  printf("<K>  : gain statique.\n");
  printf("<Te> : periode de régulation en s.\n");
  printf("\n");
  printf("Exemple : \n");
  printf("%s 0.5 0.01\n", pgm_name );
  printf("Régule le niveau de la cuve avec un gain");
  printf("de 0.5, et modifie le débit toutes les 0.01 s\n");
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
        *qe = (*yc - *y) * K;
    };

    /*................................*/
    /* arret du processus a reception */
    /* de SIGUSR1                     */
    /*................................*/
    if( signal == SIGUSR1 || *stop == 1)
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
    int                   fd_qe;      /* ->zone partagee DEBIT    */
    int                   fd_niveau;  /* ->zone partagee NIVEAU   */
    int                   fd_consigne;/* ->zone partagee CONSIGNE */
    int                   fd_stop;    /* ->zone partagee STOP     */
    
    /* verification des arguments */
    if( argc != 3 )
    {
        usage( argv[0] );
        return( 0 );
    };

    /* recuperation des arguments */
    if( (sscanf(argv[1],"%lf", &K     ) == 0)||
        (sscanf(argv[2],"%lf", &Te    ) == 0)   )  
    {
        printf("ERREUR : probleme de format des arguments\n");
        printf("         passe en ligne de commande.\n");
        usage( argv[0] );
        return( 0 );
    };

    //Vérification des arguments
    if(K<=0){
        printf("Pour que le régulateur fonctionne correctement,\n");
        printf("le gain doit être strictement positif.\n");
        return 0;
    }
    if(Te<=0){
        printf("La période d'échantillonage ne peut pas être inférieure ou égale à 0.\n");
        return 0;
    }


    /*................*/
    /* initialisation */
    /*................*/
    /* lecture des zones partagees */
    /*           --->DEBIT<-----    */
    fd_qe = shm_open(DEBIT, O_RDWR | O_CREAT, 0600);
    if( fd_qe < 0)
    {
        fprintf(stderr,"ERREUR : main() ---> appel a shm_open() DEBIT\n");
        fprintf(stderr,"        code d'erreur %d (%s)\n", 
                                errno, 
                                (char *)(strerror(errno)));
        return( -errno );
    };
    ftruncate( fd_qe, sizeof(double));
    qe =  (double *)mmap(NULL, 
                        sizeof(double), 
                        PROT_READ | PROT_WRITE, 
                        MAP_SHARED, 
                        fd_qe, 
                        0                         );
    /*         --->NIVEAU<---            */
    fd_niveau = shm_open(NIVEAU, O_RDWR | O_CREAT, 0600);
    if( fd_niveau < 0)
    {
        fprintf(stderr,"ERREUR : main() ---> appel a shm_open() NIVEAU\n");
        fprintf(stderr,"        code d'erreur %d (%s)\n", 
                                errno, 
                                (char *)(strerror(errno)));
        return( -errno );
    };
    ftruncate( fd_niveau, sizeof(double));
    y =  (double *)mmap(NULL, 
                        sizeof(double), 
                        PROT_READ, 
                        MAP_SHARED, 
                        fd_niveau, 
                        0                         );
    /*           --->CONSIGNE<-----    */
    fd_consigne = shm_open(CONSIGNE, O_RDWR | O_CREAT, 0600);
    if( fd_consigne < 0)
    {
        fprintf(stderr,"ERREUR : main() ---> appel a shm_open() CONSIGNE\n");
        fprintf(stderr,"        code d'erreur %d (%s)\n", 
                                errno, 
                                (char *)(strerror(errno)));
        return( -errno );
    };
    ftruncate( fd_consigne, sizeof(double));
    yc =  (double *)mmap(NULL, 
                        sizeof(double), 
                        PROT_READ, 
                        MAP_SHARED, 
                        fd_consigne, 
                        0                         );
    /*           --->STOP<-----    */
    fd_stop = shm_open(STOP, O_RDWR | O_CREAT, 0600);
    if( fd_stop < 0)
    {
        fprintf(stderr,"ERREUR : main() ---> appel a shm_open() STOP\n");
        fprintf(stderr,"        code d'erreur %d (%s)\n", 
                                errno, 
                                (char *)(strerror(errno)));
        return( -errno );
    };
    ftruncate( fd_stop, sizeof(int));
    stop =  (int *)mmap(NULL, 
                        sizeof(int), 
                        PROT_READ,
                        MAP_SHARED, 
                        fd_stop,
                        0               );//Only the second double of the memory



    /*Définition des signaux*/
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
    printf("SIMULATION :\n");
    do
    {
        pause();
        printf("qe = %lf\n", *qe);
    }
    while( GoOn == 1 );

    /* menage */
    close(fd_qe);
    close(fd_niveau);
    close(fd_consigne);
    close(fd_stop);
    
    /* fini */
    printf("FIN.\n");
    return( 0 );  /* ->on n'arrive pas jusque la en pratique */
}

  
