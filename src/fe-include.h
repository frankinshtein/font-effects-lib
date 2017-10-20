#pragma once
#include "fe/fe_image.h"
#include "ImageData.h"
#include <assert.h>


class Color : public fe_color
{
public:
    Color() {}
    Color(unsigned char R, unsigned char G, unsigned char B, unsigned char A) {
        r = R;
        g = G;
        b = B;
        a = A;
    }
    Color(const fe_color &c) { *static_cast<fe_color*>(this) = c; }
};


#define OX_ASSERT(arg) assert(arg)

