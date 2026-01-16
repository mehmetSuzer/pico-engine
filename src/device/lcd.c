
#include "lcd.h"
#include "swapchain/swapchain.h"

#include <pico/stdlib.h>
#include <hardware/spi.h>
#include <hardware/dma.h>
#include <hardware/pwm.h>

#if defined(CLOCK_FREQUENCY_KHZ)
    #define SPI_BAUDRATE_HZ (CLOCK_FREQUENCY_KHZ * 250)
#else
    #define SPI_BAUDRATE_HZ 62500000
#endif

#define SPI_PORT spi1

#define LCD_RST_PIN    12u
#define LCD_DC_PIN      8u
#define LCD_BL_PIN     13u
#define LCD_CS_PIN      9u
#define LCD_CLK_PIN    10u
#define LCD_MOSI_PIN   11u
#define LCD_SCL_PIN     7u
#define LCD_SDA_PIN     6u

static int lcd_dma_chan = -1;
static dma_channel_config lcd_dma_cfg;

static void lcd_gpio_set(uint pin, bool mode) 
{
    gpio_init(pin);
    gpio_set_dir(pin, mode);
}

static void lcd_set_pins()
{
    lcd_gpio_set(LCD_RST_PIN, GPIO_OUT);
    lcd_gpio_set(LCD_DC_PIN,  GPIO_OUT);
    lcd_gpio_set(LCD_CS_PIN,  GPIO_OUT);
    lcd_gpio_set(LCD_BL_PIN,  GPIO_OUT);
    lcd_gpio_set(LCD_CS_PIN,  GPIO_OUT);
    lcd_gpio_set(LCD_BL_PIN,  GPIO_OUT);

    gpio_put(LCD_CS_PIN, true);
    gpio_put(LCD_DC_PIN, false);
    gpio_put(LCD_BL_PIN, true);
}

static void lcd_reset() 
{
    gpio_put(LCD_RST_PIN, true);
    sleep_ms(100);
    gpio_put(LCD_RST_PIN, false);
    sleep_ms(100);
    gpio_put(LCD_RST_PIN, true);
    sleep_ms(100);
}

static void lcd_set_spi() 
{
    spi_init(SPI_PORT, SPI_BAUDRATE_HZ);
    // (SPI_CPOL_1, SPI_CPHA_1), where 0.5 clock cycles is wasted, 
    // is faster than (SPI_CPOL_0, SPI_CPHA_0), where 1.5 clock cycles is wasted.
    // LCD expects 8-bit commands and data. Therefore, we initalise the SPI in 8-bit mode.
    spi_set_format(SPI_PORT, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);
    gpio_set_function(LCD_CLK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(LCD_MOSI_PIN, GPIO_FUNC_SPI);
}

static inline void lcd_command_mode()
{
    gpio_put(LCD_DC_PIN, false);
    gpio_put(LCD_CS_PIN, false);
}

static inline void lcd_data_mode()
{
    gpio_put(LCD_DC_PIN, true);
    gpio_put(LCD_CS_PIN, false);
}

static inline void lcd_signal_transfer_finish()
{
    gpio_put(LCD_CS_PIN, true);
}

static void lcd_command(uint8_t command) 
{
    lcd_command_mode();
    spi_write_blocking(SPI_PORT, &command, 1);
    lcd_signal_transfer_finish();
}

static void lcd_write_8bit_data(uint8_t data) 
{
    lcd_data_mode();
    spi_write_blocking(SPI_PORT, &data, 1);
    lcd_signal_transfer_finish();
}

static void lcd_set_pwm() 
{
    gpio_set_function(LCD_BL_PIN, GPIO_FUNC_PWM);
    const uint slice_num = pwm_gpio_to_slice_num(LCD_BL_PIN);
    pwm_set_wrap(slice_num, 100);
    pwm_set_chan_level(slice_num, PWM_CHAN_B, 40); // level / wrap = 40 / 100 = 40 % brightness
    pwm_set_clkdiv(slice_num, 50.0f);
    pwm_set_enabled(slice_num, true);
}

static void lcd_configure()
{
    lcd_command(0x36);
    lcd_write_8bit_data(0X70);
    
    lcd_command(0x3A);
#if defined(RGB332)
    lcd_write_8bit_data(0x02);
#elif defined(RGB565)
    lcd_write_8bit_data(0x05);
#endif

    lcd_command(0xB2);
    lcd_write_8bit_data(0x0C);
    lcd_write_8bit_data(0x0C);
    lcd_write_8bit_data(0x00);
    lcd_write_8bit_data(0x33);
    lcd_write_8bit_data(0x33);

    lcd_command(0xB7);          // Gate Control
    lcd_write_8bit_data(0x35);

    lcd_command(0xBB);          // VCOM Setting
    lcd_write_8bit_data(0x19);

    lcd_command(0xC0);          // LCM Control     
    lcd_write_8bit_data(0x2C);

    lcd_command(0xC2);          // VDV and VRH Command Enable
    lcd_write_8bit_data(0x01);

    lcd_command(0xC3);          // VRH Set
    lcd_write_8bit_data(0x12);
    
    lcd_command(0xC4);          // VDV Set
    lcd_write_8bit_data(0x20);

    lcd_command(0xC6);          // Frame Rate Control in Normal Mode
    lcd_write_8bit_data(0x0F);
    
    lcd_command(0xD0);          // Power Control 1
    lcd_write_8bit_data(0xA4);
    lcd_write_8bit_data(0xA1);

    lcd_command(0xE0);          // Positive Voltage Gamma Control
    lcd_write_8bit_data(0xD0);
    lcd_write_8bit_data(0x04);
    lcd_write_8bit_data(0x0D);
    lcd_write_8bit_data(0x11);
    lcd_write_8bit_data(0x13);
    lcd_write_8bit_data(0x2B);
    lcd_write_8bit_data(0x3F);
    lcd_write_8bit_data(0x54);
    lcd_write_8bit_data(0x4C);
    lcd_write_8bit_data(0x18);
    lcd_write_8bit_data(0x0D);
    lcd_write_8bit_data(0x0B);
    lcd_write_8bit_data(0x1F);
    lcd_write_8bit_data(0x23);

    lcd_command(0xE1);           // Negative Voltage Gamma Control
    lcd_write_8bit_data(0xD0);
    lcd_write_8bit_data(0x04);
    lcd_write_8bit_data(0x0C);
    lcd_write_8bit_data(0x11);
    lcd_write_8bit_data(0x13);
    lcd_write_8bit_data(0x2C);
    lcd_write_8bit_data(0x3F);
    lcd_write_8bit_data(0x44);
    lcd_write_8bit_data(0x51);
    lcd_write_8bit_data(0x2F);
    lcd_write_8bit_data(0x1F);
    lcd_write_8bit_data(0x1F);
    lcd_write_8bit_data(0x20);
    lcd_write_8bit_data(0x23);

    lcd_command(0x21);          // Display Inversion On
    lcd_command(0x11);          // Sleep Out
    sleep_ms(120);
    lcd_command(0x29);          // Display On
}

static void lcd_set_window(uint8_t start_x, uint8_t end_x, uint8_t start_y, uint8_t end_y) 
{
    lcd_command(0x2A);
    lcd_write_8bit_data(0x00);
    lcd_write_8bit_data(start_x);
	lcd_write_8bit_data(0x00);
    lcd_write_8bit_data(end_x - 1);

    lcd_command(0x2B);
    lcd_write_8bit_data(0x00);
	lcd_write_8bit_data(start_y);
	lcd_write_8bit_data(0x00);
    lcd_write_8bit_data(end_y - 1);

    lcd_command(0x2C);
}

void lcd_start_transfer() 
{
    lcd_set_window(0, SCREEN_WIDTH, 0, SCREEN_HEIGHT);
    lcd_data_mode();
    
#if defined(RGB565)
    // Switch to 16-bit mode for pixel data transfer
    spi_set_format(SPI_PORT, 16, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);
#endif

    const swapchain_image_t* display_image = swapchain_request_display_image();
    dma_channel_set_read_addr(lcd_dma_chan, (const colour_t*)display_image->colours, false);
    dma_channel_set_transfer_count(lcd_dma_chan, SCREEN_WIDTH * SCREEN_HEIGHT, true);
}

static void __isr lcd_dma_irq_handler()
{
    // Clear IRQ
    dma_hw->ints0 = 1u << lcd_dma_chan;

    // Wait until SPI shifts last bit
    while (spi_is_busy(SPI_PORT)) { }

    lcd_signal_transfer_finish();

#if defined(RGB565)
    // switch to 8-bit mode for command transfer
    spi_set_format(SPI_PORT, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);
#endif
    lcd_command(0x29);    
    lcd_start_transfer();
}

static void lcd_set_dma()
{
    lcd_dma_chan = dma_claim_unused_channel(true);
    lcd_dma_cfg = dma_channel_get_default_config(lcd_dma_chan);

#if defined(RGB332)
    channel_config_set_transfer_data_size(&lcd_dma_cfg, DMA_SIZE_8);
#elif defined(RGB565)
    channel_config_set_transfer_data_size(&lcd_dma_cfg, DMA_SIZE_16);
#endif

    channel_config_set_read_increment(&lcd_dma_cfg, true);
    channel_config_set_write_increment(&lcd_dma_cfg, false);
    channel_config_set_dreq(&lcd_dma_cfg, (SPI_PORT == spi0) ? DREQ_SPI0_TX : DREQ_SPI1_TX);

    dma_channel_configure(
        lcd_dma_chan,
        &lcd_dma_cfg,
        &spi_get_hw(SPI_PORT)->dr,
        NULL,                       // Set the read address later
        0,                          // Set the transfer count later
        false                       // Do not start immediately
    );

    // Enable IRQ on DMA completion
    dma_channel_set_irq0_enabled(lcd_dma_chan, true);
    irq_set_exclusive_handler(DMA_IRQ_0, lcd_dma_irq_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    lcd_start_transfer();
}

void lcd_init() 
{
    lcd_set_pins();
    lcd_reset();
    lcd_set_spi();
    lcd_set_pwm();
    lcd_configure();
    lcd_set_dma();
}

