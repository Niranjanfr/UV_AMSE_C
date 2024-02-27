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
/*...................*/
/* prototypes locaux */
/*...................*/
void usage( char *);           /* ->aide de ce programme                */
/*&&&&&&&&&&&&&&&&&&&&&&*/
/* aide de ce programme */
/*&&&&&&&&&&&&&&&&&&&&&&*/
void usage( char *pgm_name )
{
  if( pgm_name == NULL )
  {
    exit( -1 );
  };
  printf("%s <valeur>\n", pgm_name );
  printf("modifie la valeur du signal d'entree\n");
  printf("<valeur> : nouvelle valeur du signal d'entree\n");
  printf("\n");
  printf("exemple : \n");
  printf("%s 3.0\n", pgm_name );
  printf("impose 3.0 comme nouvelle valeur de l'entree\n");
}
/*#####################*/
/* programme principal */
/*#####################*/
int main( int argc, char *argv[])
{
  double                valeur;  /* ->valeur a ecrire dans la zone  */
  double                *u;      /* ->pointeur sur la zone partagee */
  int                   fd;      /* ->"file descriptor" sur la zone partagee */
  /* verification des arguments */
  if( argc != 2 )
  {
    usage( argv[0] );
    return( 0 );
  };
  /* recuperation des arguments */
  if( sscanf(argv[1],"%lf", &valeur ) == 0)
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
  fd = shm_open("ENTREE", O_RDWR , 0600);
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
  *u = valeur;      /* ->ecriture effective dans la zone partagee */
  close( fd );
  return( 0 );  /* ->on n'arrive pas jusque la en pratique */
}

  
