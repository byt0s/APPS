// Author:       Petr Olivka, petr.olivka@vsb.cz, 09/2019
// Modified by:  La_Blazer

#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <opencv2/opencv.hpp>
#include "font8x8.hpp"
#include <opencv2\core\opengl.hpp>

#if defined (__unix__)
#include <unistd.h>
#endif


#define LCD_WIDTH		320
#define LCD_HEIGHT		240
#define LCD_SCALING		2
#define LCD_NAME		"Virtual LCD"
#define SPACE			2

// Virtual LCD
cv::Mat g_canvas(cv::Size(LCD_WIDTH, LCD_HEIGHT), CV_8UC3);

// Put color pixel on LCD (canvas)
void lcd_put_pixel(int t_x, int t_y, int t_rgb_565)
{
	if (t_x >= 0 && t_x < LCD_WIDTH && t_y >= 0 && t_y < LCD_HEIGHT) {
		// Transform the color from a LCD form into the OpenCV form.
		cv::Vec3b l_rgb_888(
			(t_rgb_565 & 0x1F) << 3,
			((t_rgb_565 >> 5) & 0x3F) << 2,
			((t_rgb_565 >> 11) & 0x1F) << 3
		);
		g_canvas.at<cv::Vec3b>(t_y, t_x) = l_rgb_888; // put pixel
	}
	else {
		std::cout << "WARNING: Bad coordinates! (x: " << t_x << ", y: " << t_y << ")" << std::endl;
	}
}
struct Point2D
{
	int32_t x, y;
};

struct RGB
{
	uint8_t r, g, b;
};

class GraphElement
{
public:
	// foreground and background color
	RGB fg_color, bg_color;

	// constructor
	GraphElement(RGB t_fg_color, RGB t_bg_color) :
		fg_color(t_fg_color), bg_color(t_bg_color) {}

	// ONLY ONE INTERFACE WITH LCD HARDWARE!!!
	void drawPixel(int32_t t_x, int32_t t_y) { lcd_put_pixel(t_x, t_y, convert_RGB888_to_RGB565(fg_color)); }

	// Draw graphics element
	virtual void draw() = 0;

	// Hide graphics element
	virtual void hide() { swap_fg_bg_color(); draw(); swap_fg_bg_color(); }
private:
	// swap foreground and backgroud colors
	void swap_fg_bg_color() { RGB l_tmp = fg_color; fg_color = bg_color; bg_color = l_tmp; }

	// IMPLEMENT!!!
	// conversion of 24-bit RGB color into 16-bit color format
	int convert_RGB888_to_RGB565(RGB t_color) { return 0x07E0; /* green color */ }
};


int convert_RGB888_to_RGB565(RGB t_color)
{
	union URGB { struct { int b : 5; int g : 6; int r : 5; }; short rgb565; } urgb;
	urgb.r = (t_color.r >> 3) & 0x1F;
	urgb.g = (t_color.g >> 2) & 0x3F;
	urgb.b = (t_color.b >> 3) & 0x1F;
	return urgb.rgb565;
}

// Clear LCD
void lcd_clear()
{
	cv::Vec3b l_black(0, 0, 0);
	g_canvas.setTo(l_black);
}

// LCD Initialization 
void lcd_init()
{
	cv::namedWindow(LCD_NAME, cv::WindowFlags::WINDOW_KEEPRATIO);
	cv::resizeWindow(LCD_NAME, { LCD_WIDTH * LCD_SCALING, LCD_HEIGHT * LCD_SCALING });
	lcd_clear();
	cv::waitKey(1);
}

// Displays virtual LCD's contents
// wait =	-1 for no waiting
//	0 to wait indefinitely for key press
//	n to wait n milliseconds for key press
void lcd_show(int wait = -1) {
	if (cv::getWindowProperty(LCD_NAME, cv::WindowPropertyFlags::WND_PROP_VISIBLE)) {
		cv::imshow(LCD_NAME, g_canvas);
	}
	else {
		cv::destroyWindow(LCD_NAME);
		lcd_init();
		cv::resizeWindow(LCD_NAME, LCD_WIDTH * LCD_SCALING, LCD_HEIGHT * LCD_SCALING);
	}
	
	if(wait >= 0)
		cv::waitKey(wait);
}

void drawCircle(int x0, int y0, int r, int color) {
	int x = r - 1;
	int y = 0;
	int dx = 1;
	int dy = 1;
	int err = dx - (r << 1);

	while (x >= y)
	{
		lcd_put_pixel(x0 + x, y0 + y, color);
		lcd_put_pixel(x0 + y, y0 + x, color);
		lcd_put_pixel(x0 - y, y0 + x, color);
		lcd_put_pixel(x0 - x, y0 + y, color);
		lcd_put_pixel(x0 - x, y0 - y, color);
		lcd_put_pixel(x0 - y, y0 - x, color);
		lcd_put_pixel(x0 + y, y0 - x, color);
		lcd_put_pixel(x0 + x, y0 - y, color);

		if (err <= 0)
		{
			y++;
			err += dy;
			dy += 2;
		}
		if (err > 0)
		{
			x--;
			dx += 2;
			err += (-r << 1) + dx;
		}
	}
}

void drawChar(char ch, int x, int y, int color) {
	// height
	for (int i = 0; i < 8; i++) {
		// width
		for (int j = 0; j < 8; j++) {
			if (font8x8[ch][i] & 1 << j) {
				lcd_put_pixel(x + j, y + i, color);
			}
		}
	}
}

void drawText(char* s, int x, int y, int color) {
	int index = 0;
	while (s[index] != '\0') {
		drawChar(s[index], x + (index * (8 + SPACE)), y, color);
		index++;
	}
}

void drawLine(int x1, int y1, int x2, int y2, int color) {
	int x, y, dx, dy, dx1, dy1, px, py, xe, ye, i;
	dx = x2 - x1;
	dy = y2 - y1;
	dx1 = fabs(dx);
	dy1 = fabs(dy);
	px = 2 * dy1 - dx1;
	py = 2 * dx1 - dy1;
	if (dy1 <= dx1) {
		if (dx >= 0) {
			x = x1;
			y = y1;
			xe = x2;
		}
		else {
			x = x2;
			y = y2;
			xe = x1;
		}
		lcd_put_pixel(x, y, color);
		for (i = 0; x < xe; i++) {
			x = x + 1;
			if (px < 0) {
				px = px + 2 * dy1;
			}
			else {
				if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) {
					y = y + 1;
				}
				else {
					y = y - 1;
				}
				px = px + 2 * (dy1 - dx1);
			}
			lcd_put_pixel(x, y, color);
		}
	}
	else {
		if (dy >= 0) {
			x = x1;
			y = y1;
			ye = y2;
		}
		else {
			x = x2;
			y = y2;
			ye = y1;
		}
		lcd_put_pixel(x, y, color);
		for (i = 0; y < ye; i++) {
			y = y + 1;
			if (py <= 0) {
				py = py + 2 * dx1;
			}
			else {
				if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) {
					x = x + 1;
				}
				else {
					x = x - 1;
				}
				py = py + 2 * (dx1 - dy1);
			}
			lcd_put_pixel(x, y, color);
		}
	}
}
