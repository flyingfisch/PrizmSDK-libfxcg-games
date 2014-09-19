#include <fxcg/display.h>
#include <fxcg/keyboard.h>
 
void main(void) {
	int key;

	while (1) {
		Bdisp_AllClr_VRAM();
		GetKey(&key);
		switch (key) {
		}
	}

	return;
}