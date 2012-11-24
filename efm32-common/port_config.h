// University of Southampton, 2012
// EMECS Group Design Project

#ifndef __PORTCONFIG_H
#define __PORTCONFIG_H

#if defined(EFM32GG990F1024)
#include "ports_gg990f1024.h"

#elif defined(EFM32TG840F32)
#include "ports_tg840f32.h"

#else
#error "port_config.h: EFM32 part number undefined!"
#endif

#endif // __PORTCONFIG_H