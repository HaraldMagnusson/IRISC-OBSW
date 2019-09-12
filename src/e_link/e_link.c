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


#define SERVER_PORT 1337

static int sockfd, newsockfd, init_flag = 0;

pthread_mutex_t e_link_mutex = PTHREAD_MUTEX_INITIALIZER;

//Thread
static pthread_t thread_read_e_link, thread_socket_t;
static pthread_attr_t thread_attr;
static struct sched_param param;
static int ret;

static void* thread_read(void*);
static void* thread_socket(void*);
static unsigned short sleep_time = 500;


int init_elink( void ){

    /*
     *  --Thread
     */

    pthread_mutex_lock( &e_link_mutex );

    ret = pthread_attr_init(&thread_attr);
    if( ret != 0 ){
        fprintf(stderr,
            "Failed pthread_attr_init for e_link component. "
            "Return value: %d\n", ret);
        return ret;
    }

    ret = pthread_attr_setstacksize(&thread_attr, PTHREAD_STACK_MIN);
    if( ret != 0 ){
        fprintf(stderr,
            "Failed pthread_attr_setstacksize of e_link component. "
            "Return value: %d\n", ret);
        return ret;
    }

    ret = pthread_attr_setschedpolicy(&thread_attr, SCHED_FIFO);
    if( ret != 0 ){
        fprintf(stderr,
            "Failed pthread_attr_setschedpolicy of e_link component. "
            "Return value: %d\n", ret);
        return ret;
    }

    param.sched_priority = 50;
    ret = pthread_attr_setschedparam(&thread_attr, &param);
    if( ret != 0 ){
        fprintf(stderr,
            "Failed pthread_attr_setschedparam of e_link component. "
            "Return value: %d\n", ret);
        return ret;
    }

    ret = pthread_attr_setinheritsched(&thread_attr, PTHREAD_EXPLICIT_SCHED);
    if( ret != 0 ){
        fprintf(stderr,
            "Failed pthread_attr_setinheritsched of e_link component. "
            "Return value: %d\n", ret);
        return ret;
    }

    ret = pthread_create(&thread_socket_t, &thread_attr, thread_socket, NULL);
    if( ret != 0 ){
        fprintf(stderr,
            "Failed pthread_create of socket component. "
            "Return value: %d\n", ret);
        return ret;
    }

    

    ret = pthread_create(&thread_read_e_link, &thread_attr, thread_read, NULL);
    if( ret != 0 ){
        fprintf(stderr,
            "Failed pthread_create of e-link component. "
            "Return value: %d\n", ret);
        return ret;
    } 

    return SUCCESS;
}

int write_elink(char *buffer, int bytes){

    pthread_mutex_lock( &e_link_mutex );

    //printf("----Sending packet----\n");

    int n;

    n=write(newsockfd, buffer, bytes);

    if (n<0){
        printf("ERROR sending to socket\n");

        do{
            sleep(1);
            n=write(newsockfd, buffer, bytes);
        } while (n<0);

        pthread_mutex_unlock( &e_link_mutex );
        return FAILURE;
    }
    //printf("Message sent:\n");

    usleep(sleep_time);
    pthread_mutex_unlock( &e_link_mutex );

    return SUCCESS;
}

static void* thread_read(void* param){
    char buffer[256];
    int n;
    while(1){
        sleep(1);
        if(init_flag==1){
            break;
        }

    }

    while(1){
        memset(&buffer[0], 0, sizeof(buffer));
        n=read(newsockfd, buffer, 255);
        fflush(stdout);
        if (n<0){
            printf("ERROR reading from socket: %s\n", strerror(errno));
        } else if(n>0){
            printf("Message read: %s\n", buffer);
        }

    }

    return SUCCESS;
}

char* read_elink(int bytes){
    int n, bytes_avail;
    char *buffer = malloc(bytes);

    do{
        ioctl(sockfd, FIONREAD, &bytes_avail);
        usleep(1);
    } while(bytes_avail < bytes);

    n=read(newsockfd, buffer, bytes);
    fflush(stdout);
    if (n<0){
        printf("ERROR reading from socket: %s\n", strerror(errno));
    } else if(n>0){
        printf("Message read: %s\n", buffer);
    }

    return buffer;
}

static void* thread_socket(void* param){

    int portno;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        printf("ERROR opening socket\n");
    }
    printf("Socket open\n");

    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = SERVER_PORT;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        printf("ERROR on binding: %s\n", strerror(errno));
    }
    printf("Binded\n");

    listen(sockfd, 5);

    printf("Listen\n");
    clilen = sizeof(cli_addr);

    while(1){    
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if(newsockfd < 0){
            printf("ERROR on accept\n");
        }
        printf("Accepted\n");
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
