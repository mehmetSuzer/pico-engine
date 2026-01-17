
#include "lcd.h"
#include "swapchain/swapchain.h"

#include <pico/stdlib.h>
#include <hardware/pwm.h>
#include <hardware/spi.h>
#include <hardware/dma.h>

#if defined(CLOCK_FREQUENCY_KHZ)
    #define SPI_BAUDRATE_HZ (CLOCK_FREQUENCY_KHZ * 250u)
#elif defined(PICO_RP2350)
    #define SPI_BAUDRATE_HZ 75000000u
#elif defined(PICO_RP2040)
    #define SPI_BAUDRATE_HZ 62500000u
#endif

#define SPI_PORT spi1

#define LCD_RST_PIN      12u
#define LCD_DC_PIN        8u
#define LCD_BL_PIN       13u
#define LCD_CS_PIN        9u
#define LCD_CLK_PIN      10u
#define LCD_MOSI_PIN     11u

#define LCD_BL_MAX      100u
#define LCD_BL_DEFAULT   40u

// MADCTL (Memory Access Control) bits
#define MADCTL_MY  0x80u // Row address order
#define MADCTL_MX  0x40u // Column address order
#define MADCTL_MV  0x20u // Row/Column exchange
#define MADCTL_ML  0x10u // Vertical refresh order
#define MADCTL_RGB 0x00u // RGB order
#define MADCTL_BGR 0x08u // BGR order
#define MADCTL_MH  0x04u // Horizontal refresh order

#define LCD_CMD_MADCTL      0x36u
#define LCD_CMD_COLMOD      0x3Au
#define LCD_CMD_PORCTRL     0xB2u
#define LCD_CMD_GCTRL       0xB7u
#define LCD_CMD_VCOMS       0xBBu
#define LCD_CMD_LCMCTRL     0xC0u
#define LCD_CMD_VDVVRHEN    0xC2u
#define LCD_CMD_VRHSET      0xC3u
#define LCD_CMD_VDVSET      0xC4u
#define LCD_CMD_FRCTRL      0xC6u
#define LCD_CMD_PWCTRL1     0xD0u
#define LCD_CMD_PVGAMCTRL   0xE0u
#define LCD_CMD_NVGAMCTRL   0xE1u
#define LCD_CMD_INVON       0x21u
#define LCD_CMD_SLPOUT      0x11u
#define LCD_CMD_DISPON      0x29u
#define LCD_CMD_NORON		0x13u

#define LCD_GATE_CTRL       0x35u
#define LCD_VCOM            0x19u
#define LCD_LCM_CTRL        0x2Cu
#define LCD_VDV_VRH_EN      0x01u
#define LCD_VRH             0x12u
#define LCD_VDV             0x20u
#define LCD_FRATE           0x0Fu

#define LCD_COLOUR_RGB332   0x02u
#define LCD_COLOUR_RGB565   0x05u

#define LCD_CMD_CASET       0x2Au // Column Address Set
#define LCD_CMD_RASET       0x2Bu // Row Address Set
#define LCD_CMD_RAMWR       0x2Cu // Memory Write

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

static void lcd_set_pwm() 
{
    const uint slice_num = pwm_gpio_to_slice_num(LCD_BL_PIN);
    const uint channel = pwm_gpio_to_channel(LCD_BL_PIN);

    gpio_set_function(LCD_BL_PIN, GPIO_FUNC_PWM);
    pwm_set_wrap(slice_num, LCD_BL_MAX);
    pwm_set_chan_level(slice_num, channel, LCD_BL_DEFAULT); // brightness = level / wrap
    pwm_set_clkdiv(slice_num, 50.0f);
    pwm_set_enabled(slice_num, true);
}

static inline void lcd_spi_set_format(uint data_bits)
{
    // (SPI_CPOL_1, SPI_CPHA_1), where 0.5 clock cycles is wasted, 
    // is faster than (SPI_CPOL_0, SPI_CPHA_0), where 1.5 clock cycles is wasted.
    spi_set_format(SPI_PORT, data_bits, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);
}

static void lcd_set_spi() 
{
    spi_init(SPI_PORT, SPI_BAUDRATE_HZ);
    lcd_spi_set_format(8); // The LCD expects 8-bit commands and data
    gpio_set_function(LCD_CLK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(LCD_MOSI_PIN, GPIO_FUNC_SPI);
}

static inline void lcd_select()
{
    gpio_put(LCD_CS_PIN, false);
}

static inline void lcd_unselect()
{
    gpio_put(LCD_CS_PIN, true);
}

static inline void lcd_command_mode()
{
    gpio_put(LCD_DC_PIN, false);
}

static inline void lcd_data_mode()
{
    gpio_put(LCD_DC_PIN, true);
}

static void lcd_command(uint8_t command) 
{
    lcd_command_mode();
    lcd_select();
    spi_write_blocking(SPI_PORT, &command, 1);
    lcd_unselect();
}

static void lcd_data_8bit(uint8_t data) 
{
    lcd_data_mode();
    lcd_select();
    spi_write_blocking(SPI_PORT, &data, 1);
    lcd_unselect();
}

static void lcd_data_8bit_n(const uint8_t* data, size_t len)
{
    lcd_data_mode();
    lcd_select();
    spi_write_blocking(SPI_PORT, data, len);
    lcd_unselect();
}

static void lcd_configure()
{
    static const uint8_t LCD_PORCH[]     = {0x0Cu, 0x0Cu, 0x00u, 0x33u, 0x33u};
    static const uint8_t LCD_PWRCTRL1[]  = {0xA4u, 0xA1u};
    static const uint8_t LCD_GAMMA_POS[] = {0xD0u, 0x04u, 0x0Du, 0x11u, 0x13u, 0x2Bu, 0x3Fu,
                                            0x54u, 0x4Cu, 0x18u, 0x0Du, 0x0Bu, 0x1Fu, 0x23u};
    static const uint8_t LCD_GAMMA_NEG[] = {0xD0u, 0x04u, 0x0Cu, 0x11u, 0x13u, 0x2Cu, 0x3Fu,
                                            0x44u, 0x51u, 0x2Fu, 0x1Fu, 0x1Fu, 0x20u, 0x23u};

    lcd_command(LCD_CMD_SLPOUT);
    sleep_ms(120);
    lcd_command(LCD_CMD_NORON);

    lcd_command(LCD_CMD_MADCTL);
    lcd_data_8bit(MADCTL_MX | MADCTL_MV | MADCTL_ML | MADCTL_RGB);
    
    lcd_command(LCD_CMD_COLMOD);
#if defined(RGB332)
    lcd_data_8bit(LCD_COLOUR_RGB332);
#elif defined(RGB565)
    lcd_data_8bit(LCD_COLOUR_RGB565);
#endif

    lcd_command(LCD_CMD_PORCTRL);
    lcd_data_8bit_n(LCD_PORCH, count_of(LCD_PORCH));

    lcd_command(LCD_CMD_GCTRL);
    lcd_data_8bit(LCD_GATE_CTRL);

    lcd_command(LCD_CMD_VCOMS);
    lcd_data_8bit(LCD_VCOM);

    lcd_command(LCD_CMD_LCMCTRL);
    lcd_data_8bit(LCD_LCM_CTRL);

    lcd_command(LCD_CMD_VDVVRHEN);
    lcd_data_8bit(LCD_VDV_VRH_EN);

    lcd_command(LCD_CMD_VRHSET);
    lcd_data_8bit(LCD_VRH);
    
    lcd_command(LCD_CMD_VDVSET);
    lcd_data_8bit(LCD_VDV);

    lcd_command(LCD_CMD_FRCTRL);
    lcd_data_8bit(LCD_FRATE);
    
    lcd_command(LCD_CMD_PWCTRL1);
    lcd_data_8bit_n(LCD_PWRCTRL1, count_of(LCD_PWRCTRL1));

    lcd_command(LCD_CMD_PVGAMCTRL);
    lcd_data_8bit_n(LCD_GAMMA_POS, count_of(LCD_GAMMA_POS));

    lcd_command(LCD_CMD_NVGAMCTRL);
    lcd_data_8bit_n(LCD_GAMMA_NEG, count_of(LCD_GAMMA_NEG));

    lcd_command(LCD_CMD_INVON);

    lcd_command(LCD_CMD_SLPOUT);
    sleep_ms(120);

    lcd_command(LCD_CMD_DISPON);
}

static void lcd_set_window(uint8_t start_x, uint8_t end_x, uint8_t start_y, uint8_t end_y) 
{
    // Set column address (X)
    lcd_command(LCD_CMD_CASET);
    lcd_data_8bit(0x00u);       // High byte of start column
    lcd_data_8bit(start_x);     // Low byte of start column
    lcd_data_8bit(0x00u);       // High byte of end column
    lcd_data_8bit(end_x - 1);   // Low byte of end column (inclusive)

    // Set row address (Y)
    lcd_command(LCD_CMD_RASET);
    lcd_data_8bit(0x00u);       // High byte of start row
    lcd_data_8bit(start_y);     // Low byte of start row
    lcd_data_8bit(0x00u);       // High byte of end row
    lcd_data_8bit(end_y - 1);   // Low byte of end row (inclusive)

    // Prepare to write memory
    lcd_command(LCD_CMD_RAMWR);
}

void lcd_start_transfer() 
{
    lcd_set_window(0, SCREEN_WIDTH, 0, SCREEN_HEIGHT);
    lcd_data_mode();
    lcd_select();
    
#if defined(RGB565)
    lcd_spi_set_format(16); // Switch to 16-bit mode for pixel data transfer
#endif

    const swapchain_image_t* display_image = swapchain_request_display_image();
    dma_channel_set_read_addr(lcd_dma_chan, (const colour_t*)display_image->colours, false);
    dma_channel_set_transfer_count(lcd_dma_chan, SCREEN_WIDTH * SCREEN_HEIGHT, true);
}

static void __isr lcd_dma_irq_handler()
{
    // Clear IRQ
    dma_hw->ints0 = 1u << lcd_dma_chan;

    // Wait until SPI shifts the last bit
    while (spi_is_busy(SPI_PORT)) { }

    // Signal the end of the pixel data transfer
    lcd_unselect();

#if defined(RGB565)
    lcd_spi_set_format(8); // Switch to 8-bit mode for command transfer
#endif

    // Display on the screen
    lcd_command(LCD_CMD_DISPON);

    // Start pixel data transfer again
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
    lcd_set_pwm();
    lcd_set_spi();
    lcd_configure();
    lcd_set_dma();
}

