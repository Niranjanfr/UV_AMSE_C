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
#define     STOP         "STOP"

/*....................*/
/* variables globales */
/*....................*/
int  *stop;                     /* ->stop : pointeur sur deuxième partie de la zone partagee  */

/*#####################*/
/* programme principal */
/*#####################*/
int main( int argc, char *argv[])
{
    int                   fd_stop;     /* ->zone partagee stop */

    /*................*/
    /* initialisation */
    /*................*/
    /* lecture des zones partagees */
    /*           --->KILL<-----    */
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
                        PROT_READ | PROT_WRITE, 
                        MAP_SHARED, 
                        fd_stop,
                        0               );

    //Arrêt des processus
    *stop = 1;

    //On attend un peu puis on remet stop à sa valeur initiale
    sleep(2);

    *stop = 0;

    //Arrêt des fd
    close(fd_stop);
    
    /* fini */
    printf("FIN.\n");
    return( 0 );  /* ->on n'arrive pas jusque la en pratique */
}