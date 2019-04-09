/* -----------------------------------------------------------------------------
 * Component Name: Downlink
 * Parent Component: Telemetry
 * Author(s): Adam Smialek
 * Purpose: Read telemetry messages from the queue and send them over the
 *          downlink whenever possible.
 * -----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h> // strlen()
#include <arpa/inet.h> // inet_pton()
#include <sys/socket.h>
#include <unistd.h>

#include "global_utils.h"

#define SERVER_PORT 8888
#define SERVER_IP   "127.0.0.1"

static int udp_socket_;
static struct sockaddr_in UDP_SERVER;

/* prototypes declaration */
int init_upd_socket( void );


int init_downlink( void ){
    int ret = init_upd_socket();
    if( ret != SUCCESS){
        return ret;
    }
    return SUCCESS;
}

/**
 * Sets up the socket on `SERVER_PORT` to work with UDP protocol.
 *
 * I have decided not to put this in `init_downlink()` for the function name to stay
 * as much related to wwhat happens inside as possible and to make it more abstract.
 * Just a quick digression and a heads up for further "standards", for anyone who
 * reads these comments.
 *
 * \return
 */
int init_upd_socket( void )
{
    int ret;

    UDP_SERVER.sin_family = AF_INET;
    UDP_SERVER.sin_port = htons( SERVER_PORT );
    if( (ret = inet_pton( AF_INET, SERVER_IP, &UDP_SERVER.sin_addr )) <= 0 )
    {
        fprintf(stderr,
                "Failed inet_pton of telemetry component. "
                "Return value: %d\n", ret);
        return ret;
    }

    udp_socket_ = socket( AF_INET, SOCK_DGRAM, 0 );
    if( udp_socket_ < 0 )
    {
        fprintf(stderr,
                "Failed socket creation of telemetry component. "
                "Return value: %d\n", udp_socket_);
        return udp_socket_;
    }

    return SUCCESS;
}

/**
 * Sends data via UDP protocol through the `socket_`.
 *   \param buffer
 *   \return
 *
 *   \todo replace `char buffer` with more fitting data type
 *   \todo remove `fprintf` statements
 */
int send_data_packet(char buffer[]) {
    printf( "|Message for server|: %s \n", buffer );

    socklen_t len = sizeof( UDP_SERVER );

    int ret;

    if( (ret = sendto( udp_socket_, buffer, strlen( buffer ), 0,( struct sockaddr * ) & UDP_SERVER, len )) < 0 )
    {
        fprintf(stderr,
                "Failed to send telemetry. "
                "Return value: %d\n", ret);
        return ret;
    }

    return SUCCESS;
}
