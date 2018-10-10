#include "kernel.h"
#include "loader.h"

FILE *getIcon( const char *fileName, uint32_t hash );


Loader api = {
    nullptr, 0, 0,
    getIcon
};

extern "C" {

    void __libc_init_array(void);

    __attribute__ ((section(".after_vectors")))
    Loader *__onLoad( Kernel *kapi, Process *dtapi ){
	__libc_init_array();
	return &api;
    }

}