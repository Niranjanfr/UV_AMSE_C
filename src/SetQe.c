/*====================================================*/
/* programme pour la modification du debit d'entree   */
/* dans la cuve.                                      */
/*____________________________________________________*/
/* Jacques BOONAERT-LEPERS  evaluation AMSE 2023-2024 */
/*====================================================*/
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
/*............................*/
/* constante de l'application */
/*............................*/
#define DEBIT       "DEBIT"
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
  printf("modifie la valeur du debit entrant\n");
  printf("<valeur> : nouvelle valeur du debit entrant (m3)\n");
  printf("\n");
  printf("exemple : \n");
  printf("%s 0.027\n", pgm_name );
  printf("impose 0.027 comme nouvelle valeur du debit entrant\n");
}
/*#####################*/
/* programme principal */
/*#####################*/
int main( int argc, char *argv[])
{
  double                valeur;  /* ->valeur a ecrire dans la zone  */
  double                *qe;      /* ->pointeur sur la zone partagee */
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
  fd = shm_open(DEBIT, O_RDWR , 0600);
  if( fd < 0)
  {
    fprintf(stderr,"ERREUR : main() ---> appel a shm_open()\n");
    fprintf(stderr,"        code d'erreur %d (%s)\n", 
                            errno, 
                            (char *)(strerror(errno)));
    return( -errno );
  };
  ftruncate( fd, sizeof(double));
  qe =  (double *)mmap(NULL, 
                      sizeof(double), 
                      PROT_READ | PROT_WRITE, 
                      MAP_SHARED, 
                      fd, 
                      0                         );
  *qe = valeur;      /* ->ecriture effective dans la zone partagee */
  close( fd );
  return( 0 );  /* ->on n'arrive pas jusque la en pratique */
}

  
