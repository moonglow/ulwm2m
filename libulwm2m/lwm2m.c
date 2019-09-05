#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "lwm2m.h"
#include "tlv.h"

#define LWM2M_OBSERVE   (100)
#define LWM2M_READ      (101)
#define LWM2M_CREATE    (102)
#define LWM2M_WRITE     (103)
#define LWM2M_EXEC      (104)
#define LWM2M_DELETE    (105)

enum
{
  INIT_CONN,
  POST_RD,
  POST_UPDATE_RD,
  WAIT_RD_ACK,
  WAIT_SERVER_COMMANDS,
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

char *lwm2m_read_item_string( struct t_lwm2m_item *p, char *p_sz, int max_size )
{
  if( p->size >= max_size )
    return (char*)0;

  memcpy( p_sz, p->data, p->size );
  p_sz[p->size] = '\0';

  return p_sz;
}

uint32_t lwm2m_read_item_int( struct t_lwm2m_item *p )
{
  uint8_t *data = p->data;

  if( p->data_type == LWM2M_ITEM_FROM_NETWORK )
  {
#ifndef HOST_IS_BIG_ENDIAN
    switch( p->size )
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
    switch(p->size)
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

float lwm2m_read_item_float( struct t_lwm2m_item *p )
{
  union
  {
    float    fl;
    double   db;
    uint8_t  raw[sizeof(double)];
  }u;

  if( p->size != sizeof(float) && p->size != sizeof(double) )
    return 0.0f;

  for( int i = 0; i < p->size; i++ )
  {
#ifndef HOST_IS_BIG_ENDIAN
    if( p->data_type == LWM2M_ITEM_FROM_NETWORK )
      u.raw[i] = ((uint8_t*)p->data)[p->size - 1 - i];
    else
#endif
      u.raw[i] = ((uint8_t*)p->data)[i];
  }

  if( p->size == sizeof(double) )
    return (float)u.db;

  return u.fl;
}

static int lwm2m_read_object_int( struct t_lwm2m *p, int obj_id, int inst_is, int id, uint32_t *val )
{
  struct t_lwm2m_item itm = { .instance = inst_is, .id = id };

  itm.p_obj = lwm2m_find_object( p, obj_id );
  if( !itm.p_obj || !itm.p_obj->read )
    return -1;

  if( itm.p_obj->read( &itm ) < 0 )
    return -1;

  if( val )
    *val = lwm2m_read_item_int( &itm );

  return 0;
}

static int lwm2m_init_server_connection( struct t_lwm2m *p )
{
  char *s, *e;
  struct t_lwm2m_item itm = { .instance = 0, .id = LWM2M_SECURITY_SERVER_URI };

  if( !p->init )
    return 0;

  itm.p_obj = lwm2m_find_object( p, LWM2M_SECURITY_OBJECT );
  if( !itm.p_obj || !itm.p_obj->read )
    return -1;
  
  if( itm.p_obj->read( &itm ) < 0 )
    return -1;
  
  if( p->mem_size < (itm.size+1) )
    return -1;

  s = strncpy( (char*)p->mem, itm.data, itm.size );
  s[itm.size] = '\0';
  s = strchr( s, '/' );
  if( !s || s[1] != '/' )
    return -1;
  s += 2;

  e = strchr( s, ':' );
  if( !e )
    return -1;

  *e++ = '\0';

  if( p->init( s,  atol( e ) ) < 0 )
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

  if( !p->send )
    return -1;

  size = coap_write_packet( &p->coap, p->mem, p->mem_size );
  if( size < 0 )
    return -1;

  return p->send( p->mem, size );
}

static int lwm2m_create_rd_update( struct t_lwm2m *p )
{
  /* create coap link request */
  coap_init_message( &p->coap, COAP_TYPE_CON, COAP_POST, rand()&0xFFFF );

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
  struct t_lwm2m_item itm = { .instance = 0 };
  char *s;
  int res, size, offset;

  itm.p_obj = lwm2m_find_object( p, LWM2M_SERVER_OBJECT );
  if( !itm.p_obj || !itm.p_obj->read )
    return -1;

  /* create coap link request */
  coap_init_message( &p->coap, COAP_TYPE_CON, COAP_POST, rand()&0xFFFF );

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
  itm.id = LWM2M_SERVER_BINDING;
  res = itm.p_obj->read( &itm );
  if( res < 0 )
    return -1;
  
  strcpy( s, "b=" );
  strncat( s, itm.data, itm.size );
  coap_option_add_string( &p->coap, COAP_OPTION_URI_QUERY, s );

  /* lifetime in seconds */
  itm.id = LWM2M_SERVER_LIFETIME;
  res = itm.p_obj->read( &itm );
  if( res < 0 )
    return -1;

  snprintf( s, temp_buffer_size, "lt=%d", lwm2m_read_item_int( &itm ) );
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

static int lwm2m_process_object_command( struct t_lwm2m *p, struct t_lwm2m_item *p_item )
{
  struct t_coap_option opt = { 0 };
  char sz_temp[8];
  int is_observe = 0, accept = COAP_TLV_FORMAT;
  int depth = 0;
  int res;

  p_item->p_obj = 0;
  p_item->instance = -1;
  p_item->id = -1;

  if( ( p->coap.type != COAP_TYPE_CON ) && ( p->coap.type != COAP_TYPE_NON ) )
    return -1;
  
  /* recognize object path */
  depth = 0;
  for( res = -1; ; )
  {
    res = coap_read_option( &p->coap, &opt );
    if( res < 0 || !res )
      break;
  
    switch( opt.number )
    {
      case COAP_OPTION_URI_PATH:
        if( !opt.size )
          return -1;
        if( sizeof( sz_temp ) <= opt.size )
          return -1;
        strncpy( sz_temp, (char*)opt.p_data, opt.size );
        sz_temp[opt.size] = '\0';

        switch( depth )
        {
          case 0:
            p_item->p_obj = lwm2m_find_object( p, (int)strtol( sz_temp, 0, 10 ) );
            ++depth;
          break;
          case 1:
            p_item->instance = (int)strtol( sz_temp, 0, 10 );
            ++depth;
          break;
          case 2:
            p_item->id = (int)strtol( sz_temp, 0, 10 );
            ++depth;
          break;
          default:
            return -1;
        }
      break;
      case COAP_OPTION_ACCEPT:
        accept = coap_option_read_int( &opt );
      break;
      case COAP_OPTION_LWM2M_OBSERVE:
        is_observe = 1;
      break;
    }
  }
  
  if( !depth )
    return -1;

  /* only TLV for now */ 
  if( accept != COAP_TLV_FORMAT )
    return -1;

  /* valid object in all cases */
  if( !p_item->p_obj )
    return -1;

  switch( p->coap.rr_code )
  {
    case COAP_SET_CODE( COAP_GET ):
      if( is_observe )
        return LWM2M_OBSERVE;
      if( p->coap.size_payload == 0 )
        return p_item->p_obj->read ? LWM2M_READ: -1;
      return -1;
    case COAP_SET_CODE( COAP_PUT ):
      if( depth < 2 )
        return -1;
      if( p->coap.size_payload == 0 )
        return -1;
      return p_item->p_obj->write ? LWM2M_WRITE: -1;
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

int lwm2m_init_content_response( struct t_lwm2m *p, int content_format )
{
  int res, size;

  /* old token size */
  size = p->coap.tkl;
  coap_init_message( &p->coap, COAP_TYPE_ACK, COAP_205_CONTENT, p->coap.mid );
  coap_set_header_token( &p->coap, p->coap.token, size );

  size = p->mem_size - COAP_HEADER_SIZE - p->coap.tkl;

  coap_set_option_buffer( &p->coap, p->mem + size, p->mem_size - size );
  coap_option_add_int( &p->coap, COAP_OPTION_CONTENT_FORMAT, content_format );
    
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

  coap_print_packet( &p->coap );
  return res;
}

static int lwm2m_tlv_walker(struct t_lwm2m_item *p, uint8_t *data, int size )
{
  struct t_tlv_item item;

  p->data_type = LWM2M_ITEM_FROM_NETWORK;

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
        p->instance = item.id;
        p->data = item.p;
        p->size = item.size;
        (void)p->p_obj->write( p );
        break;
      case TLV_RES_MULTI:
        p->id = item.id;
        if(lwm2m_tlv_walker(p, item.p, item.size) < 0 )
          return 0;
        break;
      case TLV_RES_VALUE:
        p->id = item.id;
        p->data = item.p;
        p->size = item.size;
        (void)p->p_obj->write( p );
        break;
    }
  }

  return 0;
}

static int lwm2m_tlv_encode_item( struct t_lwm2m_item *p, uint8_t *data, int size )
{
  struct t_tlv_item tlv = {
          .id = p->id,
          .id_type = TLV_RES_VALUE,
          .size = p->size,
          .p = p->data,
  };

  tlv.id_type |= (p->data_type == LWM2M_ITEM_BINARY) ? 0 : TLV_H2N;

  return _tlv_encode_item( data, size, &tlv );
}

int lwm2m_process( struct t_lwm2m *p, int event, uint32_t timestamp )
{
  int res;

  switch( event )
  {
    case LWM2M_EVENT_TX:
      switch( p->state )
      {
        case INIT_CONN:
          res = lwm2m_init_server_connection( p );
          if( res < 0 )
            return -1;
          p->reg_timestamp = 0;
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
          res = lwm2m_read_object_int( p, LWM2M_SERVER_OBJECT, 0, LWM2M_SERVER_LIFETIME, &regt );
          if( res < 0 )
            break;
          regt *= 1000u; /* to mS, use half interval */
          if( (timestamp - p->reg_timestamp ) < (regt/2) )
            break;
          p->state = p->reg_timestamp ? POST_UPDATE_RD : POST_RD;
        }
        break;
      }
    break;
    case LWM2M_EVENT_RX:
      res = lwm2m_recv_packet( p, 1000 );
      if( res <= 0 )
        break;
      switch( p->state )
      {
        case WAIT_RD_ACK:
          res = lwm2m_process_reg_ack( p );
          if( res < 0 )
          {
            p->state = POST_RD;
            break;
          }
          p->reg_timestamp = timestamp;
          p->state = WAIT_SERVER_COMMANDS;
        break;
        case WAIT_SERVER_COMMANDS:
        {
          struct t_lwm2m_item item = { 0 };
          int offset = 0;

          res = lwm2m_process_object_command( p, &item );
          if( res < 0 )
            return lwm2m_server_simple_response( p, COAP_405_METHOD_NOT_ALLOWED );

          switch( res )
          {
            case LWM2M_EXEC:
              res = item.p_obj->exec( &item );
              return lwm2m_server_simple_response( p, (res < 0 ) ? COAP_404_NOT_FOUND: COAP_204_CHANGED );
            case LWM2M_WRITE:
              res = lwm2m_tlv_walker(&item, p->coap.p_payload, p->coap.size_payload);
              return lwm2m_server_simple_response( p, (res < 0 ) ? COAP_404_NOT_FOUND: COAP_204_CHANGED );
            case LWM2M_OBSERVE: /* same for now */
            case LWM2M_READ:
              res = item.p_obj->read( &item );
              if( res < 0 )
                return lwm2m_server_simple_response( p, COAP_404_NOT_FOUND );
              
              res = lwm2m_init_content_response( p, COAP_TLV_FORMAT );
              if( res < 0 )
                return lwm2m_server_simple_response( p, COAP_413_REQUEST_ENTITY_TOO_LARGE );

              /* read whole object ? */
              if( item.id < 0 )
              {
                for( uint16_t *list = (void*)item.data, n = item.size/sizeof(uint16_t); n; )
                {
                  item.id = list[--n];
                  res = item.p_obj->read( &item );
                  if( res < 0 )
                    continue;

                  res = lwm2m_tlv_encode_item( &item, p->coap.p_payload + offset, p->coap.size_payload - offset );
                  if( res < 0 )
                    break;

                  offset += res;
                }
              }
              else
              {
                res = lwm2m_tlv_encode_item( &item, p->coap.p_payload, p->coap.size_payload );
                offset += res;
              }

              if( res < 0 )
                return lwm2m_server_simple_response( p, COAP_413_REQUEST_ENTITY_TOO_LARGE );
              
              /* update payload size and send */
              p->coap.size_payload = offset;
              return lwm2m_send_coap_msg( p );
            case LWM2M_CREATE:
            case LWM2M_DELETE:
              return lwm2m_server_simple_response( p, COAP_405_METHOD_NOT_ALLOWED );
          }
        }
        break;
      }
    break;
  }
  return 0;
}
