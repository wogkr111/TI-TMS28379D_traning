#ifndef MY_COMMON_H_
#define MY_COMMON_H_

// IDE default include
#include "driverlib.h"
#include "device.h"
#include "board.h"
#include "c2000ware_libraries.h"

// user include 
#include <stdint.h>
#include <string.h> // memset


/************************     for DEBUG Option    **********************/
#define INCLUDE_DEBUG_OPTION                false
#if INCLUDE_DEBUG_OPTION == true
    #warning include DEBUG OPTION
    #define USE_FORCE_IDS_DISABLE_BUSY      true
#endif
/************************************************************************/



#define ARRAY_LEN(x)    (sizeof(x)/sizeof((x)[0]))

#endif /* MY_COMMON_H_ */
