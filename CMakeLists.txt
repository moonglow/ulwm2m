cmake_minimum_required( VERSION 3.12 )
project( lwm2m_client C)

set(CMAKE_C_STANDARD 11)

include_directories( libulwm2m dtls dtls/tinydtls example )

set( LIBULWM2M_SOURCE
        libulwm2m/coap.c
        libulwm2m/utils.c
        libulwm2m/lwm2m.c
        libulwm2m/tlv.c
        libulwm2m/object_security.c
        libulwm2m/object_server.c
        libulwm2m/object_device.c
        )

set( LIBTINYDTLS
        dtls/tinydtls/aes/rijndael.c
        dtls/tinydtls/ecc/ecc.c
        dtls/tinydtls/sha2/sha2.c
        dtls/tinydtls/dtls.c
        dtls/tinydtls/dtls-ccm.c
        dtls/tinydtls/dtls-crypto.c
        dtls/tinydtls/dtls-hmac.c
        dtls/tinydtls/dtls-log.c
        dtls/tinydtls/dtls-peer.c
        dtls/tinydtls/netq.c

        #mem manager
        dtls/lib/memb.c
        )

set( LWM2M_CLIENT_DEMO
        example/object_3300.c
        example/object_3303.c
        example/object_3341.c
        example/main.c
        example/udp.c
        )


#set(CMAKE_C_FLAGS "-lasan -O0 -g -fno-omit-frame-pointer -fsanitize=address ${CMAKE_C_FLAGS}")

add_executable( lwm2m_client ${LWM2M_CLIENT_DEMO} ${LIBULWM2M_SOURCE} ${LIBTINYDTLS} dtls/dtls-support.c )

target_compile_definitions( lwm2m_client PRIVATE
        COAP_DBG_PRINT_PACKET WITH_DTLS
        )

target_compile_options( lwm2m_client PRIVATE
        -Wall
        -Wextra
        -pedantic
        -Wimplicit-fallthrough=3
        -Wno-missing-field-initializers
        -Wno-misleading-indentation
        )
