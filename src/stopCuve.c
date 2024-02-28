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
#define     PID         "PID"

/*....................*/
/* variables globales */
/*....................*/
pid_t  *pid;                     /* ->pid : pointeur sur deuxième partie de la zone partagee  */

/*#####################*/
/* programme principal */
/*#####################*/
int main( int argc, char *argv[])
{
    int                   fd_pid;     /* ->zone partagee PID */

    /*................*/
    /* initialisation */
    /*................*/
    /* lecture des zones partagees */
    /*           --->PID<-----    */
    fd_pid = shm_open(PID, O_RDWR | O_CREAT, 0600);
    if( fd_pid < 0)
    {
        fprintf(stderr,"ERREUR : main() ---> appel a shm_open() PID\n");
        fprintf(stderr,"        code d'erreur %d (%s)\n", 
                                errno, 
                                (char *)(strerror(errno)));
        return( -errno );
    };
    ftruncate( fd_pid, sizeof(double));
    pid =  (pid_t *)mmap(NULL, 
                        sizeof(pid_t)*2, 
                        PROT_READ, 
                        MAP_SHARED, 
                        fd_pid,
                        0               );//Both doubles of the memory


    //Lecture des PID
    pid_t pidCuve = *pid;
    pid_t pidReg = *(pid + 1);

    //Arrêt des processus
    kill(pidCuve, SIGUSR1);
    kill(pidReg, SIGUSR1);

    //Arrêt des fd
    close(fd_pid);
    
    /* fini */
    printf("FIN.\n");
    return( 0 );  /* ->on n'arrive pas jusque la en pratique */
}