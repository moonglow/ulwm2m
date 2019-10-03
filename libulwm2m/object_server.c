/*********************************************************************\
* Copyleft (>) 2019 Roman Ilichev <fxdteam@gmail.com>                 *
*                                                                     *
* This file is part of uLWM2M project                                 *
*                             WTFPL LICENSE v2                        *
\*********************************************************************/
#include <string.h>
#include "object_server.h"

static uint16_t r_short_server_id = 1;
static uint32_t        rw_lifetime = 120;
static uint8_t         rw_storing = 0;
static char            rw_binding[4] = "U";
static char            e_reg_update_trigger = 0;

static const uint16_t id_list[] =
{
  LWM2M_SERVER_SHORT_SERVER_ID, LWM2M_SERVER_LIFETIME,
  LWM2M_SERVER_STORING, LWM2M_SERVER_BINDING,
  LWM2M_SERVER_UPDATE_TRIGGER
};

int server_read( struct t_lwm2m_data *p_data )
{
  switch( p_data->id )
  {
    case LWM2M_SERVER_SHORT_SERVER_ID:
      p_data->data_type = LWM2M_ITEM_UINT;
      p_data->size = 2;
      p_data->data = &r_short_server_id;
      break;
    case LWM2M_SERVER_LIFETIME:
      p_data->data_type = LWM2M_ITEM_UINT;
      p_data->size = 4;
      p_data->data = &rw_lifetime;
      break;
    case LWM2M_SERVER_STORING:
      p_data->data_type = LWM2M_ITEM_UINT;
      p_data->size = 1;
      p_data->data = &rw_storing;
      break;
    case LWM2M_SERVER_BINDING:
      p_data->data_type = LWM2M_ITEM_BINARY;
      p_data->size = (int)strlen(rw_binding );
      p_data->data = rw_binding;
      break;
    /* read all, return id list */
    case LWM2M_GET_ID_LIST:
      p_data->data_type = LWM2M_ITEM_ID_LIST;
      p_data->data = (void*)id_list;
      p_data->size = ARRAY_NELEMS( id_list );
      break;
    default:
      return -1;
  }
  
  return 1;
}

int server_write( struct t_lwm2m_data *p_data )
{
  switch( p_data->id )
  {
    case LWM2M_SERVER_LIFETIME:
      rw_lifetime = lwm2m_read_item_int(p_data );
      return 1;
    case LWM2M_SERVER_STORING:
      rw_storing = (uint8_t)lwm2m_read_item_int(p_data );
      return 1;
    case LWM2M_SERVER_BINDING:
      if(lwm2m_read_item_string(p_data, rw_binding, sizeof( rw_binding ) ) == 0 )
        return -1;
      return 1;
    default:
      return -1;
  }
}

int server_exec( struct t_lwm2m_data *p_data )
{
  switch( p_data->id )
  {
    case LWM2M_SERVER_UPDATE_TRIGGER:
      (void)e_reg_update_trigger;
      return 1;
    default:
      return -1;
  }
}

