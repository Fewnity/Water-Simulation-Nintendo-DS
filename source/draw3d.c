// SPDX-License-Identifier: MIT
//
// Copyright (c) 2008-2011, 2019, Antonio Niño Díaz
//
// This file is part of Nitro Engine

// #include "main.h"
#include "draw3d.h"
#include "noise.h"
#include <math.h>

// Camera variables
NE_Camera *Camera;
float angle = 0;

// All water points
WaterPoint water[10][10];
// All sand height points
float sandHeight[10][10];
// Water noise offset
float waterXOff = 0;
float waterYOff = 0;

float waveHeight = 6;

// Position of the cube
float cubeXPos = 10;
float cubeYPos = 2;
float cubeZPos = 16;

bool needUpdateWater = false;
bool clearWater = true;

// Asset from https://www.kenney.nl/assets/topdown-tanks-redux
#include "crateWood_bin.h"
#include "tileSand_bin.h"

NE_Material *materialTileSand = NULL;
NE_Palette *paletteTileSand = NULL;
NE_Material *materialCrateWood = NULL;
NE_Palette *paletteCrateWood = NULL;

#define CUBE_VERTEX_COUNT 72 / 3

#define COLOR_WHITE RGB15(31, 31, 31)

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

/**
 * @brief Set the camera position based on the camera angle
 *
 */
void SetCameraPosition()
{
	NE_CameraSet(Camera,
				 10 - sinf(angle) * 12, 12, 10 - cosf(angle) * 12,
				 10, 1, 10,
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

	// Set a random water offset
	waterXOff = rand() % 10000;
	waterYOff = rand() % 10000;

	// Load textures
	paletteTileSand = NE_PaletteCreate();
	materialTileSand = NE_MaterialCreate();
	NE_MaterialTexLoadBMPtoRGB256(materialTileSand, paletteTileSand, (void *)tileSand_bin, 1);

	paletteCrateWood = NE_PaletteCreate();
	materialCrateWood = NE_MaterialCreate();
	NE_MaterialTexLoadBMPtoRGB256(materialCrateWood, paletteCrateWood, (void *)crateWood_bin, 1);
}

float lerp(float a, float b, float f)
{
	return a * (1.0 - f) + (b * f);
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
	int heightDif = (point->height * 2 - (sandHeight[x][y])) * 200;

	// TODO : Improve the color

	// If the water is close to the sand
	int blueCol = point->height * 11;
	if (heightDif <= 20)
	{
		if (heightDif < 0)
			heightDif = 0;

		int col = 20 - heightDif;
		if (clearWater)
		{
			int col1 = lerp(col, blueCol, heightDif / 20.0);
			int col2 = lerp(col, 7 + blueCol, heightDif / 20.0);
			point->color = RGB15(col1, col1, col2);
		}
		else
		{
			int col1 = lerp(col, 5 - (int)(blueCol / 2.0), heightDif / 20.0);
			int col2 = lerp(col, 11 - blueCol, heightDif / 20.0);
			int col3 = lerp(31, 31 - blueCol, heightDif / 20.0);
			point->color = RGB15(col1, col2, col3);
		}
	}
	else
	{
		// int blueCol = point->height * 11;
		if (clearWater)
			point->color = RGB15(blueCol, blueCol, 7 + blueCol);
		else
			point->color = RGB15(5 - (int)(blueCol / 2.0), 11 - blueCol, 31 - blueCol);
		// point->color = RGB15((int)(blueCol / 2.0), blueCol, 20 + blueCol);
	}
}

/**
 * @brief Draw sand ground
 *
 */
void DrawSand()
{
	NE_PolyBegin(GL_QUAD);
	NE_MaterialUse(materialTileSand);

	for (int x = 1; x < 10; x++)
	{
		for (int y = 1; y < 10; y++)
		{
			glPushMatrix();

			glTranslatef32(inttov16(x * 2), inttov16(0), inttov16(y * 2));
			glScalef32(inttov16(1), (int)(3 * 4096), inttov16(1));

			GFX_TEX_COORD = TEXTURE_PACK(inttot16(128), inttot16(128));
			glVertex3v16(inttov16(1), sandHeight[x][y] * 4096, inttov16(1));

			GFX_TEX_COORD = TEXTURE_PACK(inttot16(128), inttot16(0));
			glVertex3v16(inttov16(1), sandHeight[x][y - 1] * 4096, inttov16(-1));

			GFX_TEX_COORD = TEXTURE_PACK(inttot16(0), inttot16(0));
			glVertex3v16(inttov16(-1), sandHeight[x - 1][y - 1] * 4096, inttov16(-1));

			GFX_TEX_COORD = TEXTURE_PACK(inttot16(0), inttot16(128));
			glVertex3v16(inttov16(-1), sandHeight[x - 1][y] * 4096, inttov16(1));

			glPopMatrix(1);
		}
	}
	NE_PolyEnd();
}

/**
 * @brief Draw animated water
 *
 */
void DrawWater()
{
	if (clearWater)
		NE_PolyFormat(15, 0, NE_LIGHT_0, NE_CULL_BACK, NE_MODULATION);
	else
		NE_PolyFormat(25, 0, NE_LIGHT_0, NE_CULL_BACK, NE_MODULATION);

	NE_PolyBegin(GL_QUAD);
	NE_MaterialUse(NULL);

	for (int x = 1; x < 10; x++)
	{
		for (int y = 1; y < 10; y++)
		{
			glPushMatrix();

			glTranslatef32(inttov16(x * 2), inttov16(0), inttov16(y * 2));
			glScalef32(inttov16(1), (int)(waveHeight * 4096), inttov16(1));

			GFX_COLOR = water[x][y].color;
			glVertex3v16(inttov16(1), water[x][y].intHeight, inttov16(1));

			GFX_COLOR = water[x][y - 1].color;
			glVertex3v16(inttov16(1), water[x][y - 1].intHeight, inttov16(-1));

			GFX_COLOR = water[x - 1][y - 1].color;
			glVertex3v16(inttov16(-1), water[x - 1][y - 1].intHeight, inttov16(-1));

			GFX_COLOR = water[x - 1][y].color;
			glVertex3v16(inttov16(-1), water[x - 1][y].intHeight, inttov16(1));

			glPopMatrix(1);
		}
	}
	NE_PolyEnd();
}

/**
 * @brief Update water points
 *
 */
void UpdateWater()
{
	needUpdateWater = !needUpdateWater;
	for (int x = 0; x < 10; x++)
	{
		for (int y = 0; y < 10; y++)
		{
			// Update point every two frames
			needUpdateWater = !needUpdateWater;
			if (needUpdateWater)
			{
				// Perlin noise
				water[x][y].height = noise2((x + waterXOff) / 10.0, (y + waterYOff) / 10.0);
				water[x][y].intHeight = water[x][y].height * 4096;
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
	cubeYPos = water[5][8].height * waveHeight - 0.2;
	NE_PolyColor(COLOR_WHITE); // Set next vertices color

	glPushMatrix();
	// Set cube position
	glTranslatef32(cubeXPos * 4096, cubeYPos * 4096, cubeZPos * 4096);

	for (int x = 0; x < CUBE_VERTEX_COUNT; x++)
	{
		GFX_TEX_COORD = cubeUv[x];
		int xOff = x * 3;
		glVertex3v16(inttov16(cubeVert[xOff]), inttov16(cubeVert[xOff + 1]), inttov16(cubeVert[xOff + 2]));
	}
	glPopMatrix(1);

	NE_PolyEnd();
}

void Draw3DScene(void)
{
	// Set camera for drawing
	NE_CameraUse(Camera);

	// Reset polygons Alpha/Light/Effect
	NE_PolyFormat(31, 0, NE_LIGHT_0, NE_CULL_BACK, NE_MODULATION);

	// Rotate the camera
	angle += 0.003;
	SetCameraPosition();

	// Update water offset
	waterXOff += 0.05;
	waterYOff += 0.05;

	// Update water points
	UpdateWater();

	// Draw sand
	DrawSand();

	// Draw water
	DrawWater();

	// Draw cube
	DrawCube();

	// Draw debug text
	NE_2DViewInit();
	char perfText[40];
	sprintf(perfText, "CPU: %d%%, poly: %d\n", NE_GetCPUPercent(), NE_GetPolygonCount());

	NE_TextPrint(0,		   // Font slot
				 1, 1,	   // Coordinates x(column), y(row)
				 NE_White, // Color
				 perfText);

	char keyText[40];
	sprintf(keyText, "A: Change water style");

	NE_TextPrint(0,		   // Font slot
				 1, 2,	   // Coordinates x(column), y(row)
				 NE_White, // Color
				 keyText);
}