#ifndef DRAW3D_H_ /* Include guard */
#define DRAW3D_H_

#include <NEMain.h>

typedef struct
{
    float height;
    int intHeight;
    u32 color;
} WaterPoint;

extern WaterPoint water[10][10];
extern float sandHeight[10][10];
extern bool clearWater;

void InitGraphics();
void Draw3DScene(void);
void UpdateWater();

#endif // DRAW3D_H_