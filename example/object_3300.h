#pragma once
#include <lwm2m.h>

int object_3300_read(struct t_lwm2m_data *p_data );
int object_3300_write(struct t_lwm2m_data *p_data );
int object_3300_exec(struct t_lwm2m_data *p_data );
int object_3300_observe(struct t_lwm2m_data *p_data, int control, uint32_t arg );
