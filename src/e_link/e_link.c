/* -----------------------------------------------------------------------------
 * Component Name: E-Link
 * Author(s): William Erikssin
 * Purpose: Provide initialisation and an interface for the communications over
 *          the E-link to the ground station.
 * -----------------------------------------------------------------------------
 */

#include "global_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#include <limits.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <string.h>

static int sockfd, newsockfd, init_flag = 0;

pthread_mutex_t e_link_mutex = PTHREAD_MUTEX_INITIALIZER;

static void* thread_socket(void*);
static unsigned short sleep_time = 100;


int init_elink(void* args){

    pthread_mutex_lock( &e_link_mutex );

    return create_thread("e_link", thread_socket, 16);
}

int write_elink(char *buffer, int bytes){

    pthread_mutex_lock( &e_link_mutex );

    int n;

    n=write(newsockfd, buffer, bytes);

    if (n<0){
        logging(ERROR, "e_link", "ERROR sending to socket\n");

        do{
            sleep(1);
            n=write(newsockfd, buffer, bytes);
        } while (n<0);

        pthread_mutex_unlock( &e_link_mutex );
        return FAILURE;
    }

    usleep(sleep_time);
    pthread_mutex_unlock( &e_link_mutex );

    return SUCCESS;
}

int read_elink(char *buffer, int bytes){
    int n, bytes_avail;

    do{
        ioctl(sockfd, FIONREAD, &bytes_avail);
        sleep(1);
    } while(bytes_avail < bytes);

    n=read(newsockfd, buffer, bytes);

    if (n<0){
        logging(ERROR, "e_link", "ERROR reading from socket: %s\n", strerror(errno));
        return FAILURE;
    } else if(n>0){
        #ifdef E_LINK_DEBUG
        logging(DEBUG, "e_link", "Message read from GS: %s", buffer);
        #endif
    }

    return SUCCESS;
}

static void* thread_socket(void* param){

    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    do{
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if(sockfd < 0){
            logging(ERROR, "e_link", "Can't open socket");
        }
        sleep(1);
    } while (sockfd < 0);
    logging(INFO, "e_link", "Socket open");

    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    while (1){
        serv_addr.sin_port = htons(SERVER_PORT);
        if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
            logging(ERROR, "e_link", "Can't bind socket: %s", strerror(errno));
            sleep(1);
            serv_addr.sin_port = htons(SERVER_PORT_BACKUP);
            if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
                logging(ERROR, "e_link", "Can't bind socket: %s", strerror(errno));
                continue;
            }
        }
        break;
    }
    logging(INFO, "e_link", "Binded to socket");

    listen(sockfd, 5);

    #ifdef E_LINK_DEBUG
    logging(DEBUG, "e_link", "Listening for groundstation");
    #endif


    clilen = sizeof(cli_addr);

    while(1){    
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if(newsockfd < 0){
            logging(ERROR, "e_link", "Error when accepting GS");
            continue;
        }
        logging(INFO, "e_link", "Accepted connection to GS");
        pthread_mutex_unlock( &e_link_mutex );
        init_flag = 1;
    }

    return SUCCESS;
}

void close_socket( void ){
    close(newsockfd);
    close(sockfd);
}

int set_datarate (unsigned short datarate){
    sleep_time=datarate;

    return SUCCESS;
}
