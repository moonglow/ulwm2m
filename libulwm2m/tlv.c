/*********************************************************************\
* Copyleft (>) 2019 Roman Ilichev <fxdteam@gmail.com>                 *
*                                                                     *
* This file is part of uLWM2M project                                 *
*                             WTFPL LICENSE v2                        *
\*********************************************************************/
#include "tlv.h"

#define TLV_EX_ID_MASK    (1<<5)
#define TLV_LEN_MASK      (3<<3)
#define TLV_FLAGS_OFFSET  (0)

int _tlv_encode_item( uint8_t *data, int size, struct t_tlv_item *p )
{
  uint8_t   pos;
  
  pos = 1;
  if( size < pos )
    return -1;

  data[TLV_FLAGS_OFFSET] = (p->id_type&3)<<6u;
  if( p->id > 0xFF )
  {
    data[TLV_FLAGS_OFFSET] |= TLV_EX_ID_MASK;
    if( size < (pos+2) )
      return -1;
    data[pos+0] = (p->id>>8u)&0xFF;
    data[pos+1] = p->id&0xFF;
    pos += 2;
  }
  else
  {
    if( size < (pos+1) )
      return -1;
    data[pos] = p->id&0xFF;
    pos += 1;
  }

  if( p->size <= 0x07 )
  {
    data[TLV_FLAGS_OFFSET] |= (p->size&0x07);
  }
  else if( p->size <= 0xFF )
  {
    if( size < (pos+1) )
      return -1;
    data[TLV_FLAGS_OFFSET] |= 8;
    data[pos] = p->size;
    pos += 1;
  } 
  else if( p->size <= 0xFFFF )
  {
    if( size < (pos+2) )
      return -1;
    data[TLV_FLAGS_OFFSET] |= 16;
    data[pos+0] = (p->size>>8u)&0xFF;
    data[pos+1] = p->size&0xFF;
    pos += 2;
  }
  else if( p->size <= 0xFFFFFF )
  {
    if( size < (pos+4) )
      return -1;
    data[TLV_FLAGS_OFFSET] |= 24;
    data[pos+0] = (p->size>>16u)&0xFF;
    data[pos+1] = (p->size>>8u)&0xFF;
    data[pos+2] = p->size&0xFF;
    pos += 3;
  }
  else
  {
    return -1;
  }
  
  if( size < (pos+p->size) )
    return -1;
  
  for( int i = 0; i < p->size; i++ )
  {
#ifndef HOST_IS_BIG_ENDIAN
    if( p->id_type & TLV_H2N )
      data[pos + i] = p->p[p->size - 1 - i];
    else
#endif
      data[pos + i] = p->p[i];
  }

  return pos + p->size;
}

int _tlv_decode_item( uint8_t *data, int size, struct t_tlv_item *p )
{
  uint8_t   pos;
  
  pos = 2;
  if( size < pos )
    return -1;

  p->id_type = data[TLV_FLAGS_OFFSET]>>6;
  p->id = data[TLV_FLAGS_OFFSET+1];

  if( data[TLV_FLAGS_OFFSET] & TLV_EX_ID_MASK )
  {
    if( size < (pos+1) )
      return -1;
    p->id = (p->id<<8)|data[pos];
    pos += 1;
  }
  
  switch( data[TLV_FLAGS_OFFSET]&TLV_LEN_MASK )
  {
    case 0:
      p->size = data[TLV_FLAGS_OFFSET]&0x7;
    break;
    case 8:
      if( size < (pos+1) )
        return -1;

      p->size = data[pos];
      pos += 1;
    break;
    case 16:
      if( size < (pos+2) )
        return -1;

      p->size = (data[pos]<<8)|data[pos+1];
      pos += 2;
    break;
    case 24:
      if( size < (pos+3) )
        return -1;

      p->size = (data[pos]<<16)|(data[pos+1]<<8)|data[pos+2];
      pos += 3;
    break;
    default:
      return -1;
  }
  
  if( size < (pos+p->size) )
    return -1;
  
  p->p = data + pos;
  /* processed size, next item */
  return pos + p->size;
}
