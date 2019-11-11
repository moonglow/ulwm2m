#if _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#elif __ICCARM__
#warning just empty stubs
#else
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#define closesocket( _x ) close( _x )
#endif

#include <time.h>
#include "udp.h"

static int udp_socket = -1;

static int socket_set_rx_buffer_size( int sd, int size )
{
#ifdef __ICCARM__
  return 0;
#else
  return setsockopt( sd, SOL_SOCKET, SO_RCVBUF, (char*)&size, sizeof( size ) );
#endif
}

int socket_wait_data( int fd, int ms )
{
#ifdef __ICCARM__
  return 0;
#else
  fd_set rd;
  int res;
  struct timeval tv = { 0 };

  FD_ZERO( &rd );

  FD_SET( fd, &rd );

  tv.tv_sec = ms/1000;
  tv.tv_usec = (ms%1000)*1000;

  res = select( fd+1, &rd, 0, 0, &tv );
  if( res <= 0 )
    return res;

  if( !FD_ISSET( fd, &rd ) )
    return -1;

  return res;
#endif
}

int udp_init( int port )
{
#ifdef __ICCARM__
  (void)udp_socket;
  (void)socket_set_rx_buffer_size;
  return 0;
#else
  struct sockaddr_in src;
  int res;

#if _WIN32
  {
    WSADATA wsaData;
    WSAStartup( MAKEWORD(2, 2), &wsaData );
  }
#endif

  udp_socket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
  if( udp_socket < 0 )
    return udp_socket;

  res = socket_set_rx_buffer_size( udp_socket, (512*1024) );
  if( res < 0 )
    return -1;

  src.sin_family = AF_INET;
  src.sin_port = htons( port );
  src.sin_addr.s_addr = htonl( INADDR_ANY );

  int set_option_on = 1;
  res = setsockopt( udp_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&set_option_on, sizeof(set_option_on) );
  if( res < 0 )
  {
    closesocket( udp_socket );
    return -2;
  }

  res = bind( udp_socket, (struct sockaddr *)&src, sizeof (src) );
  if ( res < 0)
  {
    closesocket( udp_socket );
    return -3;
  }

  return 0;
#endif
}

int udp_exit( void )
{
#ifndef __ICCARM__
  closesocket( udp_socket );
#endif
  return 0;
}

uint32_t udp_get_ip( char *ip )
{
#ifdef __ICCARM__
  return 0;
#else
  struct hostent *p_h = gethostbyname( ip );
  if( p_h )
    return ((struct in_addr*)p_h->h_addr_list[0])->s_addr;
  else
    return inet_addr( ip );
#endif
}

int udp_send( uint32_t ip, uint16_t port, uint8_t *p, uint16_t size )
{
#ifdef __ICCARM__
  return 0;
#else
  struct sockaddr_in addr = { 0 };

  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = ip;
  addr.sin_port = htons( port );

  return sendto( udp_socket, (void*)p, size, 0, (struct sockaddr*)&addr, sizeof(addr) );
#endif
}

int udp_recv( uint32_t *ip, uint16_t *port, uint8_t *p, uint16_t size, int timeout )
{
#ifdef __ICCARM__
  return 0;
#else
  int res;
  struct sockaddr_in  who = { 0 };

  res = socket_wait_data( udp_socket, timeout );
  if( res <= 0 )
    return -1;

  if( !size )
    return 0;

  res = sizeof( who );
  res = recvfrom( udp_socket, (void*)p, size, 0, (struct sockaddr*)&who, (void*)&res );

  if( res < 0 )
    return -1;

  if( ip )
    *ip = who.sin_addr.s_addr;
  if( port )
    *port = ntohs( who.sin_port );

  return res;
#endif
}

uint32_t udp_timestamp( void )
{
#if _WIN32
  return GetTickCount();
#elif __ICCARM__
  return 0;
#else
  struct timespec ts;
  clock_gettime( CLOCK_MONOTONIC, &ts );

  return (ts.tv_nsec / 1000000) + ( ts.tv_sec * 1000 );
#endif
}
