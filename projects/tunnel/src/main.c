#include <fxcg/display.h>
#include <fxcg/keyboard.h>

/* FUNCTIONS */
// (from routines.c)
void CopySpriteNbit(const unsigned char* data, int x, int y, int width, int height, const color_t* palette, unsigned int bitwidth);
void CopySpriteNbitMasked(const unsigned char* data, int x, int y, int width, int height, const color_t* palette, color_t maskColor, unsigned int bitwidth);
int PRGM_GetKey(void);
void keyupdate(void);
int keydownlast(int basic_keycode);
int keydownhold(int basic_keycode);
void plot(int x0, int y0, int color);
void drawLine(int x1, int y1, int x2, int y2, int color);
void fillArea(int x, int y, int width, int height, int color);
void wait(int ms);

/* VARIABLES */
int tunnelarray[LCD_SCREEN_WIDTH];



/* MAIN FUNCTION */
int main() {
	setup();

	int key;
	Bdisp_AllClr_VRAM();

	while (1) {
		Bdisp_PutDisp_DD();
		GetKey(&key);
	}

	return 1;
}