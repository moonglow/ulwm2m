#include <string.h>
#include "object_security.h"

static char sz_server_uri[48] =
{
  "coap://leshan.eclipseprojects.io:5683"
};

static uint8_t  bootstrap_server = 0;
static uint8_t  security_mode = 3; /* no security */

int security_read( struct t_lwm2m_data *parg )
{
  switch( parg->id )
  {
    case LWM2M_SECURITY_SERVER_URI:
      parg->data_type = LWM2M_ITEM_BINARY;
      parg->size = strlen( sz_server_uri );
      parg->data = sz_server_uri;
      break;
    case LWM2M_SECURITY_BOOTSTRAP_SERVER:
      parg->data_type = LWM2M_ITEM_UINT;
      parg->size = 1;
      parg->data = &bootstrap_server;
      break;
    case LWM2M_SECURITY_SECURITY_MODE:
      parg->data_type = LWM2M_ITEM_UINT;
      parg->size = 1;
      parg->data = &security_mode;
      break;
    case LWM2M_SECURITY_PUBLIC_KEY:
      parg->data_type = LWM2M_ITEM_BINARY;
      parg->size = 0;
      parg->data = 0;
      break;
    case LWM2M_SECURITY_SERVER_PUBLIC_KEY:
      parg->data_type = LWM2M_ITEM_BINARY;
      parg->size = 0;
      parg->data = 0;
      break;
    case LWM2M_SECURITY_SECRET_KEY:
      parg->data_type = LWM2M_ITEM_BINARY;
      parg->size = 0;
      parg->data = 0;
      break;
    default:
      return -1;
  }

  return 1;
}

