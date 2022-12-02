#ifndef NOISE_H_ /* Include guard */
#define NOISE_H_

float grad1(int hash, float x);
float grad2(int hash, float x, float y);
float grad3(int hash, float x, float y, float z);
float grad4(int hash, float x, float y, float z, float t);
float noise1(float x);
float pnoise1(float x, int px);
float noise2(float x, float y);
float pnoise2(float x, float y, int px, int py);
float noise3(float x, float y, float z);
float pnoise3(float x, float y, float z, int px, int py, int pz);
float noise4(float x, float y, float z, float w);
float pnoise4(float x, float y, float z, float w, int px, int py, int pz, int pw);

#endif // NOISE_H_