# uLWM2M client library in C

* Low memory footprint
* No dynamic memory allocation
* Highly portable ( linux/windows/bare-metal )
* CMake and Pelles C demo project
* Tested with Leshan Lightweight M2M public server
------------
LIMITS:

* **CREATE**/**DELETE** requests not supported
* No **DTLS** & **bootstrap** server support
* Up to **16** object and up to **16** instances per object
* TLV data encoding scheme only
* No strict protocol implementation