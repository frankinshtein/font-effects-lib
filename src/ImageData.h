#pragma once
#include "fe-include.h"
#include "pixel.h"
#include "fe/fe_image.h"

namespace fe
{
#define ALL_FORMATS_SWITCH(format) \
    switch(format) \
    { \
            FORMAT_CASE(A8); \
            FORMAT_CASE(R8G8B8A8); \
            FORMAT_CASE(B8G8R8A8); \
            FORMAT_CASE(DISTANCE); \
        default: \
            OX_ASSERT(!"unknown format"); \
    }

    class ImageData: public fe_image
    {
    public:
        ImageData();
        ImageData(int W, int H, int Pitch, FE_IMAGE_FORMAT Format, void* Data = 0);
        ImageData(const ImageData& b, void* Data);
        ~ImageData();

        //ImageData getRect(const Rect& r) const;
        ImageData getRect(int x, int y, int w, int h) const;
        ImageData getRect(int x, int y) const;
        unsigned char* getPixelPtr(int x, int y) const;
    };
}
