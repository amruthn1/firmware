#include "common/phal_F4_F7/gpio/gpio.h"
#include "f4_testing.h"

// Guard so cmake doesn't compile all tests
#if (F4_TESTING_CHOSEN == TEST_MMC)

#include "../phal_F4_F7/spi/spi.h"
#include "common/freertos/freertos.h"
#include "common/log/log.h"
#include "common/phal/gpio.h"
#include "common/phal/rcc.h"
#include "common/phal/usart.h"

GPIOInitConfig_t gpio_config[] = {
    GPIO_INIT_OUTPUT(GPIOD, 12, GPIO_OUTPUT_LOW_SPEED), // F407VGT disco LEDs
    GPIO_INIT_OUTPUT(GPIOD, 13, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(GPIOD, 14, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(GPIOD, 15, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_USART2TX_PA2,
    GPIO_INIT_USART2RX_PA3,
    GPIO_INIT_SPI1_SCK_PA5, // SCK
    GPIO_INIT_SPI1_MISO_PA6, // MISO
    GPIO_INIT_SPI1_MOSI_PA7, // MOSI
};

extern uint32_t APB1ClockRateHz;
extern uint32_t APB2ClockRateHz;
extern uint32_t AHBClockRateHz;
extern uint32_t PLLClockRateHz;

#define TargetCoreClockrateHz 16000000
ClockRateConfig_t clock_config = {
    .clock_source = CLOCK_SOURCE_HSI,
    .use_pll = false,
    .vco_output_rate_target_hz = 160000000,
    .system_clock_target_hz = TargetCoreClockrateHz,
    .ahb_clock_target_hz = (TargetCoreClockrateHz / 1),
    .apb1_clock_target_hz = (TargetCoreClockrateHz / (1)),
    .apb2_clock_target_hz = (TargetCoreClockrateHz / (1)),
};

static uint8_t spi_rx_buf[6];
dma_init_t usart_tx_dma_config = USART2_TXDMA_CONT_CONFIG(NULL, 1);
dma_init_t usart_rx_dma_config = USART2_RXDMA_CONT_CONFIG(NULL, 2);
dma_init_t spi1_rx_dma_cfg = SPI1_RXDMA_CONT_CONFIG(spi_rx_buf, 1);
dma_init_t spi1_tx_dma_cfg = SPI1_TXDMA_CONT_CONFIG(NULL, 1);

usart_init_t usart_config = {
    .baud_rate = 115200,
    .word_length = WORD_8,
    .stop_bits = SB_ONE,
    .parity = PT_NONE,
    .hw_flow_ctl = HW_DISABLE,
    .ovsample = OV_16,
    .obsample = OB_DISABLE,
    .periph = USART2,
    .wake_addr = false,
    .usart_active_num = USART2_ACTIVE_IDX,
    .tx_dma_cfg = &usart_tx_dma_config,
    .rx_dma_cfg = &usart_rx_dma_config};

DEBUG_PRINTF_USART_DEFINE(&usart_config)

SPI_InitConfig_t spi1_cfg = {
    .data_rate = 400000,
    .data_len = 8,
    .nss_sw = true,
    .nss_gpio_port = GPIOA,
    .nss_gpio_pin = 4,
    .rx_dma_cfg = &spi1_rx_dma_cfg,
    .tx_dma_cfg = &spi1_tx_dma_cfg,
    .periph = SPI1,
};

void HardFault_Handler();
void ledblink1();
void ledblink2();
void ledblink3();
void ledblink4();
void usartSend();
void mmcTest();

// Define up here so they're global
defineThreadStack(ledblink1, 250, osPriorityNormal, 64);
defineThreadStack(ledblink2, 300, osPriorityNormal, 64);
defineThreadStack(ledblink3, 500, osPriorityNormal, 64);
defineThreadStack(ledblink4, 1000, osPriorityNormal, 64);
defineThreadStack(usartSend, 1000, osPriorityNormal, 1024);
defineThreadStack(mmcTest, 100, osPriorityNormal, 1024);

defineStaticQueue(myQueue, uint32_t, 0x45);
defineStaticSemaphore(mySemaphore);

uint16_t axis_data;

int main() {
    osKernelInitialize();

    // Initialize hardware
    if (0 != PHAL_configureClockRates(&clock_config)) {
        HardFault_Handler();
    }
    if (!PHAL_initGPIO(gpio_config, sizeof(gpio_config) / sizeof(GPIOInitConfig_t))) {
        HardFault_Handler();
    }
    if (!PHAL_initUSART(&usart_config, APB1ClockRateHz)) {
        HardFault_Handler();
    }
    log_yellow("PER PER PER\n");

    if (!PHAL_SPI_init(&spi1_cfg)) {
        log_red("SPI1 init failed!\n");
        HardFault_Handler();
    }

    // Create threads
    createThread(ledblink1);
    createThread(ledblink2);
    createThread(ledblink3);
    createThread(ledblink4);
    createThread(usartSend);
    createThread(mmcTest);

    // Create objects
    myQueue = createStaticQueue(myQueue, uint32_t, 0x45);
    mySemaphore = createStaticSemaphore(mySemaphore);

    osKernelStart(); // Go!

    return 0;
}

void ledblink1() {
    PHAL_toggleGPIO(GPIOD, 12);
}

void ledblink2() {
    PHAL_toggleGPIO(GPIOD, 13);
}

void ledblink3() {
    PHAL_toggleGPIO(GPIOD, 14);
}

void ledblink4() {
    PHAL_toggleGPIO(GPIOD, 15);
}

void mmcTest() {
    uint8_t tx_buf[3];
    for (int axis = 0; axis < 3; axis++) {
        uint8_t reg_addr = axis * 2; // 0x00, 0x02, 0x04
        tx_buf[0] = 0x80 | reg_addr; // Read command: MSB=1 + address
        tx_buf[1] = 0x00; // dummy byte 1
        tx_buf[2] = 0x00; // dummy byte 2

        PHAL_writeGPIO(GPIOA, 4, false); // CS low

        if (!PHAL_SPI_transfer_noDMA(&spi1_cfg, tx_buf, sizeof(tx_buf), sizeof(spi_rx_buf), spi_rx_buf)) {
            PHAL_writeGPIO(GPIOA, 4, true); // CS high
        }

        PHAL_writeGPIO(GPIOA, 4, true); // CS high

        axis_data = ((uint16_t)spi_rx_buf[1] << 8) | spi_rx_buf[2];
    }
}

void usartSend() {
    if (xSemaphoreTake(mySemaphore, (TickType_t)10) == pdTRUE) {
        /* We were able to obtain the semaphore and can now access the
            shared resource. */

        debug_printf("tick: %d\n", getTick());

        /* We have finished accessing the shared resource. Release the
           semaphore. */
        xSemaphoreGive(mySemaphore);
    }
}

void HardFault_Handler() {
    while (1) {
        __asm__("nop");
    }
}

#endif