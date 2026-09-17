#include <stdio.h>
#include "graphics.h"

int main(void)
{
	printf("Hello World! %s\n", CONFIG_BOARD_TARGET);
	printf("Running Tetrimini!!!\n");

	gfx_init();

	gfx_playfield();

	// t block
	gfx_block(3, 2);
	gfx_block(4, 2);
	gfx_block(5, 2);
	gfx_block(4, 3);

	// full line
	for (int i = 0; i < 10; i++)
	{
		gfx_block(i, 17);
	}

	gfx_flush();

	return 0;
}
