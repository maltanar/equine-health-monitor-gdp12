// University of Southampton, 2012
// EMECS Group Design Project

#ifndef I2CMANAGER_H
#define I2CMANAGER_H

#if defined(EFM32GG990F1024)
#include "i2cmanagergg990f1024.h"

#elif defined(EFM32TG840F32)
#include "i2cmanagertg840f32.h"

#else
#error "i2cmanager.h: PART NUMBER undefined"
#endif

#endif // I2CMANAGER_H