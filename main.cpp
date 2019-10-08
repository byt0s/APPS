#include "lcd-simulator.hpp"

int main()
{
	lcd_init();
	lcd_clear();

	
	int l_color_red = 245 | 13 | 5;
	int l_color_green = 17 | 240 | 76;
	int l_color_blue = 255 | 255 | 255;
	int l_color_white = 25 | 10 | 242;
	int l_color_orange = 242 | 149 | 10;
	int l_color_brown = 79 | 50 | 6;
	int l_color_cyane = 2 | 247 | 231;

	float time = 0.;
	int centerX = LCD_WIDTH / 2, centerY = LCD_HEIGHT / 2;
	const float scale = 70.;

	int* colors = new int[l_color_blue, l_color_brown, l_color_cyane, l_color_green, l_color_orange, l_color_red, l_color_white];

	while (1) {
		lcd_clear();

		time += 0.05;

		/*lcd_put_pixel(centerX + (int)(sin(time) * scale), centerY + (int)(cos(time) * scale), l_color_red);
		lcd_put_pixel(centerX + (int)(sin(time + 1) * scale), centerY + (int)(cos(time + 1) * scale), l_color_green);
		lcd_put_pixel(centerX + (int)(sin(time + 2) * scale), centerY + (int)(cos(time + 2) * scale), l_color_blue);
		lcd_put_pixel(centerX + (int)(sin(time + 3) * scale), centerY + (int)(cos(time + 3) * scale), l_color_white);
		lcd_put_pixel(centerX + (int)(sin(time + 4) * scale), centerY + (int)(cos(time + 4) * scale), l_color_orange);
		lcd_put_pixel(centerX + (int)(sin(time + 5) * scale), centerY + (int)(cos(time + 5) * scale), l_color_brown);
		lcd_put_pixel(centerX + (int)(sin(time + 0.2) * scale), centerY + (int)(cos(time + 2) * scale), l_color_cyane);*/

		drawCircle(centerX, centerY, 70, l_color_green);
		drawLine(centerX, centerY, (centerX + (int)(sin(time) * scale)), (centerY + (int)(-cos(time) * scale)), l_color_blue);
		drawText("Ahoj", 100, 100, l_color_orange);

		lcd_show(30);
	}
}
