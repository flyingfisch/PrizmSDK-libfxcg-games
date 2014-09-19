#include <fxcg/display.h>
#include <keyboard_syscalls.h>
#include <keyboard.hpp>
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef int int32_t;
typedef int fixed_t;
static fixed_t divX;
static fixed_t addX;
static fixed_t divY;
static fixed_t addY;
static fixed_t stepX;
static fixed_t stepY;
#define preMan 13
static fixed_t scaleM[4] = {-2<<preMan,1<<preMan,-1<<preMan,1<<preMan};
const unsigned short* keyboard_register = (unsigned short*)0xA44B0000;
unsigned short lastkey[8];
unsigned short holdkey[8];
void keyupdate(void) {
	memcpy(holdkey, lastkey, sizeof(unsigned short)*8);
	memcpy(lastkey, keyboard_register, sizeof(unsigned short)*8);
}
int keydownlast(int basic_keycode){
	int row, col, word, bit; 
	row = basic_keycode%10; 
	col = basic_keycode/10-1; 
	word = row>>1; 
	bit = col + 8*(row&1); 
	return (0 != (lastkey[word] & 1<<bit)); 
}
int keydownhold(int basic_keycode){
	int row, col, word, bit; 
	row = basic_keycode%10; 
	col = basic_keycode/10-1; 
	word = row>>1; 
	bit = col + 8*(row&1); 
	return (0 != (holdkey[word] & 1<<bit)); 
}
inline fixed_t abs(fixed_t val){
	if (val < 0)
		val*=-1;
	return val;
}
inline void plotMan(fixed_t x,fixed_t y,uint16_t col){
	uint16_t * vramAd=(uint16_t *)0xA8000000;
	x+=addX;
	x=x*384/divX;
	y+=addY;
	y=y*216/divY;
	vramAd+=(y*384)+x;
	*vramAd=col;
}
inline fixed_t square(fixed_t x){
	return x*x;
}
void setScale(fixed_t * scale){//format minx maxx miny maxy
	divX=abs(scale[0]-scale[1]);
	divY=abs(scale[2]-scale[3]);
	addX=scale[0]*-1;
	addY=scale[2]*-1;
	stepX=divX/384;
	stepY=divY/216;
}
uint16_t ManIt(fixed_t c_r,fixed_t c_i,uint16_t maxit){//manIt stands for mandelbrot iteration what did you think it stood for?
	//c_r = scaled x coordinate of pixel (must be scaled to lie somewhere in the mandelbrot X scale (-2.5, 1)
	//c_i = scaled y coordinate of pixel (must be scaled to lie somewhere in the mandelbrot Y scale (-1, 1)
	// squre optimaztion code below orgionally from http://randomascii.wordpress.com/2011/08/13/faster-fractals-through-algebra/
	//early bailout code from http://locklessinc.com/articles/mandelbrot/
	//changed by me to use fixed point math
	fixed_t ckr,cki;
	unsigned p=0,ptot=8;
	fixed_t z_r = c_r;
	fixed_t z_i = c_i;
	fixed_t zrsqr = (z_r * z_r)>>preMan;
	fixed_t zisqr = (z_i * z_i)>>preMan;
	//int zrsqr,zisqr;
	do{
		ckr = z_r;
		cki = z_i;
		ptot += ptot;
		if (ptot > maxit) ptot = maxit;
		for (; p < ptot;++p){
			z_i =(square(z_r+z_i)>>preMan)-zrsqr-zisqr;
			z_i += c_i;
			z_r = zrsqr-zisqr+c_r;
			zrsqr = (z_r*z_r)>>preMan;
			zisqr = (z_i*z_i)>>preMan;
			if ((zrsqr + zisqr) > (4<<preMan))
				return p*0xFFFF/maxit;
			if ((z_r == ckr) && (z_i == cki))
				return 0xFFFF;
		}
	} while (ptot != maxit);
	//plotMan(x0,y0,(uint32_t)iteration*(uint32_t)0xFFFF/(uint32_t)maxit);
	//return (uint32_t)p*(uint32_t)0xFFFF/(uint32_t)maxit;
	return 0xFFFF;
}
void mandel(uint16_t maxit){
	fixed_t x,y;
	uint16_t xx;
	uint16_t * vramAd=(uint16_t *)0xA8000000;
	for (y=scaleM[2];y<scaleM[3];y+=stepY){
		xx=0;
		for (x=scaleM[0];x<scaleM[1];x+=stepX){
			if (xx > 400)
				return;
			*vramAd++=ManIt(x,y,maxit);
			xx++;
		}
		vramAd+=384-xx;
	}
	Bdisp_PutDisp_DD();
}
void mandel4(uint16_t maxit){
	static uint16_t temp[100];
	fixed_t x,y;
	uint16_t xx;
	uint8_t yy,z;
	uint16_t * vramAd=(uint16_t *)0xA8000000;
	for (y=scaleM[2];y<scaleM[3];y+=stepY*4){
		xx=0;
		for (x=scaleM[0];x<scaleM[1];x+=stepX*4){
			if (xx > 96)
				return;
			temp[xx]=ManIt(x,y,maxit);
			xx++;
		}
		for (yy=0;yy<4;++yy){
			for (x=0;x<96;++x){
				*vramAd++=temp[x];
				*vramAd++=temp[x];
				*vramAd++=temp[x];
				*vramAd++=temp[x];
			}
		}
	//	vramAd+=384-xx;
	//	vramAd+=384*15;
	}
	Bdisp_PutDisp_DD();
}
/*int PRGM_GetKey(void)
{
  unsigned char buffer[12];
  PRGM_GetKey_OS( buffer );
  return ( buffer[1] & 0x0F ) * 10 + ( ( buffer[2] & 0xF0 ) >> 4 );
}*/
void main(void) {
	Bdisp_EnableColor(1);
	Bdisp_AllClr_VRAM();
	setScale(scaleM);
	uint16_t z=31;
	mandel(z);
	uint8_t redraw=1;
	//uint16_t z=28;//good values for z 28 31 36 41 42 31 is my favoirte so far
	do{
		keyupdate();
		if (keydownlast(KEY_PRGM_F1) && keydownhold(KEY_PRGM_F1)){
			z=0xFFFF;
			redraw=2;
		}
		if (keydownlast(KEY_PRGM_F2) && keydownhold(KEY_PRGM_F2)){
			z=31;
			redraw=2;
		}
		if (keydownlast(KEY_PRGM_1) && keydownhold(KEY_PRGM_1)){
			z--;
			redraw=2;
		}
		if (keydownlast(KEY_PRGM_2) && keydownhold(KEY_PRGM_2)){
			z++;
			redraw=2;
		}
		if (keydownlast(KEY_PRGM_SHIFT) && keydownhold(KEY_PRGM_SHIFT)){
			scaleM[0]+=256;
			scaleM[1]-=256;
			scaleM[2]+=144;
			scaleM[3]-=144;
			setScale(scaleM);
			redraw=2;
		}
		if (keydownlast(KEY_PRGM_ALPHA) && keydownhold(KEY_PRGM_ALPHA)){
			scaleM[0]-=256;
			scaleM[1]+=256;
			scaleM[2]-=144;
			scaleM[3]+=144;
			setScale(scaleM);
			redraw=2;
		}
		if (keydownlast(KEY_PRGM_UP) && keydownhold(KEY_PRGM_UP)){
			scaleM[2]-=144;
			scaleM[3]-=144;
			setScale(scaleM);
			redraw=2;
		}
		if (keydownlast(KEY_PRGM_DOWN) && keydownhold(KEY_PRGM_DOWN))
		{
			scaleM[2]+=144;
			scaleM[3]+=144;
			setScale(scaleM);
			redraw=2;
		}
		if (keydownlast(KEY_PRGM_LEFT) && keydownhold(KEY_PRGM_LEFT)){
			scaleM[0]-=256;
			scaleM[1]-=256;
			setScale(scaleM);
			redraw=2;
		}
		if (keydownlast(KEY_PRGM_RIGHT) && keydownhold(KEY_PRGM_RIGHT)){
			scaleM[0]+=256;
			scaleM[1]+=256;
			setScale(scaleM);
			redraw=2;
		}
		if (redraw == 1){
			mandel(z);
			redraw=0;
		}
		else if (redraw == 2){
			mandel4(z);
			redraw=1;
		}
	} while (!(keydownlast(KEY_PRGM_EXIT) && keydownhold(KEY_PRGM_EXIT)));
	//MsgBoxPop();
	return;
}