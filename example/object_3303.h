#pragma once
#include <lwm2m.h>

int object_3303_read(struct t_lwm2m_data *p_data );
int object_3303_write(struct t_lwm2m_data *p_data );
int object_3303_exec(struct t_lwm2m_data *p_data );
int object_3303_observe(struct t_lwm2m_data *p_data, int control, uint32_t arg );
