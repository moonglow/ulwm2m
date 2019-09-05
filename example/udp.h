#pragma once
#include <stdint.h>

int udp_init( int port );
int udp_exit( void );
uint32_t udp_get_ip( char *ip );

int udp_send( uint32_t ip, uint16_t port, uint8_t *p, uint16_t size );
int udp_recv( uint32_t *ip, uint16_t *port, uint8_t *p, uint16_t size );
uint32_t udp_timestamp( void );
