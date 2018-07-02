#include "ImageData.h"

namespace fe
{
    ImageData::ImageData()//: w(0), h(0), bytespp(0), pitch(0), data(0), format(TF_UNDEFINED)
    {
        w = 0;
        h = 0;
        bytespp = 0;
        pitch = 0;
        data = 0;
        format = TF_UNDEFINED;
        free = 0;
    }

    ImageData::ImageData(const fe_image& b)
    {
        *((fe_image*)this) = b;
    }

    ImageData::ImageData(int W, int H, int Pitch, FE_IMAGE_FORMAT Format, void* Data)//: w(W), h(H), pitch(Pitch), format(Format), data((unsigned char*)Data)
    {
        w = W;
        h = H;
        pitch = Pitch;
        data = (uint8_t*)Data;
        format = Format;
        free = 0;

        bytespp = getBytesPerPixel(Format);
    }

    ImageData::ImageData(const ImageData& b, void* Data)
    {
        *this = ImageData(b.w, b.h, b.pitch, (FE_IMAGE_FORMAT)b.format, Data);
    }

    ImageData::~ImageData()
    {
    }


    ImageData ImageData::getRect(int X, int Y, int W, int H) const
    {
        FE_ASSERT(X >= 0 && X <= w);
        FE_ASSERT(Y >= 0 && Y <= h);
        FE_ASSERT(X + W <= w);
        FE_ASSERT(Y + H <= h);

        void* ptr = (unsigned char*)data + X * bytespp + Y * pitch;
        ImageData buffer(W, H, pitch, format, ptr);

        return buffer;
    }

    ImageData ImageData::getRect(int x, int y) const
    {
        return getRect(x, y, w - x, h - y);
    }

    unsigned char* ImageData::getPixelPtr(int x, int y) const
    {
        return (unsigned char*)data + x * bytespp + y * pitch;
    }
}
