#include <stdlib.h>
#include <string.h>
#include "object_3303.h"

/* based on leshan server predefined object 3303  */
#define ID_R_SENSOR_VALUE     (5700)
#define ID_R_MIN_MEAS_VALUE   (5601)
#define ID_R_MAX_MEAS_VALUE   (5602)
#define ID_R_MIN_RANGE_VALUE  (5603)
#define ID_R_MAX_RANGE_VALUE  (5604)
#define ID_R_SENSOR_UNITS     (5701)
#define ID_E_RESET_MIN_MAX    (5605)

static const uint16_t id_0_list[] = 
{
  ID_R_SENSOR_VALUE, ID_R_MIN_MEAS_VALUE,
  ID_R_MAX_MEAS_VALUE, ID_R_MIN_RANGE_VALUE,
  ID_R_MAX_RANGE_VALUE, ID_R_SENSOR_UNITS
};

static const uint16_t id_0_observe_list[] =
{
  ID_R_SENSOR_VALUE
};

static struct t_lwm2m_observe_context observe_state =  { .timeout = 2000 };

static float sensor_value, min_value = 100.0f, max_value = 0.0f;
static const float sensor_min_range = 0.0f, sensor_max_range = 100.0f;
static const char sz_units[] = "Cel";

int object_3303_read(struct t_lwm2m_data *parg )
{
  switch( parg->id )
  {
    case ID_R_SENSOR_VALUE:
      sensor_value = (rand()%1000)/10.0f;
      /* update min/max */
      if( sensor_value < min_value )
        min_value = sensor_value;
      if( sensor_value > max_value )
        max_value = sensor_value;
      parg->data_type = LWM2M_ITEM_FLOAT;
      parg->size = sizeof( sensor_value );
      parg->data = &sensor_value;
      break;
    case ID_R_MIN_MEAS_VALUE:
      parg->data_type = LWM2M_ITEM_FLOAT;
      parg->size = sizeof( min_value );
      parg->data = &min_value;
      break;
    case ID_R_MAX_MEAS_VALUE:
      parg->data_type = LWM2M_ITEM_FLOAT;
      parg->size = sizeof( max_value );
      parg->data = &max_value;
      break;
    case ID_R_MIN_RANGE_VALUE:
      parg->data_type = LWM2M_ITEM_FLOAT;
      parg->size = sizeof( sensor_min_range );
      parg->data = (void*)&sensor_min_range;
      break;
    case ID_R_MAX_RANGE_VALUE:
      parg->data_type = LWM2M_ITEM_FLOAT;
      parg->size = sizeof( sensor_max_range );
      parg->data = (void*)&sensor_max_range;
      break;
    case ID_R_SENSOR_UNITS:
      parg->data_type = LWM2M_ITEM_BINARY;
      parg->size = sizeof( sz_units ) - 1;
      parg->data = (void*)sz_units;
      break;
    case LWM2M_GET_ID_LIST:
      parg->data_type = LWM2M_ITEM_ID_LIST;
      parg->data = (void*)id_0_list;
      parg->size = sizeof( id_0_list );
      break;
    case LWM2M_GET_OBSERVE_LIST:
      parg->data_type = LWM2M_ITEM_OBSERVE_ID_LIST;
      parg->data = (void*)id_0_observe_list;
      parg->size = sizeof( id_0_observe_list );
      break;
    default:
      return -1;
  }
  return 1;
}

int object_3303_write(struct t_lwm2m_data *parg )
{
  switch( parg->id )
  {
    default:
      return -1;
  }
}

int object_3303_exec(struct t_lwm2m_data *parg )
{
  switch( parg->id )
  {
    case ID_E_RESET_MIN_MAX:
      min_value = sensor_value;
      max_value = sensor_value;
      break;
    default:
      return -1;
  }

  return 1;
}

int object_3303_observe(struct t_lwm2m_data *parg, int control, uint32_t timestamp )
{
  /* call default observe handler */
  return lwm2m_process_observe_control( parg, &observe_state, control, timestamp );
}
