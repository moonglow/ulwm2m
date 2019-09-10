/*********************************************************************\
* Copyleft (>) 2019 Roman Ilichev <fxdteam@gmail.com>                 *
*                                                                     *
* This file is part of uLWM2M project                                 *
*                             WTFPL LICENSE v2                        *
\*********************************************************************/
#include "coap.h"

static void *coap_memcpy( void *dst, void *src, int size )
{
  uint8_t *d = dst, *s = src;
  for( int i = 0; i < size; i++ )
  {
    d[i] = s[i];
  }

  return dst;
}

static void *coap_memclear( void *dst, int size )
{
  uint8_t *p = (uint8_t*)dst;
  for( int i = 0; i < size; i++ )
  {
    p[i] = 0x00;
  }

  return dst;
}

static char* coap_strchr( char *s, int ch )
{
  for( int i = 0; s[i]; i++ )
  {
    if( ch == s[i] )
      return &s[i];
  }
  return 0;
}

static int coap_strlen( char *s )
{
  int len = 0;

  if( !s )
    return 0;

  while( *s++ )
    ++len;

  return len;
}

static char *coap_ltrim( char *s, char *psz_delim )
{
  if( !s )
    return 0;

  while( coap_strchr( psz_delim, *s ) )
  {
    ++s;
  }

  return s;
}

static int coap_opt_decode_field( uint16_t *val, uint8_t *data, uint16_t size )
{
  switch( *val )
  {
    case 13:
      if( !size )
        return -1;
        *val = data[0] + 13;
      return 1;
      case 14:
        if( size < 2 )
          return -1;
        *val = ((data[0]<<8) + 269)+data[1];
        return 2;
      break;
      case 15:
        return -1;
  }

  return 0;
}

static int coap_opt_encode_field( uint16_t val, uint8_t *data, int size )
{
  if( val < 13 )
  {
    if( size < 1 )
      return -1;
    return (0<<4)|val;
  }
  else if( val < 269 )
  {
    if( size < 2)
      return -1;
    val -= 13;
    data[0] = val&0xFF;
    return (1<<4)|(13);
  }
  else
  {
    if( size < 3)
      return -1;
    val -= 269;
    data[0] = val>>8;
    data[1] = val&0xFF;
    return (2<<4)|(14);
  }
}

uint32_t coap_option_read_int( struct t_coap_option *p )
{
  uint32_t value = 0;
  uint8_t *data = p->p_data;

  switch( p->size )
  {
    case 4:
      value |= *data++;
      /* fall-thru */
    case 3:
      value <<= 8u;
      value |= *data++;
      /* fall-thru */
    case 2:
      value <<= 8u;
      value |= *data++;
      /* fall-thru */
    case 1:
      value <<= 8u;
      value |= *data++;
      /* fall-thru */
    case 0:
      break;
    default:
      return -1;
  }

  return value;
}

static int coap_option_is_valid( int option )
{
	switch( option )
	{
		case COAP_OPTION_RESERVED0:
		case COAP_OPTION_IF_MATCH:
		case COAP_OPTION_URI_HOST:
		case COAP_OPTION_ETAG:
		case COAP_OPTION_IF_NONE_MATCH:
    case COAP_OPTION_OBSERVE:
		case COAP_OPTION_URI_PORT:
		case COAP_OPTION_LOCATION_PATH:
		case COAP_OPTION_URI_PATH:
		case COAP_OPTION_CONTENT_FORMAT:
		case COAP_OPTION_MAX_AGE:
		case COAP_OPTION_URI_QUERY:
		case COAP_OPTION_ACCEPT:
		case COAP_OPTION_LOCATION_QUERY:
		case COAP_OPTION_BLOCK2:
		case COAP_OPTION_BLOCK1:
		case COAP_OPTION_SIZE2:
		case COAP_OPTION_PROXY_URI:
		case COAP_OPTION_PROXY_SCHEME:
		case COAP_OPTION_SIZE1:
		case COAP_OPTION_RESERVED128:
		case COAP_OPTION_RESERVED132:
		case COAP_OPTION_RESERVED136:
		case COAP_OPTION_RESERVED140:
			return 1;
	}
	
	return 0;
}

int coap_read_option( struct t_coap_packet *header, struct t_coap_option *p )
{
  uint8_t *data, *begin;
  uint16_t len ,delta, size;
  int res;
  
  /* first read ? */
  if( !p->p_data )
  {
    size = header->size_options;
    data = header->p_options;
  }
  else
  {
    size = header->size_options - (( p->p_data - header->p_options ) + p->size);
    data = p->p_data + p->size;
  }

  /* start offset */
  begin = data;
  
  /* 0 - no more options ( no payload case ), < 0 - error */
  if( size < 1 )
    return size;
  
  /* end marker, no more options */
  if( *data == 0xFF )
    return 0;

  len = *data&0x0F;
  delta = *data>>4u;

  data += 1;
  size -= 1;

  res = coap_opt_decode_field( &delta, data, size );
  if( res < 0 )
    return -1;

  if( size < res )
    return -1;

  data += res;
  size -= res;

  res = coap_opt_decode_field( &len, data, size );
  if( res < 0 )
    return -1;

  data += res;
  size -= res;

  if( size < len )
    return -1;

  p->number += delta;
  p->size = len;
  p->p_data = data;
  
  if( !coap_option_is_valid( p->number ) )
    return -1;

  /* return option block size */
  return (uint16_t)( p->p_data - begin ) + p->size;
}

int coap_add_option( struct t_coap_packet *header, uint16_t number, void *p_opt, int opt_size )
{
  int res, offset;
  uint8_t *data = header->p_options;
  uint16_t size = header->size_options;
  struct t_coap_option opt = { 0 };

  /* find latest record */
  offset = 0;
  for( res = -1; res != 0; )
  {
    res = coap_read_option( header, &opt );
    if( res < 0 )
      return -1;

    offset += res;
  }

  if( size < offset )
    return -1;

  size -= offset;
  data += offset;

  /* write option header */
  offset = 1;
  res = coap_opt_encode_field( number - opt.number, data+offset, size - offset );
  if( res < 0 )
    return -1;
     
  /* encode delta value */
  data[0] = (res&0x0F)<<4;
  offset += res>>4;

  res = coap_opt_encode_field( opt_size, data+offset, size - offset );
  if( res < 0 )
    return -1;

  /* encode len value */
  data[0] |= (res&0x0F);
  offset += res>>4;
    
  size -= offset;
  data += offset;
    
  if( size < opt_size )
    return -1;
  
  coap_memcpy( data, p_opt, opt_size );

  size -= opt_size;
  data += opt_size;

  if( size < 1 )
    return -1;

  *data = 0xFF;
  
  return 0;
}

/* le to network order */
int coap_option_add_int( struct t_coap_packet *header, uint16_t number, uint32_t value )
{
  uint8_t buffer[sizeof(uint32_t)];
  uint8_t size = 0;

  if( value & 0xFF000000 )
    buffer[size++] = (value>>24u)&0xFF;
  if( value & 0xFFFF0000 )
    buffer[size++] = (value>>16u)&0xFF;
  if( value & 0xFFFFFF00 )
    buffer[size++] = (value>>8u)&0xFF;
  if( value & 0xFFFFFFFF )
    buffer[size++] = (value>>0u)&0xFF;

  return coap_add_option( header, number, buffer, size );
}

static int coap_add_split_string( struct t_coap_packet *header, uint16_t number, char *s, char split )
{
  char *e;
  int len;

  while( *s )
  {
    s = coap_ltrim( s, (char[]){split,0} );
    if( !s || !s[0])
      return 0;
    e = coap_strchr( s, split );
    len = e ? e-s: coap_strlen( s );
    if( coap_add_option( header, number, s, len ) < 0 )
      return -1;
    s += len;
  }

  return 0;
}

int coap_option_add_string( struct t_coap_packet *header, uint16_t number, char *s )
{
  switch( number )
  {
    case COAP_OPTION_LOCATION_PATH:
    case COAP_OPTION_URI_PATH:
      return coap_add_split_string( header, number, s, '/' );
    case COAP_OPTION_URI_QUERY:
    case COAP_OPTION_LOCATION_QUERY:
      return coap_add_split_string( header, number, s, '&' );
    case COAP_OPTION_URI_HOST:
    case COAP_OPTION_PROXY_URI:
    case COAP_OPTION_PROXY_SCHEME:
      return coap_add_option( header, number, s, coap_strlen( s ) );
    default:
      return 0;
  }
}

int coap_init_message( struct t_coap_packet *header, uint8_t type, uint16_t code, uint16_t mid )
{
  /* TODO: need fix, tkl and token data preserved */
  header->version = COAP_VERSION;
  header->type = type;
  header->rr_code = COAP_SET_CODE( code );
  header->mid = mid;
  
  header->size_options = 0;
  header->size_payload = 0;
  header->p_options = (void*)0;
  header->p_payload = (void*)0;

  return 0;
}

int coap_set_header_token( struct t_coap_packet *header, uint8_t *token, uint8_t tkl )
{
  if( tkl > COAP_MAX_TOKEN )
    return 0;

  header->tkl = tkl;
  
  coap_memcpy( header->token, token, tkl );

  return 0;
}

int coap_set_option_buffer( struct t_coap_packet *header, uint8_t *data, int size )
{
  if( size < 1 )
    return -1;

  /* end marker by default */
  data[0] = 0xFF;

  header->p_options = data;
  header->size_options = size;

  return size;
}

int coap_set_payload_buffer( struct t_coap_packet *header, uint8_t *data, int size )
{
  if( size < 1 )
    return -1;

  /* start payload mark */
  data[0] = 0xFF;

  size -= 1;
  data += 1;

  header->p_payload = data;
  header->size_payload = size;

  return size;
}

int coap_get_option_block_size( struct t_coap_packet *header, int adjust_size )
{
  int size = 0, res;
  struct t_coap_option opt = { 0 };

  for( res = -1; res != 0; )
  {
    res = coap_read_option( header, &opt );
    if( res < 0 )
      return -1;

    size += res;
  }

  if( adjust_size )
    header->size_options = size;

  return size;
}

int coap_write_packet( struct t_coap_packet *header, uint8_t *data, int size )
{
  int data_size = size;

  if( size < COAP_HEADER_SIZE )
    return -1;
  
  data[0]  = (header->version&0x03)<<6;
  data[0] |= (header->type&0x03)<<4;
  data[0] |= (header->tkl&0x0F);
  data[1] =  header->rr_code;
  data[2] = (header->mid>>8);
  data[3] = (header->mid&0xFF);

  size -= 4;
  data += 4;

  if( size < header->tkl )
    return -1;

  coap_memcpy( data, header->token, header->tkl ); 
  
  size -= header->tkl;
  data += header->tkl;

  if( size < 1 )
    return -1;
  
  /* write options from another mem ? */
  if( data != header->p_options )
  {
    if( size < header->size_options )
      return -1;
    coap_memcpy( data, header->p_options, header->size_options );
  }

  size -= header->size_options;
  data += header->size_options;

  if( header->size_payload )
  {
    /* start payload marker */
    data[0] = 0xFF;
    size -= 1;
    data += 1;

    /* write payload from another mem ? */
    if( data != header->p_payload )
    {
      if( size < header->size_payload )
        return -1;
      coap_memcpy( data, header->p_payload, header->size_payload );
    }
  }

  size -= header->size_payload;
  data += header->size_payload;
  (void)data;

  /* calculate consumed size */
  return data_size - size;
}

int coap_read_packet( struct t_coap_packet *header, uint8_t *data, int size )
{
  int res;
  struct t_coap_option opt = { 0 };

  if( size < COAP_HEADER_SIZE )
    return -1;

  coap_memclear( header, sizeof( struct t_coap_packet ) );

  header->version = (data[0]>>6u)&0x03;
  header->type    = (data[0]>>4u)&0x03;
  header->tkl     = data[0]&0x0F;
  header->rr_code = data[1];
  header->mid     = (data[2]<<8u)|data[3];

  if( header->version != COAP_VERSION )
    return -1;

  /* skip header */
  data += 4;
  size -= 4;

  /* read token */
  if( header->tkl > COAP_MAX_TOKEN || size < header->tkl )
    return -1;

  for( uint8_t i = 0; i < header->tkl; i++ )
    header->token[i] = data[i];

  /* skip token */
  data += header->tkl;
  size -= header->tkl;

  header->p_options = data;
  header->size_options = size;

  /* check options */
  for( res = -1; res && size; )
  {
    res = coap_read_option( header, &opt );
    if( res < 0 )
      return -1;
    data += res;
    size -= res;
  }

  if( size < 0 )
    return -1;
  /* without payload ? */
  if( !size )
    return 1;

  /* invalid payload marker */
  if( *data != 0xFF )
    return -1;

  data += 1;
  size -= 1;

  /* empty payload o_O ? */
  if( !size  )
    return -1;

  /* adjust options block size */
  header->size_options -= size;
  /* set payload position */
  header->size_payload = size;
  header->p_payload = data;

  return 1;
}

#ifdef COAP_DBG_PRINT_PACKET
#include <stdio.h>
int coap_print_packet( struct t_coap_packet *header )
{
  int res;
  char format[16];
  struct t_coap_option opt = { 0 };

  printf( "request/response code: %.3u\n", COAP_GET_CODE( header->rr_code ) );
  printf( "message id: %u\n", header->mid );
  printf( "options:\n" );


  for( res = -1; res != 0; )
  {
    res = coap_read_option( header, &opt );
    if( res < 0 )
      return -1;
    if( !res )
      break;

    printf( "\toption number: %u\n", opt.number );
    printf( "\toption size: %u\n", opt.size );

    switch( opt.number )
    {
      case COAP_OPTION_RESERVED0:
      case COAP_OPTION_RESERVED128:
      case COAP_OPTION_RESERVED132:
      case COAP_OPTION_RESERVED136:
      case COAP_OPTION_RESERVED140:
      case COAP_OPTION_IF_NONE_MATCH:
      case COAP_OPTION_IF_MATCH:
        break;
      case COAP_OPTION_ETAG:
        printf( "\tbinary: " );
        for( int i = 0; i <  opt.size; i++ )
        {
          printf( "%.2X ", opt.p_data[i] );
        }
        printf( "\n" );
      break;
      case COAP_OPTION_URI_HOST:
      case COAP_OPTION_PROXY_URI:
      case COAP_OPTION_PROXY_SCHEME:
      case COAP_OPTION_LOCATION_PATH:
      case COAP_OPTION_URI_PATH:
      case COAP_OPTION_URI_QUERY:
      case COAP_OPTION_LOCATION_QUERY:
        sprintf( format, "\tstring: %%.%ds\n", opt.size );
        printf( format, (char*)opt.p_data );
      break;
      case COAP_OPTION_OBSERVE:
      case COAP_OPTION_URI_PORT:
      case COAP_OPTION_MAX_AGE:
      case COAP_OPTION_CONTENT_FORMAT:
      case COAP_OPTION_ACCEPT:
      case COAP_OPTION_BLOCK2:
      case COAP_OPTION_BLOCK1:
      case COAP_OPTION_SIZE1:
      case COAP_OPTION_SIZE2:
        printf( "\tuint: %u\n", coap_option_read_int( &opt ) );
      break;
      default:
        return -1;
    }
    printf( "---------------\n" );
  }

  printf( "payload size: %u\n", header->size_payload );
#if 0
  printf( "payload:" );
  for( int i = 0; i < header->size_payload; i++ )
  {
    if( (i % 16) == 0 )
      printf( "\n" );
    printf( "%.2X ", header->p_payload[i] );
  }

  printf( "\ntext:\n" );

  sprintf( format, "%%.%ds\n", header->size_payload );
  printf( format, (char*)header->p_payload );
#endif
  return 0;
}
#else
int coap_print_packet( struct t_coap_packet *header ){ (void)header; return 0; }
#endif

