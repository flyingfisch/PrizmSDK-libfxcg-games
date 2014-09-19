#include <fxcg/display.h>
#include <fxcg/keyboard.h>

void CopySpriteNbit(const unsigned char* data, int x, int y, int width, int height, const color_t* palette, unsigned int bitwidth) {
   color_t* VRAM = (color_t*)0xA8000000;
   VRAM += (LCD_WIDTH_PX*y + x);
   int offset = 0;
   unsigned char buf;
   for(int j=y; j<y+height; j++) {
      int availbits = 0;
      for(int i=x; i<x+width;  i++) {
         if (!availbits) {
            buf = data[offset++];
            availbits = 8;
         }
         color_t this = ((color_t)buf>>(8-bitwidth));
         *VRAM = palette[(color_t)this];
         VRAM++;
         buf<<=bitwidth;
         availbits-=bitwidth;
      }
      VRAM += (LCD_WIDTH_PX-width);
   }
} 

void CopySpriteNbitMasked(const unsigned char* data, int x, int y, int width, int height, const color_t* palette, color_t maskColor, unsigned int bitwidth) {
   color_t* VRAM = (color_t*)0xA8000000;
   VRAM += (LCD_WIDTH_PX*y + x);
   int offset = 0;
   unsigned char buf;
   for(int j=y; j<y+height; j++)  
   {
      int availbits = 0;
      for(int i=x; i<x+width;  i++)  
      {
         if (!availbits)  
         {
            buf = data[offset++];
            availbits = 8;
         }
         color_t this = ((color_t)buf>>(8-bitwidth));
         color_t color = palette[this];
         if(color != maskColor)
         {
            *VRAM = color;
         }
         VRAM++;
         buf<<=bitwidth;
         availbits-=bitwidth;
      }
      VRAM += (LCD_WIDTH_PX-width);
   }
}

const unsigned short* keyboard_register = (unsigned short*)0xA44B0000;
unsigned short lastkey[8];
unsigned short holdkey[8];

void keyupdate(void) {
 memcpy(holdkey, lastkey, sizeof(unsigned short)*8);
 memcpy(lastkey, keyboard_register, sizeof(unsigned short)*8);
}
int keydownlast(int basic_keycode) {
 int row, col, word, bit; 
 row = basic_keycode%10; 
 col = basic_keycode/10-1; 
 word = row>>1; 
 bit = col + 8*(row&1); 
 return (0 != (lastkey[word] & 1<<bit)); 
}
int keydownhold(int basic_keycode) {
 int row, col, word, bit; 
 row = basic_keycode%10; 
 col = basic_keycode/10-1; 
 word = row>>1; 
 bit = col + 8*(row&1); 
 return (0 != (holdkey[word] & 1<<bit)); 
}

//draws a point of color color at (x0, y0)
void plot(int x0, int y0, int color) {
   char* VRAM = (char*)0xA8000000;
   VRAM += 2*(y0*LCD_WIDTH_PX + x0);
   *(VRAM++) = (color&0x0000FF00)>>8;
   *(VRAM++) = (color&0x000000FF);
   return;
}

//Uses the Bresenham line algorithm
void drawLine(int x1, int y1, int x2, int y2, int color) {
    signed char ix;
    signed char iy;
 
    // if x1 == x2 or y1 == y2, then it does not matter what we set here
    int delta_x = (x2 > x1?(ix = 1, x2 - x1):(ix = -1, x1 - x2)) << 1;
    int delta_y = (y2 > y1?(iy = 1, y2 - y1):(iy = -1, y1 - y2)) << 1;
 
   plot(x1, y1, color);  
    if (delta_x >= delta_y) {
        int error = delta_y - (delta_x >> 1);        // error may go below zero
        while (x1 != x2) {
            if (error >= 0) {
                if (error || (ix > 0)) {
                    y1 += iy;
                    error -= delta_x;
                }                           // else do nothing
         }                              // else do nothing
            x1 += ix;
            error += delta_y;
            plot(x1, y1, color);
        }
    } else {
        int error = delta_x - (delta_y >> 1);      // error may go below zero
        while (y1 != y2) {
            if (error >= 0) {
                if (error || (iy > 0)) {
                    x1 += ix;
                    error -= delta_y;
                }                           // else do nothing
            }                              // else do nothing
            y1 += iy;
            error += delta_x;  
            plot(x1, y1, color);
        }
    }
} 

//fills a rectangular area of (width,height) with upper-left
//corner at (x,y) with color color
void fillArea(int x, int y, int width, int height, int color) {
   //only use lower two bytes of color
   char* VRAM = (char*)0xA8000000;
   VRAM += 2*(LCD_WIDTH_PX*y + x);
   for(int j=y; j<y+height; j++) {
      for(int i=x; i<x+width;  i++) {
         *(VRAM++) = (color&0x0000FF00)>>8;
         *(VRAM++) = (color&0x000000FF);
      }
      VRAM += 2*(LCD_WIDTH_PX-width);
   }
}

// wait routine
void wait(int ms) {
   int start_time = RTC_GetTicks();
   int key;
   while ((RTC_GetTicks()-start_time)<ms) {
      // handle [menu]
      if (PRGM_GetKey()==48) {
         GetKey(&key);
      }
   }
}