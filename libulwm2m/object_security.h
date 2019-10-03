#pragma once
#include "lwm2m.h"

int security_read( struct t_lwm2m_data *p_data );
int security_write( struct t_lwm2m_data *p_data );
int security_exec( struct t_lwm2m_data *p_data );
int security_create( struct t_lwm2m_data *p_data );
int security_delete( struct t_lwm2m_data *p_data );

