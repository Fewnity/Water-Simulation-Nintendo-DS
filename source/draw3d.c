#include "draw3d.h"
#include "noise.h"
#include <math.h>

// Asset from https://www.kenney.nl/assets/topdown-tanks-redux
#include "crateWood_bin.h"
#include "tileSand_bin.h"

// Camera variables
NE_Camera *Camera;
float angle = 0;

// All water points
WaterPoint water[WATER_SIZE][WATER_SIZE];
// All sand height points
SandPoint sandHeight[WATER_SIZE][WATER_SIZE];
// Water noise offset
float waterXOff = 0;
float waterYOff = 0;
int waterGridXOff = 0;
int waterGridYOff = 0;

#define WAVE_HEIGHT 6
#define SAND_HEIGHT 3
#define WAVE_HEIGHT_INT (WAVE_HEIGHT * 4096)
#define SAND_HEIGHT_INT (SAND_HEIGHT * 4096)

#define CUBE_VERTEX_COUNT 72 / 3
#define COLOR_WHITE RGB15(31, 31, 31)

// Position of the cube
float cubeXPos = 10;
float cubeYPos = 2;
float cubeZPos = 16;

bool needUpdateWater = false;
// Clear water rendering style
bool clearWater = true;
// Fast water simulation
bool fastWater = false;

// For textures
NE_Material *materialTileSand = NULL;
NE_Palette *paletteTileSand = NULL;
NE_Material *materialCrateWood = NULL;
NE_Palette *paletteCrateWood = NULL;

// Cube vertices
int cubeVert[72] = {
	-1, 1, 1, // 0
	-1, -1, 1,
	1, -1, 1,
	1, 1, 1,
	1, 1, -1, // 1
	1, -1, -1,
	-1, -1, -1,
	-1, 1, -1,
	1, 1, 1, // 2
	1, -1, 1,
	1, -1, -1,
	1, 1, -1,
	-1, 1, -1, // 3
	-1, -1, -1,
	-1, -1, 1,
	-1, 1, 1,
	1, 1, 1, // 4
	1, 1, -1,
	-1, 1, -1,
	-1, 1, 1,
	-1, -1, 1, // 5
	-1, -1, -1,
	1, -1, -1,
	1, -1, 1};

// Cube uvs
u32 cubeUv[] = {
	TEXTURE_PACK(inttot16(0), inttot16(55)), // 0
	TEXTURE_PACK(inttot16(0), inttot16(0)),
	TEXTURE_PACK(inttot16(55), inttot16(0)),
	TEXTURE_PACK(inttot16(55), inttot16(55)),
	TEXTURE_PACK(inttot16(55), inttot16(55)), // 1
	TEXTURE_PACK(inttot16(55), inttot16(0)),
	TEXTURE_PACK(inttot16(0), inttot16(0)),
	TEXTURE_PACK(inttot16(0), inttot16(55)),
	TEXTURE_PACK(inttot16(55), inttot16(55)), // 2
	TEXTURE_PACK(inttot16(0), inttot16(55)),
	TEXTURE_PACK(inttot16(0), inttot16(0)),
	TEXTURE_PACK(inttot16(55), inttot16(0)),
	TEXTURE_PACK(inttot16(55), inttot16(0)), // 3
	TEXTURE_PACK(inttot16(0), inttot16(0)),
	TEXTURE_PACK(inttot16(0), inttot16(55)),
	TEXTURE_PACK(inttot16(55), inttot16(55)),
	TEXTURE_PACK(inttot16(55), inttot16(55)), // 4
	TEXTURE_PACK(inttot16(55), inttot16(0)),
	TEXTURE_PACK(inttot16(0), inttot16(0)),
	TEXTURE_PACK(inttot16(0), inttot16(55)),
	TEXTURE_PACK(inttot16(0), inttot16(55)), // 5
	TEXTURE_PACK(inttot16(0), inttot16(0)),
	TEXTURE_PACK(inttot16(55), inttot16(0)),
	TEXTURE_PACK(inttot16(55), inttot16(55)),
};

int Lerp(int a, int b, float f)
{
	return a * (1 - f) + (b * f);
}

/**
 * @brief Set the camera position based on the camera angle
 *
 */
void SetCameraPosition()
{
	NE_CameraSet(Camera,
				 WATER_SIZE - sinf(angle) * WATER_SIZE, 12, WATER_SIZE - cosf(angle) * WATER_SIZE,
				 WATER_SIZE, 1, WATER_SIZE,
				 0, 1, 0);
}

/**
 * @brief Init graphics
 *
 */
void InitGraphics()
{
	Camera = NE_CameraCreate();

	SetCameraPosition();

	if (!fastWater)
	{
		// Set a random water offset
		waterXOff = rand() % 10000;
		waterYOff = rand() % 10000;
	}

	// Load textures
	paletteTileSand = NE_PaletteCreate();
	materialTileSand = NE_MaterialCreate();
	NE_MaterialTexLoadBMPtoRGB256(materialTileSand, paletteTileSand, (void *)tileSand_bin, 1);

	paletteCrateWood = NE_PaletteCreate();
	materialCrateWood = NE_MaterialCreate();
	NE_MaterialTexLoadBMPtoRGB256(materialCrateWood, paletteCrateWood, (void *)crateWood_bin, 1);
}

/**
 * @brief Set the Water color from a coor
 *
 * @param x
 * @param y
 */
void SetWaterColor(int x, int y)
{
	WaterPoint *point = &water[x][y];
	// Get the difference between the height of the sand and the height of the water
	int heightDiff = 0;
	if (!fastWater)
		heightDiff = (point->height * 2 - (sandHeight[x][y].height)) * 200;
	else
		heightDiff = (point->finalHeight * 2 - (sandHeight[x][y].intHeight)) * 200 / 4096;

	// Set color intensity of the basic water color
	int colorIntensity = 0;
	if (!fastWater)
		colorIntensity = point->height * 11;
	else
		colorIntensity = point->finalHeight / 4096.0f * 11;

	// If the water is close to the sand
	if (heightDiff <= 20)
	{
		// If the water is under the sand, the value is lower than 0 so put the value to 0
		if (heightDiff < 0)
			heightDiff = 0;

		// Ration for interpolation between the basic water color and the light water color when close to the sand
		float heightDifRatio = heightDiff / 20.0f;
		if (clearWater)
		{
			int col1 = Lerp(20, colorIntensity, heightDifRatio);
			int col2 = Lerp(20, 7 + colorIntensity, heightDifRatio);
			point->color = RGB15(col1, col1, col2);
		}
		else
		{
			int col1 = Lerp(20, 5 - colorIntensity / 2, heightDifRatio);
			int col2 = Lerp(20, 11 - colorIntensity, heightDifRatio);
			int col3 = Lerp(31, 31 - colorIntensity, heightDifRatio);
			point->color = RGB15(col1, col2, col3);
		}
	}
	else // Basic water color
	{
		if (clearWater)
			point->color = RGB15(colorIntensity, colorIntensity, 7 + colorIntensity);
		else
			point->color = RGB15(5 - colorIntensity / 2, 11 - colorIntensity, 31 - colorIntensity);
	}
}

/**
 * @brief Draw sand ground
 *
 */
void DrawSand()
{
	NE_PolyFormat(31, 0, NE_LIGHT_0, NE_CULL_NONE, NE_MODULATION);
	NE_PolyBegin(GL_QUAD);
	NE_MaterialUse(materialTileSand);

	// Create a plane under the sand to hide artefacts due to float precision
	glPushMatrix();
	glTranslatef32(inttov16(WATER_SIZE + 1), inttov16(-2), inttov16(WATER_SIZE + 1));
	glScalef32(inttov16(WATER_SIZE + 1), inttov16(1), inttov16(WATER_SIZE + 1));
	GFX_TEX_COORD = TEXTURE_PACK(inttot16(127), inttot16(127));
	glVertex3v16(inttov16(1), 0, inttov16(1));
	GFX_TEX_COORD = TEXTURE_PACK(inttot16(127), inttot16(0));
	glVertex3v16(inttov16(1), 0, inttov16(-1));
	GFX_TEX_COORD = TEXTURE_PACK(inttot16(0), inttot16(0));
	glVertex3v16(inttov16(-1), 0, inttov16(-1));
	GFX_TEX_COORD = TEXTURE_PACK(inttot16(0), inttot16(127));
	glVertex3v16(inttov16(-1), 0, inttov16(1));
	glPopMatrix(1);

	// Draw sand tiles
	glPushMatrix();
	glScalef32(inttov16(1), SAND_HEIGHT_INT, inttov16(1));
	for (int x = 1; x < WATER_SIZE; x++)
	{
		for (int y = 1; y < WATER_SIZE; y++)
		{
			glPushMatrix();
			glTranslatef32(inttov16(x * 2), inttov16(0), inttov16(y * 2));

			GFX_TEX_COORD = TEXTURE_PACK(inttot16(127), inttot16(127));
			glVertex3v16(inttov16(1), sandHeight[x][y].intHeight, inttov16(1));

			GFX_TEX_COORD = TEXTURE_PACK(inttot16(127), inttot16(0));
			glVertex3v16(inttov16(1), sandHeight[x][y - 1].intHeight, inttov16(-1));

			GFX_TEX_COORD = TEXTURE_PACK(inttot16(0), inttot16(0));
			glVertex3v16(inttov16(-1), sandHeight[x - 1][y - 1].intHeight, inttov16(-1));

			GFX_TEX_COORD = TEXTURE_PACK(inttot16(0), inttot16(127));
			glVertex3v16(inttov16(-1), sandHeight[x - 1][y].intHeight, inttov16(1));

			glPopMatrix(1);
		}
	}
	glPopMatrix(1);

	NE_PolyEnd();
}

/**
 * @brief Draw animated water
 *
 */
void DrawWater()
{
	// Set water transparency
	if (clearWater)
		NE_PolyFormat(15, 0, NE_LIGHT_0, NE_CULL_NONE, NE_MODULATION);
	else
		NE_PolyFormat(25, 0, NE_LIGHT_0, NE_CULL_NONE, NE_MODULATION);

	NE_PolyBegin(GL_QUAD);
	// Do not use a texture for the wayer
	NE_MaterialUse(NULL);

	// Draw water tiles
	glPushMatrix();
	glScalef32(inttov16(1), WAVE_HEIGHT_INT, inttov16(1));
	for (int x = 1; x < WATER_SIZE; x++)
	{
		for (int y = 1; y < WATER_SIZE; y++)
		{
			glPushMatrix();
			glTranslatef32(inttov16(x * 2), inttov16(0), inttov16(y * 2));

			GFX_COLOR = water[x][y].color;
			glVertex3v16(inttov16(1), water[x][y].finalHeight, inttov16(1));

			GFX_COLOR = water[x][y - 1].color;
			glVertex3v16(inttov16(1), water[x][y - 1].finalHeight, inttov16(-1));

			GFX_COLOR = water[x - 1][y - 1].color;
			glVertex3v16(inttov16(-1), water[x - 1][y - 1].finalHeight, inttov16(-1));

			GFX_COLOR = water[x - 1][y].color;
			glVertex3v16(inttov16(-1), water[x - 1][y].finalHeight, inttov16(1));

			glPopMatrix(1);
		}
	}
	glPopMatrix(1);

	NE_PolyEnd();
}

/**
 * @brief Update water points
 *
 */
void UpdateWater(bool initFastWater)
{
	needUpdateWater = !needUpdateWater;
	float noiseXOffset = 0;
	for (int x = 0; x < WATER_SIZE; x++)
	{
		if (needUpdateWater || initFastWater || !fastWater)
			noiseXOffset = (x + waterXOff) / 10.0f;

		for (int y = 0; y < WATER_SIZE; y++)
		{
			WaterPoint *point = &water[x][y];
			// Update point every two frames
			needUpdateWater = !needUpdateWater;
			if (needUpdateWater || initFastWater)
			{
				// Perlin noise
				if (!fastWater || initFastWater)
				{
					point->height = noise2(noiseXOffset, (y + waterYOff) / 10.0f);
					point->intHeight = point->height * 4096;
					point->finalHeight = point->intHeight;
				}
				else
				{
					// Use interpolation from static noise value
					int h = Lerp(water[(x + waterGridXOff) % WATER_SIZE][y].intHeight, water[(x + 1 + waterGridXOff) % WATER_SIZE][y].intHeight, waterXOff);
					h += Lerp(water[x][(y + waterGridYOff) % WATER_SIZE].intHeight, water[(x)][(y + 1 + waterGridYOff) % WATER_SIZE].intHeight, waterYOff);
					h /= 2;
					point->finalHeight = h;
				}
				SetWaterColor(x, y);
			}
		}
	}
}

/**
 * @brief Draw floating cube
 *
 */
void DrawCube()
{
	NE_PolyFormat(31, 0, NE_LIGHT_0, NE_CULL_BACK, NE_MODULATION);
	NE_PolyBegin(GL_QUAD);
	NE_MaterialUse(materialCrateWood);

	// Extremely basic buoyancy simulation
	if (!fastWater)
		cubeYPos = water[5][8].height * WAVE_HEIGHT - 0.2f;
	else
		cubeYPos = water[5][8].finalHeight / 4096.0f * WAVE_HEIGHT - 0.2f;

	NE_PolyColor(COLOR_WHITE); // Set next vertices color

	//  Set cube position
	glTranslatef32(cubeXPos * 4096, cubeYPos * 4096, cubeZPos * 4096);

	// Draw all vertex
	for (int vI = 0; vI < CUBE_VERTEX_COUNT; vI++)
	{
		GFX_TEX_COORD = cubeUv[vI];
		int offset = vI * 3;
		glVertex3v16(inttov16(cubeVert[offset]), inttov16(cubeVert[offset + 1]), inttov16(cubeVert[offset + 2]));
	}

	NE_PolyEnd();
}

/**
 * @brief Change water rendering style
 *
 */
void ChangeWaterStyle()
{
	clearWater = !clearWater;
}

/**
 * @brief Change water simulation mode
 *
 */
void ChangeWaterMode()
{
	fastWater = !fastWater;
	// Reset values to avoid glitches
	if (fastWater)
	{
		waterGridXOff = 0;
		waterGridYOff = 0;
		waterXOff = 0;
		waterYOff = 0;
	}
	else
	{
		// Set a random water offset
		waterXOff = rand() % 10000;
		waterYOff = rand() % 10000;
	}
}

/**
 * @brief Update scene (camera rotation and water offset)
 *
 */
void UpdateScene()
{
	// Rotate the camera
	angle += 0.003f;
	SetCameraPosition();

	// Update water offset
	waterXOff += 0.05f;
	waterYOff += 0.05f;

	if (fastWater)
	{
		// Reset offset for fast water simulation
		if (waterXOff >= 1)
		{
			waterGridXOff++;
			waterXOff -= 1;
		}

		if (waterYOff >= 1)
		{
			waterGridYOff++;
			waterYOff -= 1;
		}
	}
}

/**
 * @brief Render scene
 *
 */
void Draw3DScene(void)
{
	// Set camera for drawing
	NE_CameraUse(Camera);

	// Update scene
	UpdateScene();

	// Update water points
	UpdateWater(false);

	// Draw sand
	DrawSand();

	// Draw water
	DrawWater();

	// Draw cube
	DrawCube();

	// Init text drawing
	NE_2DViewInit();

	// Print performance text
	char perfText[21];
	sprintf(perfText, "CPU: %d%%, poly: %d\n", NE_GetCPUPercent(), NE_GetPolygonCount());
	NE_TextPrint(0,		   // Font slot
				 1, 1,	   // Coordinates x(column), y(row)
				 NE_White, // Color
				 perfText);

	// Print player inputs
	char keyText[46];
	sprintf(keyText, "A: Change water style\n B:Change water quality");
	NE_TextPrint(0,		   // Font slot
				 1, 2,	   // Coordinates x(column), y(row)
				 NE_White, // Color
				 keyText);
}