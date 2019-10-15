#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <opencv2/opencv.hpp>
#include "font8x8.cpp"


#define LCD_WIDTH       320
#define LCD_HEIGHT      240
#define LCD_NAME        "Virtual LCD"

#define WIDTH 8
#define HEIGHT 8

using namespace std;

int offset = 16;
int dist = 10;

// LCD Simulator

// Virtual LCD
cv::Mat g_canvas( cv::Size( LCD_WIDTH, LCD_HEIGHT ), CV_8UC3 );

// Put color pixel on LCD (canvas)
void lcd_put_pixel( int t_x, int t_y, int t_rgb_565 )
{
    // Transform the color from a LCD form into the OpenCV form. 
    cv::Vec3b l_rgb_888( 
            (  t_rgb_565         & 0x1F ) << 3, 
            (( t_rgb_565 >> 5 )  & 0x3F ) << 2, 
            (( t_rgb_565 >> 11 ) & 0x1F ) << 3
            );
    g_canvas.at<cv::Vec3b>( t_y, t_x ) = l_rgb_888; // put pixel
}

// Clear LCD
void lcd_clear()
{
    cv::Vec3b l_black( 0, 0, 0 );
    g_canvas.setTo( l_black );
}

// LCD Initialization 
void lcd_init()
{
    cv::namedWindow( LCD_NAME, 0 );
    lcd_clear();
    cv::waitKey( 1 );
}

// Simple graphic interface

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
    GraphElement( RGB t_fg_color, RGB t_bg_color ) : 
        fg_color( t_fg_color ), bg_color( t_bg_color ) {}

    // ONLY ONE INTERFACE WITH LCD HARDWARE!!!
    void drawPixel( int32_t t_x, int32_t t_y ) { lcd_put_pixel( t_x, t_y, convert_RGB888_to_RGB565( fg_color ) ); }
    
    // Draw graphics element
    virtual void draw() = 0;
    
    // Hide graphics element
    virtual void hide() { swap_fg_bg_color(); draw(); swap_fg_bg_color(); }
    //virtual void hideVert() { swap_fg_bg_color(); drawVert(); swap_fg_bg_color(); } //pridal jsem ja
private:
    // swap foreground and backgroud colors
    void swap_fg_bg_color() { RGB l_tmp = fg_color; fg_color = bg_color; bg_color = l_tmp; } 

    // conversion of 24-bit RGB color into 16-bit color format
    int convert_RGB888_to_RGB565( RGB t_color )
    {
        union URGB {struct {int b:5; int g:6; int r:5;}; short rgb565; } urgb;
        urgb.r = (t_color.r >> 3) & 0x1F;
        urgb.g = (t_color.g >> 2) & 0x3F;
        urgb.b = (t_color.b >> 3) & 0x1F;
        return urgb.rgb565;
    }
};


class Pixel : public GraphElement
{
public:
    // constructor
    Pixel( Point2D t_pos, RGB t_fg_color, RGB t_bg_color ) : pos( t_pos ), GraphElement( t_fg_color, t_bg_color ) {}
    // Draw method implementation
    virtual void draw() { drawPixel( pos.x, pos.y ); }
    // Position of Pixel
    Point2D pos;
};


class Circle : public GraphElement
{
public:
    // Center of circle
    Point2D center;
    // Radius of circle
    int32_t radius;

    Circle( Point2D t_center, int32_t t_radius, RGB t_fg, RGB t_bg ) :
        center( t_center ), radius( t_radius ), GraphElement( t_fg, t_bg ) {};

    void draw()
    {
        int f = 1 - radius;
        int ddF_x = 0;
        int ddF_y = -2 * radius;
        int x = 0;
        int y = radius;
    
        int x0 = center.x;
        int y0 = center.y;

        drawPixel(x0, y0 + radius);
        drawPixel(x0, y0 - radius);
        drawPixel(x0 + radius, y0);
        drawPixel(x0 - radius, y0);
     
        while(x < y) 
        {
            if(f >= 0) 
            {
                y--;
                ddF_y += 2;
                f += ddF_y;
            }
            x++;
            ddF_x += 2;
            f += ddF_x + 1;    
            drawPixel(x0 + x, y0 + y);
            drawPixel(x0 - x, y0 + y);
            drawPixel(x0 + x, y0 - y);
            drawPixel(x0 - x, y0 - y);
            drawPixel(x0 + y, y0 + x);
            drawPixel(x0 - y, y0 + x);
            drawPixel(x0 + y, y0 - x);
            drawPixel(x0 - y, y0 - x);
        }
    } 
};

class Character : public GraphElement 
{
public:
    // position of character
    Point2D pos;
    // character
    char character;

    Character( Point2D t_pos, char t_char, RGB t_fg, RGB t_bg ) : 
      pos( t_pos ), character( t_char ), GraphElement( t_fg, t_bg ) {};

    void draw()
    { 
        for(int y = 0; y < HEIGHT; y++)
        {
            int radek_fontu = font8x8[character][y];
            //int radek_fontu = font[character][y];
            for(int x = 0; x < WIDTH; x++)
            {
                if(radek_fontu & (1 << x)) drawPixel(pos.x + x, pos.y + y);    //LSB
                //if(radek_fontu & (1 << x)) drawPixel(pos.x - x + offset, pos.y + y);    //MSB
            }
        }    
    }
};

class Line : public GraphElement
{
public:
    // the first and the last point of line
    Point2D pos1, pos2;

    Line( Point2D t_pos1, Point2D t_pos2, RGB t_fg, RGB t_bg ) : 
      pos1( t_pos1 ), pos2( t_pos2 ), GraphElement( t_fg, t_bg ) {}

    void draw()
    {
        int x0 = pos1.x;
        int y0 = pos1.y;
        int x1 = pos2.x;
        int y1 = pos2.y;

        int dx =  abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1; 
        int err = dx + dy, e2; //error value e_xy
        
        for(;;){  //loop
            drawPixel(x0, y0);
            if (x0 == x1 && y0 == y1) break;
            e2 = 2*err;
            if (e2 >= dy) { err += dy; x0 += sx; } //e_xy+e_x > 0
            if (e2 <= dx) { err += dx; y0 += sy; } //e_xy+e_y < 0
        }
    }
};

class Text : public GraphElement
{
public:
    // position of character
    Point2D pos;
    // characters
    string str;

    bool horizontal = false;

    Text( Point2D t_pos, string t_str, RGB t_fg, RGB t_bg, bool horizontal ) :
      pos( t_pos ), str( t_str ), GraphElement( t_fg, t_bg )
        {
            this->horizontal = horizontal;
        };

    void draw()
    { 
        int offs = 0;
        for (int i = 0; i < str.size(); i++)
        {
            for(int y = 0; y < HEIGHT; y++)
            {
                int radek_fontu = font8x8[str[i]][y];
                //int radek_fontu = font[str[i]][y];
                for(int x = 0; x < WIDTH; x++)
                {
                    if(horizontal)
                    {
                        if(radek_fontu & (1 << x)) drawPixel(pos.x + x + offs, pos.y + y);           //LSB
                        //if(radek_fontu & (1 << x)) drawPixel(pos.x - x + offset + offs, pos.y + y);    //MSB
                    }
                    else
                    {
                        if(radek_fontu & (1 << x)) drawPixel(pos.x + x, pos.y + y + offs);           //LSB
                        //if(radek_fontu & (1 << x)) drawPixel(pos.x - x + offset, pos.y + y + offs);    //MSB
                    }
                }
            }
            offs += offset;
        }
    }

    void move()
    {
        hide();
        pos.x += dist;
        draw();
    }
};

Point2D point1 = {10, 10};
Point2D point2 = {110, 120};
Point2D point3 = {0, 0};
Point2D point4 = {150, 50};
Point2D point5 = {150, 150};
Point2D pointCentre = {LCD_WIDTH / 2, LCD_HEIGHT /2};
RGB black = {0, 0, 0};
RGB white = {255, 255, 255};
RGB bordo = {128, 0, 32};
RGB cyan = {0, 255, 255};
RGB green = {0, 255, 0};
RGB blue = {0, 0, 255};
RGB red = {255, 0, 0};


int main()
{
    lcd_init();                     // LCD initialization

    lcd_clear();                    // LCD clear screen

    int max = 280;
    int min = 40;
   
    Text login(point2, "BYS0046", cyan, black, true);
        for(int i = 0; i < max; i++)
        {
            login.hide();
            login.pos.x += 5;
            login.draw();
        }
    while(true)
    {
        
       

        cv::imshow( LCD_NAME, g_canvas );
        cv::waitKey( 1 );
        
    }

    cv::imshow( LCD_NAME, g_canvas );   // refresh content of "LCD"
    cv::waitKey( 0 );                   // wait for key 
}



