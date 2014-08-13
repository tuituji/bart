/*----------------------------------------------------------------------
  File:    texture.h
  Purpose: Stuff concerning textures
           reads PPM files (only in P6 format, i.e., RGB binary)
----------------------------------------------------------------------*/

#ifndef TEXTURE_H
#define TEXTURE_H

#include "material.h"
#include <cstdlib>

struct Texture
{
   int mWidth;
   int mHeight;
   unsigned char *mRGB; /* the size is 3*mWidth*mHeight */

	Color sample(float s, float t);
	~Texture() {
		free(mRGB);
	}
private:
	Color getPixel(int x, int y);
};

inline Color Texture::getPixel(int x, int y)
{
	float r = mRGB[3 *( y * mWidth + x)]/255.0;
	float g = mRGB[3 *( y * mWidth + x) + 1]/255.0;
	float b = mRGB[3 *( y * mWidth + x) + 2]/255.0;
	return Color(r, g, b);
}

struct Texture *viReadPPM(const char *filename);
int viWritePPM(char *filename, int width, int height, unsigned char *pixels);

#endif
