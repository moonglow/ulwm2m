#include "utils.h"

int _strntoi( const char *s, int n )
{
  int v = 0;

  while( n-- )
  {
    char ch = *s++;
    if( ch < '0' || ch > '9' )
      break;
    v *= 10;
    v += ch - '0';
  }

  return v;
}

int _strtoi( const char *s )
{
  int v = 0;
  char ch;

  while( ( ch = *s++ ) != 0 )
  {
    if( ch < '0' || ch > '9' )
      break;
    v *= 10;
    v += ch - '0';
  }

  return v;
}
