#ifndef __VERSION_CFG_H__
#define __VERSION_CFG_H__

#define FW_MAJOR_VERSION	        0
#define FW_RELEASE_VERSION	        1
#define FW_BULID_VERSION	        1
#define NUM3STR(a,b,c)              #a "." #b "." #c
#define VERBUILDSTR(a,b,c)          NUM3STR(a,b,c)
#define FW_VERSION                  VERBUILDSTR(FW_MAJOR_VERSION,FW_RELEASE_VERSION,FW_BULID_VERSION)

#define HW_VERSION_0100		        0x0100
#define HW_VERSION		            HW_VERSION_0100

#endif

