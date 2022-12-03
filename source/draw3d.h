#ifndef DRAW3D_H_ /* Include guard */
#define DRAW3D_H_

#include <NEMain.h>

typedef struct
{
    float height;
    int intHeight;
    int finalHeight;
    u32 color;
} WaterPoint;

typedef struct
{
    float height;
    int intHeight;
} SandPoint;

#define WATER_SIZE 14 // EVEN NUMBER ONLY

extern WaterPoint water[WATER_SIZE][WATER_SIZE];
extern SandPoint sandHeight[WATER_SIZE][WATER_SIZE];

void InitGraphics();
void Draw3DScene(void);
void UpdateWater(bool initFastWater);
void ChangeWaterMode();
void ChangeWaterStyle();

#endif // DRAW3D_H_