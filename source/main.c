#include "main.h"
#include "draw3d.h"
#include "draw3d.h"
#include "noise.h"
#include <time.h>

// Asset from Nitro Engine's font example
#include "text_bmp_bin.h"

NE_Material *TextMaterial = NULL;
NE_Palette *textPalette = NULL;

int main(void)
{
	irqEnable(IRQ_HBLANK);
	irqSet(IRQ_VBLANK, NE_VBLFunc);
	irqSet(IRQ_HBLANK, NE_HBLFunc);
	srand(time(NULL));

	// Set sand height
	float sandXOff = 10;
	float sandYOff = 5;
	for (int x = 0; x < WATER_SIZE; x++)
	{
		for (int y = 0; y < WATER_SIZE; y++)
		{
			// Set sand height from noise
			sandHeight[x][y].height = noise2((x + sandXOff) / 4.0, (y + sandYOff) / 4.0) * 2 - 1;
			// Get height int version for fast water simulation
			sandHeight[x][y].intHeight = sandHeight[x][y].height * 4096;
		}
	}

	// Init the engine
	NE_Init3D();
	consoleDemoInit();

	// Set camera settings
	NE_ClippingPlanesSetI(floattof32(0.1), floattof32(90.0)); // Set render distance
	NE_AntialiasEnable(true);
	NE_ClearColorSet(RGB15(0, 0, 0), 31, 63); // Black sky

	// Load font
	textPalette = NE_PaletteCreate();
	TextMaterial = NE_MaterialCreate();
	NE_MaterialTexLoadBMPtoRGB256(TextMaterial, textPalette, (void *)text_bmp_bin, true); // Load bmp font format

	// Create font
	NE_TextInit(0,			  // Font slot
				TextMaterial, // Image
				8, 8);		  // Size of one character (x, y)

	InitGraphics();

	// Init water grid
	UpdateWater(true);

	// Render the scene
	while (true)
	{
		// Check player inputs
		scanKeys();
		int keysdown = keysDown();
		if (keysdown & KEY_A)
			ChangeWaterStyle();
		if (keysdown & KEY_B)
			ChangeWaterMode();

		NE_Process(Draw3DScene);
		NE_WaitForVBL(NE_CAN_SKIP_VBL);
	}
}