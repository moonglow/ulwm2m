#pragma once

#define ARRAY_NELEMS( _x ) ((int)(sizeof( _x )/sizeof( _x[0] )))

/* convert string to int for specified nums of chars */
int _strntoi( const char *s, int n );
int _strtoi( const char *s );
