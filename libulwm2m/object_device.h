#pragma once
#include "lwm2m.h"

int device_read( struct t_lwm2m_data *parg );
int device_write( struct t_lwm2m_data *parg );
int device_exec( struct t_lwm2m_data *parg );
int device_create( struct t_lwm2m_data *parg );
int device_delete( struct t_lwm2m_data *parg );
