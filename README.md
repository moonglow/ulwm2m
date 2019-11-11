# uLWM2M client library in C
[![Build Status](https://travis-ci.org/moonglow/ulwm2m.svg?branch=master)](https://travis-ci.org/moonglow/ulwm2m)

* Low memory footprint
* No dynamic memory allocation
* Highly portable ( linux/windows/bare-metal )
* CMake and Pelles C demo project
* DTLS support ( tinydlts lib )
* Tested with Leshan Lightweight M2M public server
------------
LIMITS:

* **CREATE**/**DELETE** requests not supported
* No **bootstrap** server support
* Up to **31** instances per object
* TLV data encoding scheme only
* No strict protocol implementation
