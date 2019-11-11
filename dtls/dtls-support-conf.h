#pragma once
#include <stdint.h>

#ifdef __POCC__
#pragma warn(disable: 2251 2154 )
#endif

#define DTLS_TICKS_PER_SECOND (1000)

typedef struct
{
  uint32_t addr;
  uint16_t port;
}
session_t;

typedef uint32_t dtls_tick_t;
