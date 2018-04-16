#include "fe/fe_gradient.h"
#include <stdio.h>
#include <memory.h>
#include "fe/fe_image.h"
#include <assert.h>
#include "ImageDataOperations.h"

using namespace fe;

ImageData* asImage(fe_image* im);
const ImageData* asImage(const fe_image* im);


float interp(float v)
{
	if (v < 0)
		return 0;
	if (v > 1)
		return 1;

	
	if (v < 0.5)
		v = v * v * 2;
	else
		v = -1 + (4 - 2 * v)*v;
		
	/*
	if (v < 0)
		v = 0;
	else if (v > 1)
		v = 1;
		*/

	return v;
}

void  fe_gradient_create(struct fe_image* im, int width, int height,
                         const struct fe_color* colors_, const float* colorPositions, int num,
                         const unsigned char* alphas, const float* alphaPositions, int alphaNum)
{
    fe_image_create(im, width, height, FE_IMG_R8G8B8A8);

    ImageData* dest = asImage(im);

    const Color* colors = (const Color*)colors_;



    Color colorA;
    Color colorB;
    int dr = 0;
    int dg = 0;
    int db = 0;



    float colorsPosA;
    float colorsPosB;

    const float* colorsPosEnd = colorPositions + num;
    colorA = colorB = *colors;
    colorsPosA = colorsPosB = *colorPositions  * width;
    ++colorPositions;
    ++colors;
    float colorDist = 1;



    unsigned char alphaA, alphaB;
    int da = 0;

    float alphasPosA;
    float alphasPosB;

    const float* alphasPosEnd = alphaPositions + alphaNum;
    alphaA = alphaB = *alphas;
    alphasPosA = alphasPosB = *alphaPositions  * width;
    ++alphaPositions;
    ++alphas;
    float alphaDist = 1;




    unsigned char* data = dest->data;
    for (int x = 0; x < width; ++x)
    {
        while (x > colorsPosB && colorPositions != colorsPosEnd)
        {

            colorA = colorB;
            colorsPosA = colorsPosB;

            colorB = *colors;
            colorsPosB = *colorPositions * width;
            colorDist = colorsPosB - colorsPosA;

            dr = colorB.rgba.r - colorA.rgba.r;
            dg = colorB.rgba.g - colorA.rgba.g;
            db = colorB.rgba.b - colorA.rgba.b;


            ++colorPositions;
            ++colors;
        }

        float d = x - colorsPosA;
        float colorT = interp(d / colorDist);


        while (x > alphasPosB && alphaPositions != alphasPosEnd)
        {
            alphaA = alphaB;
            alphasPosA = alphasPosB;

            alphaB = *alphas;
            alphasPosB = *alphaPositions * width;
            alphaDist = alphasPosB - alphasPosA;

            da = alphaB - alphaA;


            ++alphaPositions;
            ++alphas;
        }


        d = x - alphasPosA;
        float alphaT = interp(d / alphaDist);

        data[0] = (unsigned char)(colorA.rgba.r + dr * colorT);
        data[1] = (unsigned char)(colorA.rgba.g + dg * colorT);
        data[2] = (unsigned char)(colorA.rgba.b + db * colorT);
        data[3] = (unsigned char)(alphaA + da * alphaT);


        data += 4;
    }

    for (int y = 0; y < height; ++y)
    {
        memcpy(im->data + im->pitch * y, im->data, im->pitch);
    }
}
