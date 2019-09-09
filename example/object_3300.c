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

static const uint16_t id_0_list[] = 
{
  ID_R_SENSOR_VALUE, ID_R_SENSOR_UNITS
};

static const uint16_t id_0_observe_list[] =
{
  ID_R_SENSOR_VALUE
};

static struct t_lwm2m_observe_context observe_state[OBJECT_MAX_INSTANCE] =
{
  [0].timeout = 2000,
  [1].timeout = 2000,
  [2].timeout = 2000,
  [3].timeout = 2000,
  [4].timeout = 2000,
  [5].timeout = 2000,
  [6].timeout = 2000,
};

static float sensor_value[OBJECT_MAX_INSTANCE] = { 0 };
static const char sz_units[] = "Unk";

int object_3300_read(struct t_lwm2m_data *parg )
{
  if( parg->instance >= OBJECT_MAX_INSTANCE )
    return -1;

  switch( parg->id )
  {
    case ID_R_SENSOR_VALUE:
      sensor_value[parg->instance] += 0.1f;
      parg->data_type = LWM2M_ITEM_FLOAT;
      parg->size = sizeof( float );
      parg->data = &sensor_value[parg->instance];
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

int object_3300_write(struct t_lwm2m_data *parg )
{
  switch( parg->id )
  {
    default:
      return -1;
  }
}

int object_3300_exec(struct t_lwm2m_data *parg )
{
  switch( parg->id )
  {
    case ID_E_RESET_MIN_MAX:
      break;
    default:
      return -1;
  }

  return 1;
}

int object_3300_observe(struct t_lwm2m_data *parg, int control, uint32_t timestamp )
{
  if( parg->instance >= OBJECT_MAX_INSTANCE )
    return -1;

  /* call default observe handler */
  return lwm2m_process_observe_control( parg, &observe_state[parg->instance], control, timestamp );
}
