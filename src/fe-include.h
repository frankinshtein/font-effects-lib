#pragma once
#include "fe/fe_image.h"
#include "ImageData.h"
#include <assert.h>


class Color : public fe_color
{
public:
    Color() {}
    Color(unsigned char R, unsigned char G, unsigned char B, unsigned char A)
    {
        rgba.r = R;
        rgba.g = G;
        rgba.b = B;
        rgba.a = A;
    }
    Color(const fe_color& c) { *(fe_color*)(this) = c; }
};


#define OX_ASSERT(arg) assert(arg)

