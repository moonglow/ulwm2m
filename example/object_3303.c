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

static const uint16_t id_list[] =
{
  ID_R_SENSOR_VALUE, ID_R_MIN_MEAS_VALUE,
  ID_R_MAX_MEAS_VALUE, ID_R_MIN_RANGE_VALUE,
  ID_R_MAX_RANGE_VALUE, ID_R_SENSOR_UNITS
};

static const uint16_t id_observe_list[] =
{
  ID_R_SENSOR_VALUE
};

static struct t_lwm2m_observe_storage observe_storage[ARRAY_NELEMS(id_observe_list)] =  { [0].time_interval = 2000 };

static float sensor_value, min_value = 100.0f, max_value = 0.0f;
static const float sensor_min_range = 0.0f, sensor_max_range = 100.0f;
static const char sz_units[] = "Cel";

int object_3303_read(struct t_lwm2m_data *p_data )
{
  switch( p_data->id )
  {
    case ID_R_SENSOR_VALUE:
      sensor_value = (rand()%1000)/10.0f;
      /* update min/max */
      if( sensor_value < min_value )
        min_value = sensor_value;
      if( sensor_value > max_value )
        max_value = sensor_value;
      p_data->data_type = LWM2M_ITEM_FLOAT;
      p_data->size = sizeof( sensor_value );
      p_data->data = &sensor_value;
      break;
    case ID_R_MIN_MEAS_VALUE:
      p_data->data_type = LWM2M_ITEM_FLOAT;
      p_data->size = sizeof( min_value );
      p_data->data = &min_value;
      break;
    case ID_R_MAX_MEAS_VALUE:
      p_data->data_type = LWM2M_ITEM_FLOAT;
      p_data->size = sizeof( max_value );
      p_data->data = &max_value;
      break;
    case ID_R_MIN_RANGE_VALUE:
      p_data->data_type = LWM2M_ITEM_FLOAT;
      p_data->size = sizeof( sensor_min_range );
      p_data->data = (void*)&sensor_min_range;
      break;
    case ID_R_MAX_RANGE_VALUE:
      p_data->data_type = LWM2M_ITEM_FLOAT;
      p_data->size = sizeof( sensor_max_range );
      p_data->data = (void*)&sensor_max_range;
      break;
    case ID_R_SENSOR_UNITS:
      p_data->data_type = LWM2M_ITEM_BINARY;
      p_data->size = sizeof( sz_units ) - 1;
      p_data->data = (void*)sz_units;
      break;
    case LWM2M_GET_ID_LIST:
      p_data->data_type = LWM2M_ITEM_ID_LIST;
      p_data->data = (void*)id_list;
      p_data->size = ARRAY_NELEMS(id_list );
      break;
    case LWM2M_GET_OBSERVE_STORAGE:
      p_data->data_type = LWM2M_ITEM_OBSERVE_STORAGE;
      p_data->data = observe_storage;
      p_data->size = ARRAY_NELEMS(id_observe_list);
      break;
    case LWM2M_GET_OBSERVE_LIST:
      p_data->data_type = LWM2M_ITEM_OBSERVE_ID_LIST;
      p_data->data = (void*)id_observe_list;
      p_data->size = ARRAY_NELEMS(id_observe_list);
      break;
    default:
      return -1;
  }
  return 1;
}

int object_3303_write(struct t_lwm2m_data *p_data )
{
  switch( p_data->id )
  {
    default:
      return -1;
  }
}

int object_3303_exec(struct t_lwm2m_data *p_data )
{
  switch( p_data->id )
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

int object_3303_observe(struct t_lwm2m_data *p_data, int control, uint32_t arg )
{
  /* call default observe handler */
  return lwm2m_process_observe_control( p_data, control, arg );
}
