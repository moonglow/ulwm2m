#include <object_security.h>
#include <object_server.h>
#include <object_device.h>
#include "object_3300.h"
#include "object_3303.h"
#include "object_3341.h"
#include "udp.h"
#include <stdlib.h>

static uint32_t udp_ip = 0;
static uint16_t udp_port = 0;
static uint8_t shared_mem[0x100];

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
  return udp_recv( 0, 0, data, size, timeout );
}

int network_send( uint8_t *data, int size )
{
  return udp_send( udp_ip, udp_port, data, size );
}

int network_init( char *psz_host, int port, int is_secure )
{
  (void)is_secure;

  udp_exit();

  udp_init( 0 );

  udp_ip = udp_get_ip( psz_host );
  udp_port = port;

  return 0;
}

int main(int argc, char *argv[])
{
  (void)argc;
  (void)argv;

  int res, event = LWM2M_EVENT_IDLE;
  struct t_lwm2m lwm2m;

  srand( udp_timestamp() );

  lwm2m_init( &lwm2m, (void*)&base, shared_mem, sizeof( shared_mem ) );

  lwm2m.init = network_init;
  lwm2m.recv = network_recv;
  lwm2m.send = network_send;

  lwm2m.sz_endpoint_name = "ulwm2m_demo_client";
  for(;;)
  {
    res = lwm2m_process( &lwm2m, event, udp_timestamp() );
    if( res < 0 )
      break;

    /* just poll for RX data */
    res = lwm2m.recv( 0, 0, 100 );
    if( res < 0 )
      event = LWM2M_EVENT_IDLE;
    else if( res == 0 )
      event = LWM2M_EVENT_RX;
  }

  return 0;
}
