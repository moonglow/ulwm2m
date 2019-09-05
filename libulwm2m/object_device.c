#include "object_device.h"

const char r_sz_manufacturer[] = "bitbucket.org/moonglow";
const char r_sz_model[] = "lwm2m C lib";
const char r_sz_serial[] = "0123456789";
const char r_sz_fw_version[] = __DATE__;
uint8_t r_err_codes = 0;
const char r_sz_bindings[] = "U";

static const uint16_t id_0_list[] = 
{
  LWM2M_DEVICE_MANUFACTURER, LWM2M_DEVICE_MODEL_NUMBER,
  LWM2M_DEVICE_SERIAL_NUMBER, LWM2M_DEVICE_FIRMWARE_VERSION,
  LWM2M_DEVICE_REBOOT, LWM2M_DEVICE_ERROR_CODE,
  LWM2M_DEVICE_BINDINDS,
};

int device_read( struct t_lwm2m_item *parg )
{
  switch( parg->id )
  {
    case LWM2M_SERVER_SHORT_SERVER_ID:
      parg->data_type = LWM2M_ITEM_BINARY;
      parg->size = sizeof( r_sz_manufacturer )-1;
      parg->data = (void*)r_sz_manufacturer;
      break;
    case LWM2M_DEVICE_MODEL_NUMBER:
      parg->data_type = LWM2M_ITEM_BINARY;
      parg->size = sizeof( r_sz_model )-1;
      parg->data = (void*)r_sz_model;
      break;
    case LWM2M_DEVICE_SERIAL_NUMBER:
      parg->data_type = LWM2M_ITEM_BINARY;
      parg->size = sizeof( r_sz_serial )-1;
      parg->data = (void*)r_sz_serial;
      break;
    case LWM2M_DEVICE_FIRMWARE_VERSION:
      parg->data_type = LWM2M_ITEM_BINARY;
      parg->size = sizeof( r_sz_fw_version )-1;
      parg->data = (void*)r_sz_fw_version;
      break;
    case LWM2M_DEVICE_ERROR_CODE:
      parg->data_type = LWM2M_ITEM_UINT;
      parg->size = sizeof( r_err_codes );
      parg->data = &r_err_codes;
      break;
    case LWM2M_DEVICE_BINDINDS:
      parg->data_type = LWM2M_ITEM_BINARY;
      parg->size = sizeof( r_sz_bindings ) - 1;
      parg->data = (void*)r_sz_bindings;
      break;
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

int device_write( struct t_lwm2m_item *parg )
{
  (void)parg;
  return -1;
}

int device_exec( struct t_lwm2m_item *parg )
{
  switch( parg->id )
  {
    case LWM2M_DEVICE_REBOOT:
      ;
    break;
    default:
      return -1;
  }
  return 1;
}

int device_create( struct t_lwm2m_item *parg )
{
  (void)parg;
  return -1;
}

int device_delete( struct t_lwm2m_item *parg )
{
  (void)parg;
  return -1;
}
