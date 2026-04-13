/**
 * @file      ral_lr20xx_bsp.c
 *
 * @brief     HAL implementation for LR20xx radio chip.
 *
 *
 * The Clear BSD License
 * Copyright Semtech Corporation 2021. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the disclaimer
 * below) provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Semtech corporation nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY
 * THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT
 * NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SEMTECH CORPORATION BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * -----------------------------------------------------------------------------
 * --- DEPENDENCIES ------------------------------------------------------------
 */

#include <stddef.h>
#include <string.h>
#include "lr20xx_hal.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "freertos/FreeRTOS.h"

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE MACROS-----------------------------------------------------------
 */
#define RADIO_SPI_HOST  CONFIG_LR2021_RADIO_SPI_ID

#define PIN_NUM_MOSI    CONFIG_LR2021_SPI_MOSI
#define PIN_NUM_MISO    CONFIG_LR2021_SPI_MISO
#define PIN_NUM_CLK     CONFIG_LR2021_SPI_CLK

#define RADIO_NRST      CONFIG_LR2021_NRST_GPIO
#define RADIO_NSS       CONFIG_LR2021_NSS_GPIO
#define RADIO_BUSY      CONFIG_LR2021_BUSY_GPIO

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE CONSTANTS -------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE TYPES -----------------------------------------------------------
 */

typedef enum
{
    RADIO_SLEEP,
    RADIO_AWAKE
} radio_mode_t;

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE VARIABLES -------------------------------------------------------
 */
static volatile radio_mode_t radio_mode = RADIO_AWAKE;
static spi_device_handle_t radio_spi = NULL;

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DECLARATION -------------------------------------------
 */

/**
 * @brief Wait until radio busy pin returns to 0
 */
static void lr20xx_hal_wait_on_busy( void );

/**
 * @brief Check if device is ready to receive spi transaction.
 * @remark If the device is in sleep mode, it will awake it and wait until it is ready
 */
static void lr20xx_hal_check_device_ready( void );

static void radio_gpio_init(void);
static void radio_spi_bus_init(void);
static void radio_spi_device_init(void);

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITION ---------------------------------------------
 */
lr20xx_hal_status_t lr20xx_hal_init( const void* radio )
{
    radio_gpio_init();

    radio_spi_bus_init();

    radio_spi_device_init();

    return LR20XX_HAL_STATUS_OK;
}

lr20xx_hal_status_t lr20xx_hal_reset( const void* radio )
{
    gpio_set_level( RADIO_NRST, 0 );
    // wait for 1ms
    vTaskDelay(pdMS_TO_TICKS(1));
    gpio_set_level( RADIO_NRST, 1 );

    return LR20XX_HAL_STATUS_OK;
}

lr20xx_hal_status_t lr20xx_hal_wakeup( const void* radio )
{
    // Busy is HIGH in sleep mode, wake-up the device with a small glitch on NSS
    gpio_set_level( RADIO_NSS, 0 );
    // wait for 1ms
    vTaskDelay(pdMS_TO_TICKS(1));
    gpio_set_level( RADIO_NSS, 1 );
    radio_mode = RADIO_AWAKE;
    return LR20XX_HAL_STATUS_OK;
}

lr20xx_hal_status_t IRAM_ATTR lr20xx_hal_read( const void* radio, const uint8_t* cbuffer, const uint16_t cbuffer_length,
                                     uint8_t* rbuffer, const uint16_t rbuffer_length )
{
    lr20xx_hal_check_device_ready( );

    // Put NSS low to start spi transaction
    gpio_set_level( RADIO_NSS, 0 );

    spi_transaction_t t_cmd = {
        .flags = 0,
        .length    = cbuffer_length * 8,
        .tx_buffer = cbuffer,
    };
    spi_device_polling_transmit(radio_spi, &t_cmd);

    gpio_set_level( RADIO_NSS, 1 );

    if( rbuffer_length > 0 )
    {
        lr20xx_hal_wait_on_busy( );
        gpio_set_level( RADIO_NSS, 0 );

        uint8_t dummy[2 + rbuffer_length];
        uint8_t rxbuf[2 + rbuffer_length];
        memset(dummy, 0x00, sizeof(dummy));

        spi_transaction_t r_cmd = {
            .flags = 0,
            .length    = (rbuffer_length + 2) * 8,
            .tx_buffer = dummy,
            .rx_buffer = rxbuf,
        };

        spi_device_polling_transmit(radio_spi, &r_cmd);

        // Put NSS high as the spi transaction is finished
        gpio_set_level( RADIO_NSS, 1 );
        
        memcpy(rbuffer, &rxbuf[2], rbuffer_length);
    }

    return LR20XX_HAL_STATUS_OK;
}

lr20xx_hal_status_t IRAM_ATTR lr20xx_hal_write( const void* radio, const uint8_t* cbuffer, const uint16_t cbuffer_length,
                                      const uint8_t* cdata, const uint16_t cdata_length )
{
    uint16_t total_len = cbuffer_length + cdata_length;
    if( total_len == 0 )
    {
        return LR20XX_HAL_STATUS_OK;
    }

    /* Build a contiguous TX buffer: command + data */
    uint8_t tx_buf[total_len];

    memcpy( tx_buf, cbuffer, cbuffer_length );
    memcpy( tx_buf + cbuffer_length, cdata, cdata_length );

    spi_transaction_t t = {
        .flags = 0,
        .length    = total_len * 8,
        .tx_buffer = tx_buf,
    };

    lr20xx_hal_check_device_ready( );

    // Put NSS low to start spi transaction
    gpio_set_level( RADIO_NSS, 0 );

    // Send command
    spi_device_polling_transmit( radio_spi, &t );

    // Put NSS high as the spi transaction is finished
    gpio_set_level( RADIO_NSS, 1 );

    // Check if command sent is a sleep command LR20XX_SYSTEM_SET_SLEEP_MODE_OC = 0x0127 and save context
    if( ( cbuffer_length >= 2 ) && ( cbuffer[0] == 0x01 ) && ( cbuffer[1] == 0x27 ) )
    {
        radio_mode = RADIO_SLEEP;

        // add a incompressible delay to prevent trying to wake the radio before it is full asleep
        // TODO check if needed
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    return LR20XX_HAL_STATUS_OK;
}

lr20xx_hal_status_t IRAM_ATTR lr20xx_hal_direct_read( const void* radio, uint8_t* data, const uint16_t data_length )
{
    if( data_length == 0 )
    {
        return LR20XX_HAL_STATUS_OK;
    }

    uint8_t dummy[data_length];
    memset(dummy, 0x00, sizeof(dummy));

    spi_transaction_t t = {
        .flags = 0,
        .length    = data_length * 8,  /* Number of generated SPI clock cycles */
        .tx_buffer = dummy,
        .rx_buffer = data,             /* MISO data destination */
    };

    lr20xx_hal_check_device_ready( );

    // Put NSS low to start spi transaction
    gpio_set_level( RADIO_NSS, 0 );

    esp_err_t err = spi_device_polling_transmit( radio_spi, &t );

    // Put NSS high as the spi transaction is finished
    gpio_set_level( RADIO_NSS, 1 );

    if( err != ESP_OK )
    {
        return LR20XX_HAL_STATUS_ERROR;
    }

    return LR20XX_HAL_STATUS_OK;
}

lr20xx_hal_status_t IRAM_ATTR lr20xx_hal_direct_read_fifo( const void* radio, const uint8_t* command,
                                                 const uint16_t command_length, uint8_t* data,
                                                 const uint16_t data_length )
{
    /* Build TX buffer: command + dummy bytes (0x00) */
    uint16_t total_len = command_length + data_length;
    uint8_t tx_buf[total_len];
    uint8_t rx_buf[total_len];

    memcpy( tx_buf, command, command_length );
    memset( tx_buf + command_length, 0x00, data_length );

    spi_transaction_t t = {
        .flags = 0,
        .length    = total_len * 8,     /* Total number of clock cycles */
        .tx_buffer = tx_buf,
        .rx_buffer = rx_buf,
    };

    lr20xx_hal_check_device_ready( );

    // Put NSS low to start spi transaction
    gpio_set_level( RADIO_NSS, 0 );

    esp_err_t err = spi_device_polling_transmit( radio_spi, &t );

    // Put NSS high as the spi transaction is finished
    gpio_set_level( RADIO_NSS, 1 );

    memcpy(data, &rx_buf[2], data_length);

    if( err != ESP_OK )
    {
        return LR20XX_HAL_STATUS_ERROR;
    }

    return LR20XX_HAL_STATUS_OK;
}

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DEFINITION --------------------------------------------
 */

static void IRAM_ATTR lr20xx_hal_wait_on_busy( void )
{
    while (gpio_get_level(RADIO_BUSY) == 1)
    {
        // vTaskDelay(pdMS_TO_TICKS(1));      /* TODO: later this can be replaced with interrupt + event notification */
    }
}

static void IRAM_ATTR lr20xx_hal_check_device_ready( void )
{
    if( radio_mode != RADIO_SLEEP )
    {
        lr20xx_hal_wait_on_busy( );
    }
    else
    {
        // Busy is HIGH in sleep mode, wake-up the device with a small glitch on NSS
        gpio_set_level( RADIO_NSS, 0 );
        // wait for 1ms
        vTaskDelay(pdMS_TO_TICKS(1));
        gpio_set_level( RADIO_NSS, 1 );
        lr20xx_hal_wait_on_busy( );
        radio_mode = RADIO_AWAKE;
    }
}

static void radio_gpio_init(void)
{
    gpio_config_t io_conf = { 0 };

    /* ---------- NRST ---------- */
    io_conf.pin_bit_mask = 1ULL << RADIO_NRST;
    io_conf.mode         = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en   = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type    = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);

    /* Default high level to avoid unintended reset during power-up */
    gpio_set_level(RADIO_NRST, 1);

    /* ---------- NSS (CS) ---------- */
    io_conf.pin_bit_mask = 1ULL << RADIO_NSS;
    io_conf.mode         = GPIO_MODE_OUTPUT;
    gpio_config(&io_conf);

    /* Default high level for SPI idle state */
    gpio_set_level(RADIO_NSS, 1);

    /* ---------- BUSY ---------- */
    io_conf.pin_bit_mask = 1ULL << RADIO_BUSY;
    io_conf.mode         = GPIO_MODE_INPUT;
    io_conf.pull_up_en   = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type    = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);
}

static bool sRadioSpiBusInited = false;
static bool sRadioSpiDevAdded = false;
static void radio_spi_bus_init(void)
{
    if (sRadioSpiBusInited == true) {
        return;
    }

    spi_bus_config_t buscfg = {
        .mosi_io_num     = PIN_NUM_MOSI,
        .miso_io_num     = PIN_NUM_MISO,
        .sclk_io_num     = PIN_NUM_CLK,
        .quadwp_io_num   = -1,
        .quadhd_io_num   = -1,
        .max_transfer_sz = 1024,
    };

    esp_err_t ret = spi_bus_initialize(RADIO_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (ret == ESP_OK)
    {
        sRadioSpiBusInited = true;
    }
}

static void radio_spi_device_init(void)
{
    if (sRadioSpiDevAdded == true)
    {
        return;
    }

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 16 * 1000 * 1000, /* Start with 8 MHz if signal integrity needs tuning */
        .mode           = 0,              /* SPI Mode 0 */
        .spics_io_num   = -1,             /* CS is controlled by the application layer */
        .queue_size     = 1,
        .flags          = 0,    /* Half-duplex SPI_DEVICE_HALFDUPLEX */
    };

    esp_err_t ret = spi_bus_add_device(RADIO_SPI_HOST, &devcfg, &radio_spi);
    if (ret == ESP_OK) {
        sRadioSpiDevAdded = true;
    }
}


/* --- EOF ------------------------------------------------------------------ */
