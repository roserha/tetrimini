#include "graphics.h"
#include <zephyr/drivers/display.h>

// Framebuffer. Each pixel is monochrome, so 8 consecutive pixels are bundled per byte
static uint8_t fb[GFX_W * GFX_H / 8];

// Zephyr-driven display device
static const struct device *disp = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));

// Display Buffer Descriptor
static struct display_buffer_descriptor desc = {
    .buf_size = sizeof(fb),
    .width    = GFX_W,
    .height   = GFX_H,
    .pitch    = GFX_W,
};

// Initializes graphics information and frame buffer using transposed coordinate system
int  gfx_init(void)
{
    if (!device_is_ready(disp)) {
        return -1;
    }

    gfx_clear();
    return 0;
}

// Clears the buffer
void gfx_clear(void)
{
    memset(fb, 0, sizeof(fb)); 
}

// Specifices state of individual pixel using transposed coordinate system
// x:  x coordinate
// y:  y coordinate
// on: pixel color (black or white)
void gfx_px(uint8_t x, uint8_t y, bool on)
{
    // Framebuffer y coordinate's direction inside pages is reversed.
    // As such, we need to flip the last 8 bits to properly target the bit
    uint8_t y_adj = y ^ 0b111;
    if (x >= GFX_W || y_adj >= GFX_H) return;

    size_t idx  = (y_adj / 8) * GFX_W + x;   /* which page, then which column */
    uint8_t bit = BIT(y_adj % 8);           /* which row inside the page     */

    if (on) {
        fb[idx] |= bit;
    } else {
        fb[idx] &= ~bit;
    }
}

// Draws a w-x-h unfilled rect at x and y of color on using transposed coordinate system
// x:  x coordinate
// y:  y coordinate
// w: width
// h: height
// on: pixel color (black or white)
void gfx_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, bool on)
{
    uint8_t xpw = x + w;
    uint8_t xpwm1 = x + w - 1;
    uint8_t yphm1 = y + h - 1;

    // Horizontal lines
    for (uint8_t i = x; i < xpw; i++)
    {
        gfx_px(i, y, on);
        gfx_px(i, yphm1, on);
    }

    // Vertical lines
    for (uint8_t j = y + 1; j < yphm1; j++)
    {
        gfx_px(x, j, on);
        gfx_px(xpwm1, j, on);
    }
}

// Draws a w-x-h filled rect at x and y of color on using transposed coordinate system
// x:  x coordinate
// y:  y coordinate
// w: width
// h: height
// on: pixel color (black or white)
void gfx_fill(uint8_t x, uint8_t y, uint8_t w, uint8_t h, bool on)
{
    uint8_t xpw = x + w;
    uint8_t yph = y + h;

    for (uint8_t i = x; i < xpw; i++)
    {
        for (uint8_t j = y; j < yph; j++)
        {
            gfx_px(i,j,on);
        }
    }
}

// Flushes display
int  gfx_flush(void)
{
    display_write(disp, 0, 0, &desc, fb);
    display_blanking_off(disp);

    return 0;
}

// Draws playfield bounds
void gfx_playfield()
{
    gfx_rect(0, 0, 112, 64, true);
}

// Draws 6px-wide block using tetrimino coordinate system
// x:  x coordinate
// y:  y coordinate
// block_type: which block to draw based on block enum ids
void gfx_block(uint8_t x, uint8_t y, uint8_t block_type)
{
    // TODO: Change these to a table format to save on time??
    if (x >= 10 || y >= 18) { return; }

    uint8_t x_raw = y * 6 + 2;
    uint8_t y_raw = x * 6 + 2;

    switch (block_type)
    {
        case 0: // T-Block
            gfx_rect(x_raw, y_raw, 6, 6, true);
            gfx_rect(x_raw + 1, y_raw + 1, 4, 4, false);
            gfx_fill(x_raw + 2, y_raw + 2, 2, 2, true);
            break;

        case 1: // O Block
            gfx_rect(x_raw, y_raw, 6, 6, true);
            for (int j = 0; j < 4; j++) 
            {
                for (int i = 0; i < 4; i++)
                {
                    bool px_on = (i + j) & 1;
                    gfx_px(x_raw + 1 + i, y_raw + 1 + j, px_on);
                }
            }
            break;

        case 2: // I Block
            gfx_rect(x_raw, y_raw, 6, 6, true);
            gfx_fill(x_raw + 1, y_raw + 1, 4, 4, false);
            break;

        case 3: // S-Block
            gfx_rect(x_raw, y_raw, 6, 6, true);
            gfx_fill(x_raw + 1, y_raw + 1, 4, 4, true);
            for (int i = 0; i < 4; i++)
            {
                gfx_px(x_raw + 4 - i, y_raw + 1 + i, false);
                gfx_px(x_raw + 1 + i, y_raw + 1 + i, false);
            }
            break;

        case 4: // Z-Block
            gfx_rect(x_raw, y_raw, 6, 6, true);
            gfx_fill(x_raw + 1, y_raw + 1, 4, 4, false);
            for (int i = 0; i < 4; i++)
            {
                gfx_px(x_raw + 4 - i, y_raw + 1 + i, true);
                gfx_px(x_raw + 1 + i, y_raw + 1 + i, true);
            }
            break;

        case 5: // J-Block
            gfx_rect(x_raw, y_raw, 6, 6, true);
            gfx_fill(x_raw + 1, y_raw + 1, 4, 4, false);
            for (int i = 0; i < 4; i++)
            {
                gfx_px(x_raw + 4 - i, y_raw + 1 + i, true);
            }
            break;

        case 6:
        default: // L-Block
            gfx_rect(x_raw, y_raw, 6, 6, true);
            gfx_fill(x_raw + 1, y_raw + 1, 4, 4, false);
            for (int i = 0; i < 4; i++)
            {
                gfx_px(x_raw + 1 + i, y_raw + 1 + i, true);
            }
            break;
    }
}