
#include <mach/mach_types.h>
#include "Branding.h"

// additional layer of macro to ensure that BRANDING_KEXTINFO_KMODNAME gets expanded - this is because the
// KMOD_EXPLICIT_DECL macro uses the stringify operator (#) for the first argument
#define KMOD_DECL_LAYER(KMODNAME, KMODVER, KMODSTART, KMODSTOP) KMOD_EXPLICIT_DECL(KMODNAME, KMODVER, KMODSTART, KMODSTOP)

extern kern_return_t _start(kmod_info_t *ki, void *data);
extern kern_return_t _stop(kmod_info_t *ki, void *data);
 
__attribute__((visibility("default"))) KMOD_DECL_LAYER(BRANDING_KEXTINFO_KMODNAME, BRANDING_KEXTINFO_KMODVERSION, _start, _stop)

__private_extern__ kmod_start_func_t *_realmain = 0;
__private_extern__ kmod_stop_func_t *_antimain = 0;
__private_extern__ int _kext_apple_cc = __APPLE_CC__ ;

