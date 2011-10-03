
#include <mach/mach_types.h>
#include "Branding.h"

extern kern_return_t _start(kmod_info_t *ki, void *data);
extern kern_return_t _stop(kmod_info_t *ki, void *data);
 
__attribute__((visibility("default"))) KMOD_EXPLICIT_DECL(BRANDING_KEXTINFO_KMODNAME, BRANDING_KEXTINFO_KMODVERSION, _start, _stop)

__private_extern__ kmod_start_func_t *_realmain = 0;
__private_extern__ kmod_stop_func_t *_antimain = 0;
__private_extern__ int _kext_apple_cc = __APPLE_CC__ ;

