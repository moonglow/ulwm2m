/*********************************************************************\
* Copyleft (>) 2019 Roman Ilichev <fxdteam@gmail.com>                 *
*                                                                     *
* This file is part of uLWM2M project                                 *
*                             WTFPL LICENSE v2                        *
\*********************************************************************/
#include <string.h>
#include "object_security.h"

static char sz_server_uri[48] =
{
  "coaps://leshan.eclipseprojects.io:5684"
};

static uint8_t  bootstrap_server = 0;
static uint8_t  security_mode = 3; /* no security */

int security_read( struct t_lwm2m_data *p_data )
{
  switch( p_data->id )
  {
    case LWM2M_SECURITY_SERVER_URI:
      p_data->data_type = LWM2M_ITEM_BINARY;
      p_data->size = (int)strlen(sz_server_uri );
      p_data->data = sz_server_uri;
      break;
    case LWM2M_SECURITY_BOOTSTRAP_SERVER:
      p_data->data_type = LWM2M_ITEM_UINT;
      p_data->size = 1;
      p_data->data = &bootstrap_server;
      break;
    case LWM2M_SECURITY_SECURITY_MODE:
      p_data->data_type = LWM2M_ITEM_UINT;
      p_data->size = 1;
      p_data->data = &security_mode;
      break;
    case LWM2M_SECURITY_PUBLIC_KEY:
      p_data->data_type = LWM2M_ITEM_BINARY;
      p_data->size = 0;
      p_data->data = 0;
      break;
    case LWM2M_SECURITY_SERVER_PUBLIC_KEY:
      p_data->data_type = LWM2M_ITEM_BINARY;
      p_data->size = 0;
      p_data->data = 0;
      break;
    case LWM2M_SECURITY_SECRET_KEY:
      p_data->data_type = LWM2M_ITEM_BINARY;
      p_data->size = 0;
      p_data->data = 0;
      break;
    default:
      return -1;
  }

  return 1;
}

