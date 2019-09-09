#pragma once
#include <lwm2m.h>

int object_3300_read(struct t_lwm2m_data *parg );
int object_3300_write(struct t_lwm2m_data *parg );
int object_3300_exec(struct t_lwm2m_data *parg );
int object_3300_observe(struct t_lwm2m_data *parg, int control, uint32_t timestamp );
