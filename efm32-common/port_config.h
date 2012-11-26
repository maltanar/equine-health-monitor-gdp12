// University of Southampton, 2012
// EMECS Group Design Project

#ifndef __PORTCONFIG_H
#define __PORTCONFIG_H

#if defined(EFM32GG990F1024)	// Giant Gecko STK and DK
#include "ports_gg990f1024.h"
#warning "Using port configuration for Giant Gecko STK / DK"

#elif defined(EFM32GG332F1024)	// our own PCB
#include "ports_gg332f1024.h"
#warning "Using port configuration for GDP PCB"

#elif defined(EFM32TG840F32)	// Tiny Gecko STK
#include "ports_tg840f32.h"
#warning "Using port configuration for Tiny Gecko STK"

#else
#error "port_config.h: EFM32 part number undefined!"
#endif

#endif // __PORTCONFIG_H