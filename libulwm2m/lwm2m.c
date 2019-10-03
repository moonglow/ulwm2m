/*********************************************************************\
* Copyleft (>) 2019 Roman Ilichev <fxdteam@gmail.com>                 *
*                                                                     *
* This file is part of uLWM2M project                                 *
*                             WTFPL LICENSE v2                        *
\*********************************************************************/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "lwm2m.h"
#include "utils.h"
#include "tlv.h"

#define LWM2M_OBSERVE             (100)
#define LWM2M_READ                (101)
/* DISCOVER only while bootstrap process */
#define LWM2M_DISCOVER            (1010)
#define LWM2M_CREATE              (102)
#define LWM2M_WRITE               (103)
#define LWM2M_WRITE_ATTRIBUTES    (1030)
#define LWM2M_EXEC                (104)
#define LWM2M_DELETE              (105)

enum
{
  INIT_CONN,
  POST_RD,
  POST_UPDATE_RD,
  WAIT_RD_ACK,
  WAIT_NOTHING,
};

int lwm2m_init( struct t_lwm2m *p, struct t_lwm2m_obj *obj_llist, uint8_t *mem, uint16_t size )
{
  memset( p, 0x00, sizeof( struct t_lwm2m ) );

  p->root = obj_llist;
  p->mem = mem;
  p->mem_size = size;
  p->state = INIT_CONN;
  return 0;
}

static struct t_lwm2m_obj *lwm2m_find_object( struct t_lwm2m *p, int object_id )
{
  struct t_lwm2m_obj *obj;

  for( obj = p->root; obj; obj = obj->next )
  {
    if( obj->id == object_id )
      return obj;
  }

  return (struct t_lwm2m_obj *)0;
}

static struct t_lwm2m_obj *lwm2m_find_object_by_index( struct t_lwm2m *p, uint8_t ifind )
{
  struct t_lwm2m_obj *obj;
  uint8_t object_index = 0;

  for( obj = p->root; obj; obj = obj->next, ++object_index )
  {
    if( ifind == object_index )
      return obj;
  }

  return (struct t_lwm2m_obj *)0;
}

static uint8_t lwm2m_find_object_index( struct t_lwm2m *p, struct t_lwm2m_obj *p_obj_src )
{
  struct t_lwm2m_obj *obj;
  uint8_t object_index = 0;

  for( obj = p->root; obj; obj = obj->next, ++object_index )
  {
    if( obj == p_obj_src )
      return object_index;
  }

  return 0;
}

char *lwm2m_read_item_string(struct t_lwm2m_data *p_item, char *p_sz, int max_size )
{
  if(p_item->size >= max_size )
    return (char*)0;

  memcpy(p_sz, p_item->data, p_item->size );
  p_sz[p_item->size] = '\0';

  return p_sz;
}

uint32_t lwm2m_read_item_int( struct t_lwm2m_data *p_item )
{
  uint8_t *data = p_item->data;

  if(p_item->data_type == LWM2M_ITEM_FROM_NETWORK )
  {
#ifndef HOST_IS_BIG_ENDIAN
    switch( p_item->size )
    {
      case 4:
        return data[0]<<24u|data[1]<<16|data[2]<<8|data[3];
      case 3:
        return data[0]<<16|data[1]<<8|data[2];
      case 2:
        return data[0]<<8|data[1];
      case 1:
        return data[0];
      default:
        return 0;
    }
#endif
  }
  else
  {
    switch(p_item->size)
    {
#ifndef HOST_IS_BIG_ENDIAN
      case 4:
        return data[3] << 24u | data[2] << 16 | data[1] << 8 | data[0];
      case 3:
        return data[2] << 16 | data[1] << 8 | data[0];
      case 2:
        return data[1] << 8 | data[0];
#else
      case 4:
        return data[0]<<24u|data[1]<<16|data[2]<<8|data[3];
      case 3:
        return data[0]<<16|data[1]<<8|data[2];
      case 2:
        return data[0]<<8|data[1];
#endif
      case 1:
        return data[0];
      default:
        return 0;
    }
  }
}

float lwm2m_read_item_float( struct t_lwm2m_data *p_item )
{
  union
  {
    float    fl;
    double   db;
    uint8_t  raw[sizeof(double)];
  }u;

  if(p_item->size != sizeof(float) && p_item->size != sizeof(double) )
    return 0.0f;

  for(int i = 0; i < p_item->size; i++ )
  {
#ifndef HOST_IS_BIG_ENDIAN
    if(p_item->data_type == LWM2M_ITEM_FROM_NETWORK )
      u.raw[i] = ((uint8_t*)p_item->data)[p_item->size - 1 - i];
    else
#endif
      u.raw[i] = ((uint8_t*)p_item->data)[i];
  }

  if(p_item->size == sizeof(double) )
    return (float)u.db;

  return u.fl;
}

int lwm2m_process_observe_control(struct t_lwm2m_data *p_data, int control, uint32_t arg )
{
  struct t_lwm2m_observe_storage *p_storage;
  struct t_lwm2m_data item = { 0 };
  int16_t *p_id_list;

  /* set active instance */
  item.instance = p_data->instance;

  /* request observe storage */
  item.id = LWM2M_GET_OBSERVE_STORAGE;
  if( p_data->p_obj->read( &item ) < 0 )
    return -1;

  p_storage = item.data;

  /* return observe resource list*/
  item.id = LWM2M_GET_OBSERVE_LIST;
  if( p_data->p_obj->read( &item ) < 0 )
    return -1;

  p_id_list = item.data;

  switch( control )
  {
    case LWM2M_OBSERVE_CHECK:
      for(int i = 0; i < item.size; i++ )
      {
        if(!p_storage[i].token_len)
          continue;

        if((uint32_t)(arg - p_storage[i].time_start) < p_storage[i].time_interval)
          continue;

        p_storage[i].time_start = arg;

        p_data->data_type = LWM2M_ITEM_OBSERVE_TOKEN;
        p_data->id = p_id_list[i];
        p_data->data = p_storage[i].token;
        p_data->size = p_storage[i].token_len;
        return 1;
      }
      break;
    case LWM2M_OBSERVE_RESET:
      for( int i = 0; i < item.size; i++ )
      {
        if( p_storage[i].message_id == arg )
        {
          p_storage[i].message_id = 0;
          p_storage[i].token_len = 0;
          return 1;
        }
      }
      break;
    case LWM2M_OBSERVE_SET_TOKEN:
      for( int i = 0; i < item.size; i++ )
      {
        if( p_id_list[i] == p_data->id )
        {
          memcpy( p_storage[i].token, p_data->data, p_data->size );
          p_storage[i].token_len = p_data->size;
          p_storage[i].time_start = arg;
          /* set default interval ? */
          if( !p_storage[i].time_interval )
            p_storage[i].time_interval = 2000;
          return 1;
        }
      }
      break;
    case LWM2M_OBSERVE_SET_MID:
      for(int i = 0; i < item.size; i++ )
      {
        if( p_id_list[i] == p_data->id )
        {
          p_storage[i].message_id = arg;
          return 1;
        }
      }
      break;
    default:
      return -1;
  }

  return -1;
}

static int lwm2m_read_object_int( struct t_lwm2m *p, int obj_id, int inst_is, int id, uint32_t *val )
{
  struct t_lwm2m_data item = { .instance = inst_is, .id = id };

  item.p_obj = lwm2m_find_object(p, obj_id );
  if(!item.p_obj || !item.p_obj->read )
    return -1;

  if(item.p_obj->read(&item ) < 0 )
    return -1;

  if( val )
    *val = lwm2m_read_item_int( &item );

  return 0;
}

static int lwm2m_init_server_connection( struct t_lwm2m *p )
{
  char *s, *e;
  struct t_lwm2m_data item = { .instance = 0, .id = LWM2M_SECURITY_SERVER_URI };

  if( !p->init )
    return 0;

  item.p_obj = lwm2m_find_object(p, LWM2M_SECURITY_OBJECT );
  if(!item.p_obj || !item.p_obj->read )
    return -1;
  
  if(item.p_obj->read(&item ) < 0 )
    return -1;
  
  if( p->mem_size < (item.size + 1) )
    return -1;

  s = strncpy((char*)p->mem, item.data, item.size );
  s[item.size] = '\0';
  s = strchr( s, '/' );
  if( !s || s[1] != '/' )
    return -1;
  s += 2;

  e = strchr( s, ':' );
  if( !e )
    return -1;

  *e++ = '\0';

  if( p->init( s,  _strtoi( e ) ) < 0 )
    return -1;

  return 0;
}

static int lwm2m_coap_init_options( struct t_lwm2m *p, int gap )
{
  int size = COAP_HEADER_SIZE + p->coap.tkl;
  return coap_set_option_buffer( &p->coap, p->mem + size, p->mem_size - size - gap );
}

static int lwm2m_send_coap_msg( struct t_lwm2m *p )
{
  int size;

#ifdef COAP_DBG_PRINT_PACKET
  (void)coap_print_packet( &p->coap );
#endif

  if( !p->send )
    return -1;

  size = coap_write_packet( &p->coap, p->mem, p->mem_size );
  if( size < 0 )
    return -1;

  return p->send( p->mem, size );
}

static void lwm2m_create_transaction_id( struct t_lwm2m *p, uint8_t obj_index, uint8_t obj_instance  )
{
  ++p->trans_id;
  p->trans_id = ((obj_index & 0x0Fu)<<12u) | ((obj_instance & 0x0Fu)<<8u) | (p->trans_id & 0xFFu);
}

static int lwm2m_create_rd_update( struct t_lwm2m *p )
{
  /* create coap link request update */
  lwm2m_create_transaction_id( p, 0, 0 );
  coap_init_message( &p->coap, COAP_TYPE_CON, COAP_POST, p->trans_id );
  coap_set_header_token( &p->coap, (void*)(int32_t[]){ rand() }, 4 );

  if( lwm2m_coap_init_options( p, 0 ) < 0 )
    return -1;

  /* write request option */
  coap_option_add_string( &p->coap, COAP_OPTION_URI_PATH, "rd" );
  coap_option_add_string( &p->coap, COAP_OPTION_URI_PATH, p->sz_reg_path );

  /* commit options */
  if( coap_get_option_block_size( &p->coap, 1 ) < 0 )
    return -1;
 
  return lwm2m_send_coap_msg( p );    
}

static int lwm2m_create_rd_request( struct t_lwm2m *p )
{
  const int temp_buffer_size = (32);
  struct t_lwm2m_data item = { .instance = 0 };
  char *s;
  int res, size, offset;

  item.p_obj = lwm2m_find_object(p, LWM2M_SERVER_OBJECT );
  if(!item.p_obj || !item.p_obj->read )
    return -1;

  /* create coap link request to resource directory */
  lwm2m_create_transaction_id( p, 0, 0 );
  coap_init_message( &p->coap, COAP_TYPE_CON, COAP_POST, p->trans_id );
  coap_set_header_token( &p->coap, (void*)(int32_t[]){ rand() }, 4 );

  /* allocate temporary buffer */
  s = (char*)(p->mem + (p->mem_size - temp_buffer_size) );
  
  size = COAP_HEADER_SIZE + p->coap.tkl;
  coap_set_option_buffer( &p->coap, p->mem + size, (p->mem_size - temp_buffer_size) - size );

  /* write request option */
  coap_option_add_string( &p->coap, COAP_OPTION_URI_PATH, "/rd" );
  coap_option_add_int( &p->coap, COAP_OPTION_CONTENT_FORMAT, COAP_APP_LINK_FORMAT );

  snprintf( s, temp_buffer_size, "lwm2m=1.%d", LWM2M_VERSION );
  coap_option_add_string( &p->coap, COAP_OPTION_URI_QUERY, s );

  /* device endpoint */
  strcpy( s, "ep=" );
  strcat( s,  p->sz_endpoint_name );
  coap_option_add_string( &p->coap, COAP_OPTION_URI_QUERY, s );

  /* transport binding */
  item.id = LWM2M_SERVER_BINDING;
  res = item.p_obj->read(&item );
  if( res < 0 )
    return -1;
  
  strcpy( s, "b=" );
  strncat(s, item.data, item.size );
  coap_option_add_string( &p->coap, COAP_OPTION_URI_QUERY, s );

  /* lifetime in seconds */
  item.id = LWM2M_SERVER_LIFETIME;
  res = item.p_obj->read(&item );
  if( res < 0 )
    return -1;

  snprintf( s, temp_buffer_size, "lt=%d", lwm2m_read_item_int( &item ) );
  coap_option_add_string( &p->coap, COAP_OPTION_URI_QUERY, s );

  /* commit options */
  if( coap_get_option_block_size( &p->coap, 1 ) < 0 )
    return -1;
  
  /* write payload */
  size = COAP_HEADER_SIZE + p->coap.tkl + p->coap.size_options;
  s = (char*)(p->mem + size + 1);
  size = p->mem_size - size - 1;

  /* resource type(lwm2m), content type(tlv) and objects list */
  res = snprintf( s, size, "</>;rt=\"oma.lwm2m\";ct=%d", COAP_TLV_FORMAT );
  if( res < 0 )
    return -1;
  
  offset = res;
  for( struct t_lwm2m_obj *p_obj = p->root; p_obj; p_obj = p_obj->next )
  {
    /* you should skip some security related objects */
    if( p_obj->id == LWM2M_SECURITY_OBJECT )
      continue;
    if( p_obj->id == LWM2M_OSCORE_OBJECT )
      continue;
    
    /* instance 0 is always present */
    for( uint8_t i = 0; i < (sizeof( p_obj->instances )<<3u); i++ )
    {
      if( (p_obj->instances&(1<<i)) == 0 )
        continue;
      res = snprintf( s + offset, size - offset, ",</%u/%u>", p_obj->id, i );
      if( res < 0 )
        return -1;
      offset += res;
    }
  }
  coap_set_payload_buffer( &p->coap, (void*)(s - 1), offset + 1 );

  return lwm2m_send_coap_msg( p );    
}

static int lwm2m_process_reg_ack( struct t_lwm2m *p )
{
  int res, depth;
  struct t_coap_option opt = { 0 };

  if( p->coap.type != COAP_TYPE_ACK )
    return -1;

  /* COAP_CREATED  or COAP_CHANGED */
  if( p->coap.rr_code != COAP_SET_CODE( COAP_201_CREATED ) )
  {
    if( p->coap.rr_code != COAP_SET_CODE( COAP_204_CHANGED ) )
      return -1;
  }

  depth = 0;
  for( res = -1; ; )
  {
    res = coap_read_option( &p->coap, &opt );
    if( res < 0 || !res )
      break;
  
    if( opt.number == COAP_OPTION_LOCATION_PATH )
    {
      switch( depth )
      {
        case 0:
          if( opt.size != 2 )
            return -1;
          if( strncmp( (char*)opt.p_data, "rd", 2 ) != 0 )
            return -1;
          depth = 1;
        break;
        case 1:
          /* reg path too long ? */
          if( sizeof( p->sz_reg_path ) <= opt.size )
            return -1;
          strncpy( p->sz_reg_path, (char*)opt.p_data, opt.size );
          p->sz_reg_path[opt.size] = '\0';
          /* check for abnormal path */
          depth = 2;
        break;
        default:
          return -1;
      }
    }
  }

  return res;
}

static int lwm2m_process_object_command( struct t_lwm2m *p, struct t_lwm2m_data *p_item )
{
  struct t_coap_option opt = { 0 };
  char is_observe = 0, depth = 0, param_count = 0;
  int res, format = COAP_TLV_FORMAT;

  p_item->p_obj = 0;
  p_item->instance = -1;
  p_item->id = -1;

  for( res = -1; ; )
  {
    res = coap_read_option( &p->coap, &opt );
    if( res < 0 || !res )
      break;
  
    switch( opt.number )
    {
      case COAP_OPTION_URI_PATH:
        if( !opt.size )
          break;

        if( opt.p_data[0] < '0' || opt.p_data[0] > '9' )
          break;

        switch( depth )
        {
          case 0:
            p_item->p_obj = lwm2m_find_object( p, _strntoi( (char*)opt.p_data, opt.size ) );
            ++depth;
          break;
          case 1:
            p_item->instance = _strntoi( (char*)opt.p_data, opt.size );
            ++depth;
          break;
          case 2:
            p_item->id = _strntoi( (char*)opt.p_data, opt.size );
            ++depth;
          break;
          default:
            return -1;
        }
      break;
      case COAP_OPTION_URI_QUERY:
        ++param_count;
        break;
      case COAP_OPTION_CONTENT_FORMAT:
      case COAP_OPTION_ACCEPT:
        format = coap_option_read_int( &opt );
      break;
      case COAP_OPTION_OBSERVE:
        is_observe = 1;
      break;
    }
  }

  /* valid object in all cases */
  if( !p_item->p_obj )
    return -1;

  switch( p->coap.rr_code )
  {
    case COAP_SET_CODE( COAP_GET ):
      if( is_observe )
        return p_item->p_obj->read ? LWM2M_OBSERVE: -1;
      if( p->coap.size_payload != 0 )
        return -1;
      if( format == COAP_APP_LINK_FORMAT )
          return LWM2M_DISCOVER;
      /* must accept TLV */
      return p_item->p_obj->read && (format == COAP_TLV_FORMAT) ? LWM2M_READ : -1;
    case COAP_SET_CODE( COAP_PUT ):
      if( depth < 2 )
        return -1;
      if( p->coap.size_payload == 0 )
        return p_item->p_obj->observe && param_count ? LWM2M_WRITE_ATTRIBUTES: -1;
      /* must provide TLV */
      return p_item->p_obj->write && ( format == COAP_TLV_FORMAT ) ? LWM2M_WRITE: -1;
    case COAP_SET_CODE( COAP_POST ):
      if( depth == 1 )
        return p_item->p_obj->create ? LWM2M_CREATE : -1;
      if( depth != 3 )
        return -1;
      /* call args */
      p_item->data = (void*)p->coap.p_payload;
      p_item->size = p->coap.size_payload;
      return p_item->p_obj->exec ? LWM2M_EXEC: -1;
    case COAP_SET_CODE( COAP_DELETE ):
      return p_item->p_obj->delete ? LWM2M_DELETE: -1;
  }

  return -1;
}

int lwm2m_init_content_response( struct t_lwm2m *p, int type, int is_observe )
{
  int res, size;

  /* old token size */
  size = p->coap.tkl;

  coap_init_message( &p->coap, type, COAP_205_CONTENT, ( type == COAP_TYPE_ACK ) ? p->coap.mid: p->trans_id );
  coap_set_header_token( &p->coap, p->coap.token, size );

  size = p->mem_size - COAP_HEADER_SIZE - p->coap.tkl;

  coap_set_option_buffer( &p->coap, p->mem + size, p->mem_size - size );
  if( is_observe )
  {
    coap_option_add_int(&p->coap, COAP_OPTION_OBSERVE, p->trans_id&0xFFFFFFu );
  }
  coap_option_add_int( &p->coap, COAP_OPTION_CONTENT_FORMAT, COAP_TLV_FORMAT );
    
  res = coap_get_option_block_size( &p->coap, 1 );
  if( res < 0 )
    return -1;

  size = COAP_HEADER_SIZE + p->coap.tkl + p->coap.size_options;

  /* skip payload start marker */
  coap_set_payload_buffer( &p->coap, (p->mem + size + 1), p->mem_size - size - 1 );

  /* max possible content size */
  return p->mem_size - size - 1; 
}

static int lwm2m_server_simple_response( struct t_lwm2m *p, int code )
{
  coap_init_message( &p->coap, COAP_TYPE_ACK, code, p->coap.mid );

  return lwm2m_send_coap_msg( p );
}

static int lwm2m_recv_packet( struct t_lwm2m *p, int timeout )
{
  int res;
  res = p->recv( p->mem, p->mem_size, timeout );
  if( res < 0 )
    return -1;
  if( res == 0 )
    return 0;

  res = coap_read_packet( &p->coap, p->mem, res );
  if( res < 0 )
    return -1;

#ifdef COAP_DBG_PRINT_PACKET
  (void)coap_print_packet( &p->coap );
#endif
  return res;
}

static int lwm2m_tlv_walker(struct t_lwm2m_data *p_item, uint8_t *data, int size )
{
  struct t_tlv_item item;

  p_item->data_type = LWM2M_ITEM_FROM_NETWORK;

  for (int offset = 0; offset < size;)
  {
    int res = _tlv_decode_item( data + offset, size - offset, &item);
    if (res < 0)
      return -1;

    offset += res;

    switch (item.id_type)
    {
      case TLV_OBJ_INSTANCE:
        /* TODO: */
        return -1;
      case TLV_RES_INSTANCE:
        p_item->instance = item.id;
        p_item->data = item.p;
        p_item->size = item.size;
        (void)p_item->p_obj->write(p_item );
        break;
      case TLV_RES_MULTI:
        p_item->id = item.id;
        if(lwm2m_tlv_walker(p_item, item.p, item.size) < 0 )
          return 0;
        break;
      case TLV_RES_VALUE:
        p_item->id = item.id;
        p_item->data = item.p;
        p_item->size = item.size;
        (void)p_item->p_obj->write(p_item );
        break;
    }
  }

  return 0;
}

static int lwm2m_tlv_encode_item(struct t_lwm2m_data *p_item, uint8_t *data, int size )
{
  struct t_tlv_item tlv = {
          .id = p_item->id,
          .id_type = TLV_RES_VALUE,
          .size = p_item->size,
          .p = p_item->data,
  };

  tlv.id_type |= (p_item->data_type == LWM2M_ITEM_BINARY) ? 0 : TLV_H2N;

  return _tlv_encode_item( data, size, &tlv );
}

static int lwm2m_object_write_attributes( struct t_lwm2m *p, struct t_lwm2m_data *p_item )
{
  struct t_coap_option opt = { 0 };
  int res;

  for( res = -1; ; )
  {
    res = coap_read_option( &p->coap, &opt );
    if(res < 0 || !res)
      break;

    if( opt.number != COAP_OPTION_URI_QUERY )
      continue;

    if( opt.size < 2 || !p_item->p_obj->observe )
      break;

    p_item->data = 0;
    p_item->size = 0;

    switch( opt.p_data[0] )
    {
      /* pmin, pmax */
      case 'p':
        if( opt.size < 4 || opt.p_data[1] != 'm' )
          return -1;
        if( opt.size > 4 && opt.p_data[4] == '=' )
        {
          p_item->data = opt.p_data + 5;
          p_item->size = opt.size - 5;
        }
        (void)p_item->p_obj->observe( p_item, opt.p_data[2] == 'i' ? LWM2M_OBSERVE_ATTR_PMIN: LWM2M_OBSERVE_ATTR_PMAX, 0 );
      break;
      /* lt, gt, st */
      case 'l':case 'g':case 's':
        if( opt.p_data[1] != 't' )
          return -1;
        if( opt.size > 2 && opt.p_data[2] == '=' )
        {
          p_item->data = opt.p_data + 3;
          p_item->size = opt.size - 3;
        }
        (void)p_item->p_obj->observe( p_item, opt.p_data[0] == 'l' ? LWM2M_OBSERVE_ATTR_LT: opt.p_data[0] == 'g' ? LWM2M_OBSERVE_ATTR_GT: LWM2M_OBSERVE_ATTR_STEP, 0 );
        break;
      default:
        return -1;
    }
  }

  return 0;
}

static int lwm2m_object_provide_content( struct t_lwm2m *p, struct t_lwm2m_data *p_item, int is_response, int is_observe )
{
  int res, offset;

  offset = 0;
  res = p_item->p_obj->read(p_item);
  if(res < 0)
    return lwm2m_server_simple_response(p, COAP_404_NOT_FOUND);

  if( !is_response )
  {
    lwm2m_create_transaction_id( p, lwm2m_find_object_index(p, p_item->p_obj), (uint8_t)p_item->instance );
  }

  if( is_observe && p_item->p_obj->observe )
  {
    (void)p_item->p_obj->observe( p_item, LWM2M_OBSERVE_SET_MID, p->trans_id );
  }

  res = lwm2m_init_content_response( p, is_response ? COAP_TYPE_ACK: COAP_TYPE_NON, is_observe );
  if( res < 0 )
    return lwm2m_server_simple_response( p, COAP_413_REQUEST_ENTITY_TOO_LARGE );

  /* read whole object ? */
  if( p_item->id < 0 )
  {
    for( uint16_t *list = (void*)p_item->data, n = p_item->size; n; )
    {
      p_item->id = list[--n];
      res = p_item->p_obj->read( p_item );
      if( res < 0 )
        continue;

      res = lwm2m_tlv_encode_item( p_item, p->coap.p_payload + offset, p->coap.size_payload - offset );
      if( res < 0 )
        break;

      offset += res;
    }
  }
  else
  {
    res = lwm2m_tlv_encode_item( p_item, p->coap.p_payload, p->coap.size_payload );
    offset += res;
  }

  if( res < 0 )
    return lwm2m_server_simple_response( p, COAP_413_REQUEST_ENTITY_TOO_LARGE );

  /* update payload size and send */
  p->coap.size_payload = offset;
  return lwm2m_send_coap_msg( p );
}

static int lwm2m_object_observe_content( struct t_lwm2m *p, struct t_lwm2m_data *p_item, uint32_t timestamp )
{
  struct t_lwm2m_obj *p_obj;

  for( p_obj = p->root; p_obj; p_obj = p_obj->next )
  {
    if( !p_obj->observe )
      continue;
    p_item->p_obj = p_obj;

    /* instance 0 is always present */
    for( uint8_t i = 0; i < (sizeof( p_obj->instances )<<3u); i++ )
    {
      if( (p_obj->instances&(1<<i)) == 0 )
        continue;
      p_item->instance = i;

      /* check observe and return context token */
      if( p_obj->observe( p_item, LWM2M_OBSERVE_CHECK, timestamp ) <= 0 )
        continue;

      if( p_item->data_type == LWM2M_ITEM_OBSERVE_TOKEN )
      {
        (void)coap_set_header_token(&p->coap, p_item->data, p_item->size);
      }

      return lwm2m_object_provide_content( p, p_item, 0, 1 );
    }
  }

  return 0;
}

int lwm2m_process( struct t_lwm2m *p, int event, uint32_t timestamp )
{
  int res;
  struct t_lwm2m_data item = {0 };

  switch( event )
  {
    case LWM2M_EVENT_IDLE:
      switch( p->state )
      {
        case INIT_CONN:
          res = lwm2m_init_server_connection( p );
          if( res < 0 )
            return -1;
          p->trans_id = 0x0000;
          p->reg_timestamp = timestamp;
          p->state = POST_RD;
        /* fall-thru */
        case POST_RD:
          res = lwm2m_create_rd_request( p );
          if( res < 0 )
            return -1;
          p->state = WAIT_RD_ACK;
        break;
        case POST_UPDATE_RD:
          res = lwm2m_create_rd_update( p );
          if( res < 0 )
            return -1;
          p->state = WAIT_RD_ACK;
        break;
        default:
        {
          uint32_t regt = 0;

          /* check observe data action */
          (void)lwm2m_object_observe_content( p, &item, timestamp );

          res = lwm2m_read_object_int( p, LWM2M_SERVER_OBJECT, 0, LWM2M_SERVER_LIFETIME, &regt );
          if( res < 0 )
            break;
          regt *= 1000u; /* to mS */
          if( (timestamp - p->reg_timestamp ) < regt )
            break;
          p->state = ( p->state  == WAIT_RD_ACK )? POST_RD: POST_UPDATE_RD;
          p->reg_timestamp = timestamp;
        }
        break;
      }
    break;
    case LWM2M_EVENT_RX:
      res = lwm2m_recv_packet( p, 1000 );
      if( res <= 0 )
        break;

      switch( p->coap.type )
      {
        case COAP_TYPE_ACK:
          if( p->state == WAIT_RD_ACK )
          {
            res = lwm2m_process_reg_ack( p );
            if(res < 0)
            {
              p->state = POST_RD;
              break;
            }
            p->state = WAIT_NOTHING;
          }
          break;
        case COAP_TYPE_RST:
          /* cancel observe */
          item.p_obj = lwm2m_find_object_by_index( p, LWM2M_GET_OBJECT_INDEX_FROM_MID( p->coap.mid ) );
          if( item.p_obj && item.p_obj->observe )
          {
            item.instance = LWM2M_GET_OBJECT_INSTANCE_FROM_MID( p->coap.mid );
            item.p_obj->observe( &item, LWM2M_OBSERVE_RESET, p->coap.mid  );
          }
          break;
        case COAP_TYPE_CON:
        case COAP_TYPE_NON:
          res = lwm2m_process_object_command( p, &item );
          if( res < 0 )
            return lwm2m_server_simple_response( p, COAP_405_METHOD_NOT_ALLOWED );

          switch( res )
          {
            case LWM2M_EXEC:
              res = item.p_obj->exec( &item );
              return lwm2m_server_simple_response( p, (res < 0 ) ? COAP_404_NOT_FOUND: COAP_204_CHANGED );
            case LWM2M_WRITE_ATTRIBUTES:
              res = lwm2m_object_write_attributes( p, &item );
              return lwm2m_server_simple_response( p, (res < 0 ) ? COAP_400_BAD_REQUEST: COAP_204_CHANGED );
            case LWM2M_WRITE:
              res = lwm2m_tlv_walker( &item, p->coap.p_payload, p->coap.size_payload );
              return lwm2m_server_simple_response( p, (res < 0 ) ? COAP_404_NOT_FOUND: COAP_204_CHANGED );
            case LWM2M_OBSERVE:
              if( !item.p_obj->observe )
                return lwm2m_server_simple_response( p, COAP_405_METHOD_NOT_ALLOWED );
              /* save token context */
              item.data = p->coap.token;
              item.size = p->coap.tkl;
              if( item.p_obj->observe( &item, LWM2M_OBSERVE_SET_TOKEN, timestamp ) < 0 )
                return lwm2m_server_simple_response( p, COAP_404_NOT_FOUND );
              /* fall-thru */
            case LWM2M_READ:
              return lwm2m_object_provide_content( p, &item, 1, res == LWM2M_OBSERVE );
            case LWM2M_CREATE:
            case LWM2M_DELETE:
            case LWM2M_DISCOVER:
            default:
              return lwm2m_server_simple_response( p, COAP_405_METHOD_NOT_ALLOWED );
          }
      }
  }
  return 0;
}
