#pragma once
#include "lwm2m.h"

int server_read( struct t_lwm2m_item *parg );
int server_write( struct t_lwm2m_item *parg );
int server_exec( struct t_lwm2m_item *parg );
