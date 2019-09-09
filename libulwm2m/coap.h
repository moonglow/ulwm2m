#pragma once
#include <stdint.h>

#define COAP_DBG_PRINT_PACKET

/* must be confirmed */
#define COAP_TYPE_CON   (0)
/* one-way message, no need to confirm */
#define COAP_TYPE_NON   (1)
/* acknowledge needs for COAP_TYPE_CON */
#define COAP_TYPE_ACK   (2)
/* we got all we need */
#define COAP_TYPE_RST   (3)

#define COAP_GET        (1)
#define COAP_POST       (2)
#define COAP_PUT        (3)
#define COAP_DELETE     (4)

#define COAP_SET_CODE( _x )           (((_x/100)<<5)|(_x%100))
#define COAP_GET_CODE( _x )           (((_x>>5u)&0x7)*100+(_x&0x1F))
#define COAP_MAX_TOKEN                (8)

#define COAP_HEADER_SIZE              (4)
#define COAP_VERSION                  (1)
/* coap options */
#define COAP_OPTION_RESERVED0         (0)
#define COAP_OPTION_IF_MATCH          (1)
#define COAP_OPTION_URI_HOST          (3)
#define COAP_OPTION_ETAG              (4)
#define COAP_OPTION_IF_NONE_MATCH     (5)
#define COAP_OPTION_OBSERVE           (6)
#define COAP_OPTION_URI_PORT          (7)
#define COAP_OPTION_LOCATION_PATH     (8)
#define COAP_OPTION_URI_PATH          (11)
#define COAP_OPTION_CONTENT_FORMAT    (12)
#define COAP_OPTION_MAX_AGE           (14)
#define COAP_OPTION_URI_QUERY         (15)
#define COAP_OPTION_ACCEPT            (17)
#define COAP_OPTION_LOCATION_QUERY    (20)
/* --- rfc7959 --- */
#define COAP_OPTION_BLOCK2            (23)
#define COAP_OPTION_BLOCK1            (27)
#define COAP_OPTION_SIZE2             (28)
/* ---    end    --- */
#define COAP_OPTION_PROXY_URI         (35)
#define COAP_OPTION_PROXY_SCHEME      (39)
#define COAP_OPTION_SIZE1             (60)
#define COAP_OPTION_RESERVED128       (128)
#define COAP_OPTION_RESERVED132       (132)
#define COAP_OPTION_RESERVED136       (136)
#define COAP_OPTION_RESERVED140       (140)

#define COAP_PLAIN_TEXT_FORMAT        (0)
#define COAP_APP_LINK_FORMAT          (40)
#define COAP_OPAQUE_FORMAT            (42)
#define COAP_TLV_FORMAT               (11542)
#define COAP_JSON_FORMAT              (11543)

/* basic response codes */
#define COAP_201_CREATED                     201
#define COAP_202_DELETED                     202
#define COAP_203_VALID                       203
#define COAP_204_CHANGED                     204
#define COAP_205_CONTENT                     205
#define COAP_400_BAD_REQUEST                 400
#define COAP_401_UNAUTHORIZED                401
#define COAP_402_BAD_OPTION                  402
#define COAP_403_FORBIDDEN                   403
#define COAP_404_NOT_FOUND                   404
#define COAP_405_METHOD_NOT_ALLOWED          405
#define COAP_406_NOT_ACCEPTABLE              406
#define COAP_412_PRECONDITION_FAILED         412
#define COAP_413_REQUEST_ENTITY_TOO_LARGE    413
#define COAP_415_UNSUPPORTED_CONTENT_FORMAT  415
#define COAP_500_INTERNAL_SERVER_ERROR       500
#define COAP_501_NOT_IMPLEMENTED             501
#define COAP_502_BAD_GATEWAY                 502
#define COAP_503_SERVICE_UNAVAILABLE         503
#define COAP_504_GATEWAY_TIMEOUT             504
#define COAP_505_PROXYING_NOT_SUPPORTED      505


struct t_coap_option
{
  uint16_t  number;
  uint16_t  size;
  uint8_t   *p_data;
};

struct t_coap_packet
{
  uint8_t   version;
  uint8_t   type;
  uint8_t   tkl;
  /* request/response code */
  uint8_t   rr_code;
  uint16_t  mid;
  uint8_t   token[COAP_MAX_TOKEN];
  /* options */
  uint16_t  size_options;
  uint16_t  size_payload;
  uint8_t   *p_options;
  uint8_t   *p_payload;
};

int coap_init_message( struct t_coap_packet *header, uint8_t type, uint16_t code, uint16_t mid );
int coap_set_header_token( struct t_coap_packet *header, uint8_t *token, uint8_t tkl );
int coap_set_option_buffer( struct t_coap_packet *header, uint8_t *data, int size );
int coap_set_payload_buffer( struct t_coap_packet *header, uint8_t *data, int size );
int coap_get_option_block_size( struct t_coap_packet *header, int adjust_size );
int coap_option_add_int( struct t_coap_packet *header, uint16_t number, uint32_t value );
int coap_option_add_string( struct t_coap_packet *header, uint16_t number, char *s );

int coap_write_packet( struct t_coap_packet *header, uint8_t *data, int size );
int coap_read_packet( struct t_coap_packet *header, uint8_t *data, int size );
uint32_t coap_option_read_int( struct t_coap_option *p );
int coap_read_option( struct t_coap_packet *header, struct t_coap_option *p );
int coap_print_packet( struct t_coap_packet *header );
