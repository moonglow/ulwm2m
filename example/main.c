#include <object_security.h>
#include <object_server.h>
#include <object_device.h>
#include "object_3300.h"
#include "object_3303.h"
#include "object_3341.h"
#include "udp.h"
#include <stdlib.h>

#ifdef WITH_DTLS
#define COAP_DTLS_PSK_DEFAULT_IDENTITY "user"
#define COAP_DTLS_PSK_DEFAULT_KEY      "password"
#define LOG_MODULE "main"
#define LOG_LEVEL  LOG_LEVEL_NONE
#include <dtls-log.h>
#include <tinydtls.h>
#include <dtls.h>
static dtls_handler_t cb;
static dtls_context_t *dtls_context = NULL;
static session_t session = { 0 };
int    secure_connection = 0;
#endif

static uint32_t udp_ip = 0;
static uint16_t udp_port = 0;
static uint8_t shared_mem[0x100];
struct t_lwm2m lwm2m;

const struct t_lwm2m_obj base = 
{
    .id = LWM2M_SECURITY_OBJECT,
    .instances = LWM2M_SET_INSTANCE( 1 ),
    .read = security_read,
    /* can be changed only by bootstrap server */
    .write = 0,
    .exec = 0,
    .observe = 0,

  .next = (void*)&(const struct t_lwm2m_obj)
  {
    .id = LWM2M_SERVER_OBJECT,
    .instances = LWM2M_SET_INSTANCE(1),
    .read = server_read,
    .write = server_write,
    .exec = server_exec,
    .observe = 0,

  .next = (void*)&(const struct t_lwm2m_obj)
  {
    .id = LWM2M_DEVICE_OBJECT,
    .instances = LWM2M_SET_INSTANCE( 1 ),
    .read = device_read,
    .write = device_write,
    .exec = device_exec,
    .observe = 0,

  .next = (void*) &(const struct t_lwm2m_obj)
  {
    .id = 3303,
    .instances = LWM2M_SET_INSTANCE( 1 ),
    .read = object_3303_read,
    .write = object_3303_write,
    .exec = object_3303_exec,
    .observe = object_3303_observe,

  .next = (void*) &(const struct t_lwm2m_obj)
  {
    .id = 3341,
    .instances = LWM2M_SET_INSTANCE( 1 ),
    .read = object_3341_read,
    .write = object_3341_write,
    .exec = object_3341_exec,
    .observe = 0,

  .next = (void*) &(const struct t_lwm2m_obj)
  {

    .id = 3300,
    /* 7 instance device */
    .instances = LWM2M_SET_INSTANCE(7 ),
    .read = object_3300_read,
    .write = object_3300_write,
    .exec = object_3300_exec,
    .observe = object_3300_observe,

  }
  }
  }
  }
  },
};

int network_recv( uint8_t *data, int size, int timeout )
{
#ifdef WITH_DTLS
  if( secure_connection )
    return 0; /* no raw data */
  else
#endif
    return udp_recv( 0, 0, data, size, timeout );
}

int network_send( uint8_t *data, int size )
{
#ifdef WITH_DTLS
  if( secure_connection )
    return dtls_write( dtls_context, &session, data, size );
  else
#endif
    return udp_send( udp_ip, udp_port, data, size );
}

int network_init( char *psz_host, int port, int is_secure )
{
  udp_exit();

  udp_init( port );

  udp_ip = udp_get_ip( psz_host );
  udp_port = port;

#ifdef WITH_DTLS
  secure_connection = is_secure;
#else
  if( is_secure )
    return -1;
#endif
  return 0;
}

int main(int argc, char *argv[])
{
  (void)argc;
  (void)argv;

  int res;

  srand( udp_timestamp() );

  lwm2m_init( &lwm2m, (void*)&base, shared_mem, sizeof( shared_mem ) );

  lwm2m.init = network_init;
  lwm2m.recv = network_recv;
  lwm2m.send = network_send;

  lwm2m.sz_endpoint_name = "ulwm2m_demo_client";

#ifdef WITH_DTLS
    dtls_context = dtls_new_context( 0 );
    dtls_set_handler( dtls_context, &cb );
    dtls_init();
#endif

  for( ;; )
  {
    res = udp_recv( 0, 0, 0, 0, 250 );
    if( res >= 0 )
    {
#ifdef WITH_DTLS
      if( secure_connection )
      {
        res = udp_recv( 0, 0, shared_mem, sizeof( shared_mem ), 100 );
        dtls_handle_message( dtls_context, &session, shared_mem, res );
      }
      else
#endif
      {
        if( lwm2m_process( &lwm2m, LWM2M_EVENT_RX, udp_timestamp() ) < 0 )
          break;
      }
    }
    else
    {
      if( lwm2m_process( &lwm2m, LWM2M_EVENT_IDLE, udp_timestamp() ) < 0 )
        break;
    }
  }
}

#ifdef WITH_DTLS
static int input_from_peer( struct dtls_context_t *p, session_t *s, uint8_t *d, size_t l )
{
  (void)p;
  (void)s;

  if( lwm2m.mem_size < l )
    return -1;

  memcpy( lwm2m.mem, d, l );

  if( lwm2m_process( &lwm2m, LWM2M_SET_EVENT_ARG( LWM2M_EVENT_RX, l ), udp_timestamp() ) < 0 )
    return -1;

  return 0;
}

static int output_to_peer( struct dtls_context_t *p, session_t *s, uint8_t *d, size_t l )
{
  (void)p;
  (void)s;

  return udp_send( udp_ip, udp_port, d, l );
}

static int get_psk_info( struct dtls_context_t *ctx, const session_t *session, 
                          dtls_credentials_type_t type, const unsigned char *id, size_t id_len,
                          unsigned char *result, size_t result_length
)
{
  (void)ctx;
  (void)session;
  (void)id;
  (void)id_len;

  size_t len;

  switch (type)
  {
    case DTLS_PSK_HINT:
      break;
    case DTLS_PSK_IDENTITY:
      /* ignore identity hint */
      len = strlen( COAP_DTLS_PSK_DEFAULT_IDENTITY );
      if( result_length < len )
        break;
      memcpy( result, COAP_DTLS_PSK_DEFAULT_IDENTITY, len );
      return len;
    case DTLS_PSK_KEY:
      len = strlen( COAP_DTLS_PSK_DEFAULT_KEY );
      if( result_length < len )
        break;
      memcpy( result, COAP_DTLS_PSK_DEFAULT_KEY, len );
      return len;
  }

  return dtls_alert_fatal_create( DTLS_ALERT_INTERNAL_ERROR );
}

static int dtls_event(struct dtls_context_t *ctx, session_t *session,
                      dtls_alert_level_t level, unsigned short code)
{

  (void)ctx;
  (void)session;
  (void)level;

  switch( code )
  {
    case DTLS_EVENT_CONNECTED:
      /* restart registration over secure link */
      lwm2m_process( &lwm2m, LWM2M_EVENT_RESET, udp_timestamp() );
      LOG_OUTPUT( "+DTLS_EVENT_CONNECTED\n" );
      break;
  }
  return 0;
}

static dtls_handler_t cb =
{
  .write = output_to_peer,
  .read = input_from_peer,
  .event = dtls_event,
  .get_psk_info = get_psk_info,
};
#endif
