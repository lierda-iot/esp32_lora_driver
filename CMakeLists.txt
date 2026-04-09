set(RADIO_HAL_SRCS "radio_hal/radio_utilities.c")
set(PRIV_RADIO_HAL_INCLUDES "radio_hal")
set(PUB_RADIO_HAL_INCLUDES "")

set(RADIO_DRIVER_SRCS "")
set(PRIV_RADIO_DRIVER_INCLUDES "")
set(PUB_RADIO_DRIVER_INCLUDES "")

set(RAL_SRCS "")
set(PRIV_RAL_INCLUDES "")
set(PUB_RAL_INCLUDES "ral")

set(RALF_SRCS "")
set(PRIV_RALF_INCLUDES "")
set(PUB_RALF_INCLUDES "ralf")

if(CONFIG_SMTC_RADIO_SX126X)
    list(APPEND RADIO_HAL_SRCS
        radio_hal/sx126x_hal/sx126x_hal.c
    )
    list(APPEND PUB_RADIO_HAL_INCLUDES
        radio_hal/sx126x_hal
    )

    list(APPEND RADIO_DRIVER_SRCS
        radio_drivers/sx126x_driver/src/lr_fhss_mac.c
        radio_drivers/sx126x_driver/src/sx126x_bpsk.c
        radio_drivers/sx126x_driver/src/sx126x_driver_version.c
        radio_drivers/sx126x_driver/src/sx126x_lr_fhss.c
        radio_drivers/sx126x_driver/src/sx126x.c
    )
    list(APPEND PUB_RADIO_DRIVER_INCLUDES
        radio_drivers/sx126x_driver/inc
    )

    list(APPEND RAL_SRCS 
        ral/sx126x_ral/ral_sx126x.c
        ral/sx126x_ral/ral_sx126x_bsp.c
    )
    list(APPEND PUB_RAL_INCLUDES 
        ral/sx126x_ral
    )

    list(APPEND RALF_SRCS 
        ralf/sx126x_ralf/ralf_sx126x.c
    )
    list(APPEND PUB_RALF_INCLUDES 
        ralf/sx126x_ralf
    )

elseif(CONFIG_SMTC_RADIO_LR11XX)
    list(APPEND RADIO_HAL_SRCS
        radio_hal/lr11xx_hal/lr11xx_hal.c
    )
    list(APPEND PUB_RADIO_HAL_INCLUDES
        radio_hal/lr11xx_hal
    )

    list(APPEND RADIO_DRIVER_SRCS
        radio_drivers/lr11xx_driver/src/lr11xx_bootloader.c
        radio_drivers/lr11xx_driver/src/lr11xx_crypto_engine.c
        radio_drivers/lr11xx_driver/src/lr11xx_driver_version.c
        radio_drivers/lr11xx_driver/src/lr11xx_gnss.c
        radio_drivers/lr11xx_driver/src/lr11xx_lr_fhss.c
        radio_drivers/lr11xx_driver/src/lr11xx_radio_timings.c
        radio_drivers/lr11xx_driver/src/lr11xx_radio.c
        radio_drivers/lr11xx_driver/src/lr11xx_regmem.c
        radio_drivers/lr11xx_driver/src/lr11xx_rttof.c
        radio_drivers/lr11xx_driver/src/lr11xx_system.c
        radio_drivers/lr11xx_driver/src/lr11xx_wifi.c
    )
    list(APPEND PUB_RADIO_DRIVER_INCLUDES
        radio_drivers/lr11xx_driver/inc
    )

    list(APPEND RAL_SRCS 
        ral/lr11xx_ral/ral_lr11xx.c
        ral/lr11xx_ral/ral_lr11xx_bsp.c
    )
    list(APPEND PUB_RAL_INCLUDES 
        ral/lr11xx_ral
    )

    list(APPEND RALF_SRCS 
        ralf/lr11xx_ralf/ralf_lr11xx.c
    )
    list(APPEND PUB_RALF_INCLUDES 
        ralf/lr11xx_ralf
    )

elseif(CONFIG_SMTC_RADIO_LR20XX)
    list(APPEND RADIO_HAL_SRCS
        radio_hal/lr20xx_hal/lr20xx_hal.c
    )
    list(APPEND PUB_RADIO_HAL_INCLUDES
        radio_hal/lr20xx_hal
    )

    list(APPEND RADIO_DRIVER_SRCS
        radio_drivers/lr20xx_driver/src/lr20xx_driver_version.c
        radio_drivers/lr20xx_driver/src/lr20xx_radio_bluetooth_le.c
        radio_drivers/lr20xx_driver/src/lr20xx_radio_bpsk.c
        radio_drivers/lr20xx_driver/src/lr20xx_radio_common.c
        radio_drivers/lr20xx_driver/src/lr20xx_radio_fifo.c
        radio_drivers/lr20xx_driver/src/lr20xx_radio_flrc.c
        radio_drivers/lr20xx_driver/src/lr20xx_radio_fsk.c
        radio_drivers/lr20xx_driver/src/lr20xx_radio_lora.c
        radio_drivers/lr20xx_driver/src/lr20xx_radio_lr_fhss.c
        radio_drivers/lr20xx_driver/src/lr20xx_radio_ook.c
        radio_drivers/lr20xx_driver/src/lr20xx_radio_oqpsk_15_4.c
        radio_drivers/lr20xx_driver/src/lr20xx_radio_wi_sun.c
        radio_drivers/lr20xx_driver/src/lr20xx_radio_wm_bus.c
        radio_drivers/lr20xx_driver/src/lr20xx_radio_z_wave.c
        radio_drivers/lr20xx_driver/src/lr20xx_regmem.c
        radio_drivers/lr20xx_driver/src/lr20xx_rttof.c
        radio_drivers/lr20xx_driver/src/lr20xx_system.c
        radio_drivers/lr20xx_driver/src/lr20xx_workarounds.c
    )
    list(APPEND PUB_RADIO_DRIVER_INCLUDES
        radio_drivers/lr20xx_driver/inc
    )

    list(APPEND RAL_SRCS 
        ral/lr20xx_ral/ral_lr20xx.c
    )
    if(CONFIG_LIERDA_BOARD_LRMWP35_FANN4)
        list(APPEND RAL_SRCS
            ral/lr20xx_ral/ral_lr20xx_bsp_lrmwp35_fann4.c
        )
    elseif(CONFIG_LIERDA_BOARD_LRMAM36_FANN4)
        list(APPEND RAL_SRCS
            ral/lr20xx_ral/ral_lr20xx_bsp_lrmam36_fann4.c
        )
    else()
        list(APPEND RAL_SRCS
            ral/lr20xx_ral/ral_lr20xx_bsp.c
        )
    endif()
    list(APPEND PUB_RAL_INCLUDES 
        ral/lr20xx_ral
    )

    list(APPEND RALF_SRCS 
       ralf/lr20xx_ralf/ralf_lr20xx.c
    )
    list(APPEND PUB_RALF_INCLUDES 
        ralf/lr20xx_ralf
    )
endif()

set(RAC_SRCS   "")
set(PRIV_RAC_INCLUDES "")
set(PUB_RAC_INCLUDES "rac")

set(RAC_API_SRCS "")
set(PRIV_RAC_API_INCLUDES "")
set(PUB_RAC_API_INCLUDES "rac_api")

set(RADIO_PLANNER_SRCS "")
set(PRIV_RADIO_PLANNER_INCLUDES "")
set(PUB_RADIO_PLANNER_INCLUDES "radio_planner")

list(APPEND RAC_SRCS
    rac/smtc_rac_flrc.c
    rac/smtc_rac_fsk.c
    rac/smtc_rac_lbt.c
    rac/smtc_rac_lora.c
    rac/smtc_rac_lrfhss.c
    rac/smtc_rac.c
)

list(APPEND RADIO_PLANNER_SRCS
    radio_planner/duty_cycle.c
    radio_planner/radio_planner.c
)

idf_component_register(
    SRCS
        modem_hal/smtc_modem_hal.c
        ${RADIO_HAL_SRCS}
        ${RADIO_DRIVER_SRCS}
        ${RAL_SRCS}
        ${RALF_SRCS}
        ${RAC_SRCS}
        ${RAC_API_SRCS}
        ${RADIO_PLANNER_SRCS}

    INCLUDE_DIRS
        include
        ${PUB_RADIO_HAL_INCLUDES}
        ${PUB_RADIO_DRIVER_INCLUDES}
        ${PUB_RAL_INCLUDES}
        ${PUB_RALF_INCLUDES}
        ${PUB_RAC_INCLUDES}
        ${PUB_RAC_API_INCLUDES}
        ${PUB_RADIO_PLANNER_INCLUDES}

    PRIV_INCLUDE_DIRS
        modem_hal
        ${PRIV_RADIO_HAL_INCLUDES}
        ${PRIV_RADIO_DRIVER_INCLUDES}
        ${PRIV_RAL_INCLUDES}
        ${PRIV_RALF_INCLUDES}
        ${PRIV_RAC_INCLUDES}
        ${PRIV_RAC_API_INCLUDES}
        ${PRIV_RADIO_PLANNER_INCLUDES}

    REQUIRES

    PRIV_REQUIRES
       esp_driver_gpio
       esp_driver_spi
       freertos
       esp_timer
)


# ------------------------------------------------------------------
# RAC logging profile mapping
# ------------------------------------------------------------------

if(CONFIG_SMTC_RAC_LOG_OFF)
    target_compile_definitions(${COMPONENT_LIB} PRIVATE
        RAC_FSK_LOG_ENABLE=0
        RAC_LORA_LOG_ENABLE=0
        RAC_LRFHSS_LOG_ENABLE=0
    )
elseif(CONFIG_SMTC_RAC_LOG_MINIMAL)
    target_compile_definitions(${COMPONENT_LIB} PRIVATE
        RAC_FSK_LOG_ENABLE=0
        RAC_LORA_LOG_ENABLE=1
        RAC_LRFHSS_LOG_ENABLE=0
    )
elseif(CONFIG_SMTC_RAC_LOG_VERBOSE OR CONFIG_SMTC_RAC_LOG_ALL)
    target_compile_definitions(${COMPONENT_LIB} PRIVATE
        RAC_FSK_LOG_ENABLE=1
        RAC_LORA_LOG_ENABLE=1
        RAC_LRFHSS_LOG_ENABLE=1
    )
endif()
