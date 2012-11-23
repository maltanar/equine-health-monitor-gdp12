// University of Southampton, 2012
// EMECS Group Design Project

#ifndef __UARTMANAGER_H
#define __UARTMANAGER_H

#if defined(EFM32GG990F1024)
#include "uartmanagergg990f1024.h"

#elif defined(EFM32TG840F32)
#include "uartmanagertg840f32.h"

#else
#error "uartmanager.h: PART NUMBER undefined"
#endif

#endif // __UARTMANAGER_H