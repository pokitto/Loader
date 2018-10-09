#include "kernel.h"
#include "../desktop/api.h"
#include "api.h"

WEAK void setup( DESKTOP::API * );
WEAK void loop( Kernel *kernel );
API api = { loop };

extern "C" {

    void __libc_init_array(void);

    __attribute__ ((section(".after_vectors")))
    Process *__onLoad( Kernel *kapi, DESKTOP::API *dtapi ){
	__libc_init_array();
	if( setup )
	    setup( dtapi );
	return &api;
    }

}
