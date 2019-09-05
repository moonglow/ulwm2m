#pragma once
#include "coap.h"

#define LWM2M_VERSION_1_0                0
#define LWM2M_VERSION_1_1                1

#ifndef LWM2M_VERSION
#define LWM2M_VERSION LWM2M_VERSION_1_0
#endif

#define LWM2M_SECURITY_OBJECT            0
#define LWM2M_SERVER_OBJECT              1
#define LWM2M_ACL_OBJECT                 2
#define LWM2M_DEVICE_OBJECT              3
#define LWM2M_CONN_MONITOR_OBJECT        4
#define LWM2M_FIRMWARE_UPDATE_OBJECT     5
#define LWM2M_LOCATION_OBJECT            6
#define LWM2M_CONN_STATS_OBJECT          7
#define LWM2M_OSCORE_OBJECT              21

/* SECURITY OBJECT */
#define LWM2M_SECURITY_SERVER_URI             0
#define LWM2M_SECURITY_BOOTSTRAP_SERVER       1
#define LWM2M_SECURITY_SECURITY_MODE          2
#define LWM2M_SECURITY_PUBLIC_KEY             3
#define LWM2M_SECURITY_SERVER_PUBLIC_KEY      4
#define LWM2M_SECURITY_SECRET_KEY             5

/* SERVER OBJECT */
#define LWM2M_SERVER_SHORT_SERVER_ID          0
#define LWM2M_SERVER_LIFETIME                 1
#define LWM2M_SERVER_STORING                  6
#define LWM2M_SERVER_BINDING                  7
#define LWM2M_SERVER_UPDATE_TRIGGER           8

/* DEVICE OBJECT */
#define LWM2M_DEVICE_MANUFACTURER             0
#define LWM2M_DEVICE_MODEL_NUMBER             1
#define LWM2M_DEVICE_SERIAL_NUMBER            2
#define LWM2M_DEVICE_FIRMWARE_VERSION         3
#define LWM2M_DEVICE_REBOOT                   4
#define LWM2M_DEVICE_ERROR_CODE               11
#define LWM2M_DEVICE_BINDINDS                 16


#define LWM2M_GET_ID_LIST                         -1
#define LWM2M_GET_INST_LIST                       -2
#define LWM2M_ITEM_INT                       (0xC0DE0)
#define LWM2M_ITEM_UINT                      (0xC0DE1)
#define LWM2M_ITEM_FLOAT                     (0xC0DE2)
#define LWM2M_ITEM_DOUBLE                    (0xC0DE3)
#define LWM2M_ITEM_BINARY                    (0xC0DE4)
#define LWM2M_ITEM_ID_LIST                   (0xC0DE5)
#define LWM2M_ITEM_INST_LIST                 (0xC0DE6)
#define LWM2M_ITEM_FROM_NETWORK              (0xC0DE7)

#define LWM2M_EVENT_RX                        1
#define LWM2M_EVENT_TX                        2
#define LWM2M_EVENT_RESET                     3

struct t_lwm2m_item
{
  struct t_lwm2m_obj *p_obj;
  int     instance;
  int     id;
  int     data_type;
  void    *data;
  int     size;
};

struct t_lwm2m
{
  char *sz_endpoint_name;
  char sz_reg_path[16];
  uint32_t reg_timestamp;
  struct t_lwm2m_obj *root;
  int state;
  
  /* shared mem */
  uint8_t *mem;
  uint16_t mem_size;
  /* transport */
  struct t_coap_packet coap;
  int (*init)( char *psz_host, int port );
  int (*recv)( uint8_t *data, int size, int timeout );
  int (*send)( uint8_t *data, int size );
};

struct t_lwm2m_obj
{
  struct t_lwm2m_obj *next;
  uint16_t instances; /* bitmap */
  uint16_t id;
  /* read/discover */
  int (*read)( struct t_lwm2m_item *p );
  int (*write)( struct t_lwm2m_item *p );
  int (*exec)( struct t_lwm2m_item *p );
  int (*create)( struct t_lwm2m_item *p );
  int (*delete)( struct t_lwm2m_item *p );
};

int lwm2m_init( struct t_lwm2m *p, struct t_lwm2m_obj *obj_llist, uint8_t *mem, uint16_t size );
int lwm2m_process( struct t_lwm2m *p, int event, uint32_t timestamp );
char *lwm2m_read_item_string( struct t_lwm2m_item *p, char *p_sz, int max_size );
uint32_t lwm2m_read_item_int( struct t_lwm2m_item *p );
float lwm2m_read_item_float( struct t_lwm2m_item *p );

