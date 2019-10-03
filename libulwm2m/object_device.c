/*********************************************************************\
* Copyleft (>) 2019 Roman Ilichev <fxdteam@gmail.com>                 *
*                                                                     *
* This file is part of uLWM2M project                                 *
*                             WTFPL LICENSE v2                        *
\*********************************************************************/
#include "object_device.h"

static const char r_sz_manufacturer[] = "bitbucket.org/moonglow";
static const char r_sz_model[] = "lwm2m C lib";
static const char r_sz_serial[] = "0123456789";
static const char r_sz_fw_version[] = __DATE__;
static uint8_t r_err_codes = 0;
static const char r_sz_bindings[] = "U";

static const uint16_t id_list[] =
{
  LWM2M_DEVICE_MANUFACTURER, LWM2M_DEVICE_MODEL_NUMBER,
  LWM2M_DEVICE_SERIAL_NUMBER, LWM2M_DEVICE_FIRMWARE_VERSION,
  LWM2M_DEVICE_REBOOT, LWM2M_DEVICE_ERROR_CODE,
  LWM2M_DEVICE_BINDINDS,
};

int device_read( struct t_lwm2m_data *p_data )
{
  switch( p_data->id )
  {
    case LWM2M_SERVER_SHORT_SERVER_ID:
      p_data->data_type = LWM2M_ITEM_BINARY;
      p_data->size = sizeof( r_sz_manufacturer ) - 1;
      p_data->data = (void*)r_sz_manufacturer;
      break;
    case LWM2M_DEVICE_MODEL_NUMBER:
      p_data->data_type = LWM2M_ITEM_BINARY;
      p_data->size = sizeof( r_sz_model ) - 1;
      p_data->data = (void*)r_sz_model;
      break;
    case LWM2M_DEVICE_SERIAL_NUMBER:
      p_data->data_type = LWM2M_ITEM_BINARY;
      p_data->size = sizeof( r_sz_serial ) - 1;
      p_data->data = (void*)r_sz_serial;
      break;
    case LWM2M_DEVICE_FIRMWARE_VERSION:
      p_data->data_type = LWM2M_ITEM_BINARY;
      p_data->size = sizeof( r_sz_fw_version ) - 1;
      p_data->data = (void*)r_sz_fw_version;
      break;
    case LWM2M_DEVICE_ERROR_CODE:
      p_data->data_type = LWM2M_ITEM_UINT;
      p_data->size = sizeof( r_err_codes );
      p_data->data = &r_err_codes;
      break;
    case LWM2M_DEVICE_BINDINDS:
      p_data->data_type = LWM2M_ITEM_BINARY;
      p_data->size = sizeof( r_sz_bindings ) - 1;
      p_data->data = (void*)r_sz_bindings;
      break;
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

int device_write( struct t_lwm2m_data *p_data )
{
  (void)p_data;
  return -1;
}

int device_exec( struct t_lwm2m_data *p_data )
{
  switch( p_data->id )
  {
    case LWM2M_DEVICE_REBOOT:
      ;
    break;
    default:
      return -1;
  }
  return 1;
}

int device_create( struct t_lwm2m_data *p_data )
{
  (void)p_data;
  return -1;
}

int device_delete( struct t_lwm2m_data *p_data )
{
  (void)p_data;
  return -1;
}
