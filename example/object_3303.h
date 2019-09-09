#pragma once
#include <lwm2m.h>

int object_3303_read(struct t_lwm2m_data *parg );
int object_3303_write(struct t_lwm2m_data *parg );
int object_3303_exec(struct t_lwm2m_data *parg );
int object_3303_observe(struct t_lwm2m_data *parg, int control, uint32_t timestamp );
