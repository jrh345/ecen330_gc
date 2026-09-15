#include <stdio.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "lcd.h"
#include "pac.h"

static const char *TAG = "lab01";

#define DELAY_MS(ms) \
	vTaskDelay(((ms)+(portTICK_PERIOD_MS-1))/portTICK_PERIOD_MS)

//----------------------------------------------------------------------------//
// Car Implementation - Begin
//----------------------------------------------------------------------------//

// Car constants
#define CAR_CLR rgb565(220,30,0)
#define WINDOW_CLR rgb565(180,210,238)
#define TIRE_CLR BLACK
#define HUB_CLR GRAY

// TODO: Finish car part constants
#define CAR_W 60
#define CAR_H 32

#define BODY_X0 0
#define BODY_Y0 12
#define BODY_X1 59
#define BODY_Y1 24

#define TOP_X0 1
#define TOP_Y0 0
#define TOP_X1 39
#define TOP_Y1 11

#define WINB_X0 3
#define WINB_Y0 1
#define WINB_X1 18
#define WINB_Y1 8

#define WINR 	2

#define WINF_X0 21
#define WINF_Y0 1
#define WINF_X1 37
#define WINF_Y1 8

#define TIREB_X 11
#define TIREB_Y 24

#define TIRER 	7

#define TIRERF_X 48
#define TIRERF_Y 24

#define HUBB_X 11
#define HUBB_Y 24

#define HUBR 4

#define HUBF_X 48
#define HUBF_Y 24

#define HOOD_X0 40
#define HOOD_X1 59
#define HOOD_Y0 9
#define HOOD_Y1 11
/**
 * @brief Draw a car at the specified location.
 * @param x      Top left corner X coordinate.
 * @param y      Top left corner Y coordinate.
 * @details Draw the car components relative to the anchor point (top, left).
 */
void drawCar(coord_t x, coord_t y)
{
	// TODO: Implement car procedurally with lcd geometric primitives.
	lcd_fillRect2(TOP_X0 + x, TOP_Y0 + y, TOP_X1 + x, TOP_Y1 + y, CAR_CLR); // Car top body
	lcd_fillRect2(BODY_X0 + x, BODY_Y0 + y, BODY_X1 + x, BODY_Y1 + y, CAR_CLR); //Car body
	lcd_fillRoundRect2(WINB_X0 + x, WINB_Y0 + y, WINB_X1 + x, WINB_Y1 + y, WINR, WINDOW_CLR); //Back window
	lcd_fillRoundRect2(WINF_X0 + x, WINF_Y0 + y, WINF_X1 + x, WINF_Y1 + y, WINR, WINDOW_CLR); //Front window
	lcd_fillCircle(TIREB_X + x, TIREB_Y + y, TIRER, TIRE_CLR); //back tire tread
	lcd_fillCircle(HUBB_X + x, HUBB_Y + y, HUBR, HUB_CLR); //back hub cap
	lcd_fillCircle(TIRERF_X + x, TIRERF_Y + y, TIRER, TIRE_CLR); //front tire tread
	lcd_fillCircle(HUBF_X + x, HUBF_Y + y, HUBR, HUB_CLR); //front hub cap
	lcd_fillTriangle(HOOD_X0 + x, HOOD_Y0 + y, HOOD_X0 + x, HOOD_Y1 + y, HOOD_X1 + x, HOOD_Y1 + y, CAR_CLR); // Car hood
	

}

//----------------------------------------------------------------------------//
// Car Implementation - End
//----------------------------------------------------------------------------//

// Main display constants
#define BACKGROUND_CLR rgb565(0,60,90)
#define TITLE_CLR GREEN
#define STATUS_CLR WHITE
#define STR_BUF_LEN 12 // string buffer length
#define FONT_SIZE 2
#define FONT_W (LCD_CHAR_W*FONT_SIZE)
#define FONT_H (LCD_CHAR_H*FONT_SIZE)
#define STATUS_W (FONT_W*3)

#define WAIT 2000 // milliseconds
#define DELAY_EX3 20 // milliseconds

// Object position and movement
#define OBJ_X 100
#define OBJ_Y 100
#define OBJ_MOVE 3 // pixels


// Application main
void app_main(void)
{
	ESP_LOGI(TAG, "Start up");
	lcd_init();
	lcd_fillScreen(BACKGROUND_CLR);
	lcd_setFontSize(FONT_SIZE);
	lcd_drawString(0, 0, "Hello World! (lcd)", TITLE_CLR);
	printf("Hello World! (terminal)\n");
	
	DELAY_MS(WAIT);

	// TODO: Exercise 1 - Draw car in one location.
	lcd_fillScreen(BACKGROUND_CLR);
	lcd_drawString(0,0, "Exercise 1", TITLE_CLR);
	drawCar(OBJ_X, OBJ_Y);
	DELAY_MS(WAIT);

	// TODO: Exercise 2 - Draw moving car (Method 1), one pass across display.
	char str[8];
	for (coord_t x = -CAR_W; x <= LCD_W; x += OBJ_MOVE)
	{
		lcd_fillScreen(BACKGROUND_CLR);
		lcd_drawString(0,0, "Exercise 2", TITLE_CLR);
		drawCar(x, OBJ_Y);
		sprintf(str, "%3ld", x);
		lcd_drawString(0, LCD_H - FONT_H, str , STATUS_CLR);
	}
	
	
	// TODO: Exercise 3 - Draw moving car (Method 2), one pass across display.
	// Move by erasing car at old position, then redrawing at new position.
	// Objects that don't change or move are drawn once.
	lcd_fillScreen(BACKGROUND_CLR);
	lcd_drawString(0,0, "Exercise 3", TITLE_CLR);
	DELAY_MS(WAIT);
	for (coord_t x = -CAR_W; x <= LCD_W; x += OBJ_MOVE)
	{
		lcd_fillRect2(x - OBJ_MOVE,OBJ_Y, x + OBJ_MOVE, OBJ_Y + CAR_H, BACKGROUND_CLR);
		drawCar(x, OBJ_Y);
		sprintf(str, "%3ld", x);
		lcd_fillRect2(0, LCD_H - FONT_H, FONT_W * 3, LCD_H, BACKGROUND_CLR);
		lcd_drawString(0, LCD_H - FONT_H, str , STATUS_CLR);
		DELAY_MS(DELAY_EX3);
	}
	DELAY_MS(WAIT);
	

	// TODO: Exercise 4 - Draw moving car (Method 3), one pass across display.
	// First, draw all objects into a cleared, off-screen frame buffer.
	// Then, transfer the entire frame buffer to the screen.
	lcd_frameEnable();
	lcd_fillScreen(BACKGROUND_CLR);
	lcd_drawString(0,0, "Exercise 4", TITLE_CLR);
	for (coord_t x = -CAR_W; x <= LCD_W; x += OBJ_MOVE)
	{
		lcd_fillScreen(BACKGROUND_CLR);
		lcd_drawString(0,0, "Exercise 4", TITLE_CLR);
		drawCar(x, OBJ_Y);
		sprintf(str, "%3ld", x);
		lcd_fillRect2(0, LCD_H - FONT_H, FONT_W * 3, LCD_H, BACKGROUND_CLR);
		lcd_drawString(0, LCD_H - FONT_H, str , STATUS_CLR);
		lcd_writeFrame();
		DELAY_MS(DELAY_EX3);
	}
	lcd_frameDisable();


	// TODO: Exercise 5 - Draw an animated Pac-Man moving across the display.
	// Use Pac-Man sprites instead of the car object.
	// Cycle through each sprite when moving the Pac-Man character.
	lcd_frameEnable();
	lcd_fillScreen(BACKGROUND_CLR);
	lcd_drawString(0,0, "Exercise 5", TITLE_CLR);
	const uint8_t pacman_order[] = {0, 1, 2, 1}; //order of sprites
	uint8_t frame_index = 0;
	for (coord_t x = -PAC_W; x <= LCD_W; x += OBJ_MOVE)
	{
		lcd_fillScreen(BACKGROUND_CLR);
		lcd_drawString(0,0, "Exercise 5", TITLE_CLR);
		lcd_drawBitmap(x, OBJ_Y, pac[pacman_order[frame_index++ % 4]], PAC_W, PAC_H, YELLOW);
		sprintf(str, "%3ld", x);
		lcd_fillRect2(0, LCD_H - FONT_H, FONT_W * 3, LCD_H, BACKGROUND_CLR);
		lcd_drawString(0, LCD_H - FONT_H, str , STATUS_CLR);
		lcd_writeFrame();
		DELAY_MS(DELAY_EX3 + 20); //added 20ms delay for smoothness (40ms total delay)
	}
	lcd_frameDisable();
}
