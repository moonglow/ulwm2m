#pragma once
#include "lwm2m.h"

int security_read( struct t_lwm2m_item *parg );
int security_write( struct t_lwm2m_item *parg );
int security_exec( struct t_lwm2m_item *parg );
int security_create( struct t_lwm2m_item *parg );
int security_delete( struct t_lwm2m_item *parg );

