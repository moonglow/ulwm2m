#include <stdlib.h>
#include <string.h>
#include "object_3300.h"

#define OBJECT_MAX_INSTANCE   (7)

/* generic sensor object, example of multi instance device */
#define ID_R_SENSOR_VALUE     (5700)
#define ID_R_SENSOR_UNITS     (5701)
#define ID_R_MIN_MEAS_VALUE   (5601)
#define ID_R_MAX_MEAS_VALUE   (5602)
#define ID_R_MIN_RANGE_VALUE  (5603)
#define ID_R_MAX_RANGE_VALUE  (5604)
#define ID_RW_APP_TYPE        (5750)
#define ID_R_SENSOR_TYPE      (5751)
#define ID_E_RESET_MIN_MAX    (5605)

static const uint16_t id_list[] =
{
  ID_R_SENSOR_VALUE, ID_R_SENSOR_UNITS
};

static const int16_t id_observe_list[] =
{
  LWM2M_GET_ID_LIST/*whole object*/, ID_R_SENSOR_VALUE, ID_R_SENSOR_UNITS
};


/* all readable resource + whole object */
struct t_lwm2m_observe_storage observe_storage[OBJECT_MAX_INSTANCE][ARRAY_NELEMS(id_observe_list)];

static float sensor_value[OBJECT_MAX_INSTANCE] = { 0 };
static const char sz_units[] = "Unk";

int object_3300_read(struct t_lwm2m_data *p_data )
{
  if(p_data->instance >= OBJECT_MAX_INSTANCE )
    return -1;

  switch( p_data->id )
  {
    case ID_R_SENSOR_VALUE:
      sensor_value[p_data->instance] += 0.1f;
      p_data->data_type = LWM2M_ITEM_FLOAT;
      p_data->size = sizeof( float );
      p_data->data = &sensor_value[p_data->instance];
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
      p_data->data = &observe_storage[p_data->instance];
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

int object_3300_write(struct t_lwm2m_data *p_data )
{
  switch( p_data->id )
  {
    default:
      return -1;
  }
}

int object_3300_exec(struct t_lwm2m_data *p_data )
{
  switch( p_data->id )
  {
    case ID_E_RESET_MIN_MAX:
      break;
    default:
      return -1;
  }

  return 1;
}

int object_3300_observe(struct t_lwm2m_data *p_data, int control, uint32_t arg )
{
  if(p_data->instance >= OBJECT_MAX_INSTANCE )
    return -1;

  /* call default observe handler */
  return lwm2m_process_observe_control( p_data, control, arg );
}
