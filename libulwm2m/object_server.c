#include <string.h>
#include "object_server.h"

static uint16_t r_short_server_id = 1;
uint32_t        rw_lifetime = 120;
uint8_t         rw_storing = 0;
char            rw_binding[4] = "U";
char            e_reg_update_trigger = 0;

static const uint16_t id_0_list[] = 
{
  LWM2M_SERVER_SHORT_SERVER_ID, LWM2M_SERVER_LIFETIME,
  LWM2M_SERVER_STORING, LWM2M_SERVER_BINDING,
  LWM2M_SERVER_UPDATE_TRIGGER
};

int server_read( struct t_lwm2m_data *parg )
{
  switch( parg->id )
  {
    case LWM2M_SERVER_SHORT_SERVER_ID:
      parg->data_type = LWM2M_ITEM_UINT;
      parg->size = 2;
      parg->data = &r_short_server_id;
      break;
    case LWM2M_SERVER_LIFETIME:
      parg->data_type = LWM2M_ITEM_UINT;
      parg->size = 4;
      parg->data = &rw_lifetime;
      break;
    case LWM2M_SERVER_STORING:
      parg->data_type = LWM2M_ITEM_UINT;
      parg->size = 1;
      parg->data = &rw_storing;
      break;
    case LWM2M_SERVER_BINDING:
      parg->data_type = LWM2M_ITEM_BINARY;
      parg->size = strlen( rw_binding );
      parg->data = rw_binding;
      break;
    /* read all, return id list */
    case LWM2M_GET_ID_LIST:
      parg->data_type = LWM2M_ITEM_ID_LIST;
      parg->data = (void*)id_0_list;
      parg->size = sizeof( id_0_list );
      break;
    default:
      return -1;
  }
  
  return 1;
}

int server_write( struct t_lwm2m_data *parg )
{
  switch( parg->id )
  {
    case LWM2M_SERVER_LIFETIME:
      rw_lifetime = lwm2m_read_item_int( parg );
      return 1;
    case LWM2M_SERVER_STORING:
      rw_storing = (uint8_t)lwm2m_read_item_int( parg );
      return 1;
    case LWM2M_SERVER_BINDING:
      if( lwm2m_read_item_string( parg, rw_binding, sizeof( rw_binding ) ) == 0 )
        return -1;
      return 1;
    default:
      return -1;
  }
}

int server_exec( struct t_lwm2m_data *parg )
{
  switch( parg->id )
  {
    case LWM2M_SERVER_UPDATE_TRIGGER:
      return 1;
    default:
      return -1;
  }
}

