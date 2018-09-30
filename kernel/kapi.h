#pragma once

#include <cstdint>
#include "../bootloader/fsapi.h"

struct KAPI;

struct PAPI {
    void (*run)( KAPI * );
};

enum class ProcessState {
    error,
    pending,
    ready
};

struct ProcessHandle {
    ProcessState state;
    PAPI *api;
    uint32_t pid;
};

struct KAPI {
    void *ipcbuffer;
    ProcessHandle (*createProcess)( const char *name );
    void (*killProcess)( uint32_t pid );
    uint32_t (*getActivePID)();
};

extern KAPI *kapi;

#define isPressedA() 	    ((volatile uint8_t *) 0xA0000020)[9]
#define isPressedB() 	    ((volatile uint8_t *) 0xA0000020)[4]
#define isPressedC() 	    ((volatile uint8_t *) 0xA0000020)[10]
#define isPressedD() 	    ((volatile uint8_t *) 0xA0000000)[1]
#define isPressedUp() 	    ((volatile uint8_t *) 0xA0000020)[13]
#define isPressedDown()     ((volatile uint8_t *) 0xA0000020)[3]
#define isPressedLeft()     ((volatile uint8_t *) 0xA0000020)[25]
#define isPressedRight()    ((volatile uint8_t *) 0xA0000020)[7]
