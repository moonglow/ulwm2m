#pragma once
#include <stdint.h>

/* object ( id == object instance ) */
#define TLV_OBJ_INSTANCE  (0)
/* instanced resource value ( id == resource instance ) */
#define TLV_RES_INSTANCE  (1)
/* value with many instance ( id == resource id )*/
#define TLV_RES_MULTI     (2)
/* single value ( id == resource id ) */
#define TLV_RES_VALUE     (3)
/* convert to host order */
#define TLV_H2N           (0x80)

struct t_tlv_item
{
  uint8_t   id_type;
  uint16_t  id;
  int       size;
  uint8_t   *p;
};

int _tlv_decode_item( uint8_t *data, int size, struct t_tlv_item *p );
int _tlv_encode_item( uint8_t *data, int size, struct t_tlv_item *p );
