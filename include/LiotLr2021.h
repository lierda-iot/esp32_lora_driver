/*
 * Public single-entry header for the LiotLr2021 ESP-IDF component.
 *
 * Applications only need:
 *   #include "LiotLr2021.h"
 *
 * This header aggregates the public RAC API, RAC, RALF, and RAL layers.
 */

#ifndef LIOT_LR2021_H
#define LIOT_LR2021_H

#include "sdkconfig.h"

#include "../modem_hal/smtc_modem_hal.h"
#include "../rac_api/smtc_rac_api.h"
#include "../rac/smtc_rac.h"
#include "../ralf/ralf.h"
#include "../ral/ral.h"

#if defined(CONFIG_SMTC_RADIO_SX126X)
#include "../ral/sx126x_ral/ral_sx126x.h"
#include "../ralf/sx126x_ralf/ralf_sx126x.h"
#elif defined(CONFIG_SMTC_RADIO_LR11XX)
#include "../ral/lr11xx_ral/ral_lr11xx.h"
#include "../ralf/lr11xx_ralf/ralf_lr11xx.h"
#elif defined(CONFIG_SMTC_RADIO_LR20XX)
#include "../ral/lr20xx_ral/ral_lr20xx.h"
#include "../ralf/lr20xx_ralf/ralf_lr20xx.h"
#endif

#endif  // LIOT_LR2021_H
