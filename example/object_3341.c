#include <string.h>
#include "object_3341.h"

#define ID_RW_TEXT            (5527)
#define ID_RW_X_POS           (5528)
#define ID_RW_Y_POS           (5529)
#define ID_R_W_MAX            (5545)
#define ID_R_H_MAX            (5546)
#define ID_E_CLEAR_DISPLAY    (5530)
#define ID_RW_BRIGHT_LEVEL    (5548)
#define ID_RW_CONTRAST_LEVEL  (5531)
#define ID_RW_APP_TYPE        (5750)

static const uint16_t id_list[] =
{
  ID_RW_TEXT,
  ID_RW_X_POS, ID_RW_Y_POS,
  ID_R_W_MAX, ID_R_H_MAX,
  ID_E_CLEAR_DISPLAY,
  ID_RW_BRIGHT_LEVEL, ID_RW_CONTRAST_LEVEL,
  ID_RW_APP_TYPE
};

static char sz_rw_text[64];
static uint32_t x_pos, y_pos;
static const uint8_t screen_w = 80, screen_h = 24;
static float bright_level, contrast_level;
static char sz_app_type[16];

int object_3341_read(struct t_lwm2m_data *p_data )
{
  switch( p_data->id )
  {
    case ID_RW_TEXT:
      p_data->data_type = LWM2M_ITEM_BINARY;
      p_data->size = (int)strlen(sz_rw_text );
      p_data->data = sz_rw_text;
      break;
    case ID_RW_X_POS:
      p_data->data_type = LWM2M_ITEM_UINT;
      p_data->size = sizeof( x_pos );
      p_data->data = &x_pos;
      break;
    case ID_RW_Y_POS:
      p_data->data_type = LWM2M_ITEM_UINT;
      p_data->size = sizeof( y_pos );
      p_data->data = &y_pos;
      break;
    case ID_R_W_MAX:
      p_data->data_type = LWM2M_ITEM_UINT;
      p_data->size = sizeof( screen_w );
      p_data->data = (void*)&screen_w;
      break;
    case ID_R_H_MAX:
      p_data->data_type = LWM2M_ITEM_UINT;
      p_data->size = sizeof( screen_h );
      p_data->data = (void*)&screen_h;
      break;
    case ID_RW_BRIGHT_LEVEL:
      p_data->data_type = LWM2M_ITEM_FLOAT;
      p_data->size = sizeof( bright_level );
      p_data->data = &bright_level;
      break;
    case ID_RW_CONTRAST_LEVEL:
      p_data->data_type = LWM2M_ITEM_FLOAT;
      p_data->size = sizeof( contrast_level );
      p_data->data = &contrast_level;
      break;
    case ID_RW_APP_TYPE:
      p_data->data_type = LWM2M_ITEM_BINARY;
      p_data->size = (int)strlen(sz_app_type );
      p_data->data = sz_app_type;
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

int object_3341_write(struct t_lwm2m_data *p_data )
{
  switch( p_data->id )
  {
    case ID_RW_TEXT:
      if( !lwm2m_read_item_string(p_data, sz_rw_text, sizeof( sz_rw_text ) ) )
        return -1;
      break;
    case ID_RW_X_POS:
      x_pos = lwm2m_read_item_int(p_data );
      break;
    case ID_RW_Y_POS:
      y_pos = lwm2m_read_item_int(p_data );
      break;
    case ID_RW_BRIGHT_LEVEL:
      bright_level = lwm2m_read_item_float(p_data );
      break;
    case ID_RW_CONTRAST_LEVEL:
      contrast_level = lwm2m_read_item_float(p_data );
      break;
    case ID_RW_APP_TYPE:
      if( !lwm2m_read_item_string(p_data, sz_app_type, sizeof( sz_app_type ) ) )
        return -1;
      break;
    default:
      return -1;
  }
  return 1;
}


int object_3341_exec(struct t_lwm2m_data *p_data )
{
  switch( p_data->id )
  {
    case ID_E_CLEAR_DISPLAY:
      memset( sz_rw_text, 0x00, sizeof( sz_rw_text ) );
      break;
    default:
      return -1;
  }

  return 1;
}
