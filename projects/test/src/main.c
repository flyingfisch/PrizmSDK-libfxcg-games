#include <fxcg/display.h>
#include <fxcg/keyboard.h>
#include <stdio.h>
 
int main() {
	int key;
	Bdisp_AllClr_VRAM();

	while (1) {
		printf("Blah\n");
		Bdisp_PutDisp_DD();
		GetKey(&key);
	}

	return 1;
}