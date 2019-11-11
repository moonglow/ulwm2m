#include "dtls-support.h"
#include <udp.h>
#include <stdlib.h>

#define LOG_MODULE "dtls-support"
#define LOG_LEVEL  LOG_LEVEL_DTLS
#include "dtls-log.h"

static dtls_context_t the_dtls_context;
static dtls_cipher_context_t cipher_context;

dtls_context_t *dtls_context_acquire(void)
{
  return &the_dtls_context;
}

void dtls_context_release(dtls_context_t *context)
{
  (void)context;
}

dtls_cipher_context_t *dtls_cipher_context_acquire(void)
{
  return &cipher_context;
}


void dtls_cipher_context_release(dtls_cipher_context_t *c)
{
  (void)c;
}


void dtls_session_init(session_t *sess)
{
  memset( sess, 0, sizeof( session_t ) );
}


int dtls_session_equals(const session_t *a, const session_t *b)
{
  return a->port == b->port && ( a->addr == b->addr );
}

void *dtls_session_get_address(const session_t *a)
{
  return (void *)a;
}

int dtls_session_get_address_size(const session_t *a)
{
  return sizeof(*a);
}

void dtls_session_log(const session_t *addr)
{
  LOG_OUTPUT("[");
  LOG_OUTPUT( "%x", addr->addr );
  LOG_OUTPUT("]");
}

void dtls_session_print( const session_t *addr )
{
  printf("[");
  LOG_OUTPUT( "%x", addr->addr );
  printf("]");
}

int dtls_fill_random( uint8_t *buf, size_t len )
{
  if( !buf )
    return 0;
  
  while( len-- )
    buf[len] = rand() & 0xff;
  
  return 1;
}

void dtls_ticks( dtls_tick_t *t )
{
  if( t )
    *t = udp_timestamp();
}

void dtls_set_retransmit_timer(dtls_context_t *context, unsigned int time )
{
  (void)context;
  (void)time;
}

void dtls_support_init(void)
{

}
