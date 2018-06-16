#pragma once
#include "fe-include.h"

namespace fe
{
    const unsigned char lookupTable4to8[] = {0, 17, 34, 51, 68, 85, 102, 119, 136, 153, 170, 187, 204, 221, 238, 255};
    const unsigned char lookupTable5to8[] = {0, 8, 16, 24, 32, 41, 49, 57, 65, 74, 82, 90, 98, 106, 115, 123, 131, 139, 148, 156, 164, 172, 180, 189, 197, 205, 213, 222, 230, 238, 246, 255};
    const unsigned char lookupTable6to8[] = {0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60, 64, 68, 72, 76, 80, 85, 89, 93, 97, 101, 105, 109, 113, 117, 121, 125, 129, 133, 137, 141, 145, 149, 153, 157, 161, 165, 170, 174, 178, 182, 186, 190, 194, 198, 202, 206, 210, 214, 218, 222, 226, 230, 234, 238, 242, 246, 250, 255};

    struct Pixel
    {
        union
        {
            struct
            {
                unsigned char bytes[4];
            };

            struct
            {
                unsigned char r, g, b, a;
            };

            unsigned int rgba;
        };
    };

    inline Pixel initPixel(unsigned int rgba)
    {
        Pixel p;
        p.rgba = rgba;
        return p;
    }

    inline Pixel initPixel(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
    {
        Pixel p;
        p.r = r;
        p.g = g;
        p.b = b;
        p.a = a;
        return p;
    }

#define GET_PIXEL_ARGS const unsigned char* data, Pixel& p, int x, int y
#define GET_PIXEL_ARGS_PASS   data,  p,  x,  y
#define OPERATOR_ARGS int x, int y
#define OPERATOR_ARGS_PASS x, y

    class PixelA8
    {
    public:
        void getPixel(GET_PIXEL_ARGS) const
        {
            p.a = data[0];
            p.r = p.a;
            p.g = p.a;
            p.b = p.a;
            //p.a = data[0];
        }

        void setPixel(unsigned char* data, const Pixel& p) const
        {
            *data = p.a;
        }

        void copy(const unsigned char* src, unsigned char* dst) const
        {
            *((unsigned char*)dst) = *((unsigned char*)src);
        }

        unsigned char snap_a(unsigned char alpha) const
        {
            return alpha;
        }
    };

    class PixelDISTANCE
    {
    public:
        void getPixel(GET_PIXEL_ARGS) const
        {
            float d1 = *(float*)data;
            float d2 = *((float*)data + 1);
            int t = int((d1 - d2) * 10);
            int f = t;
            if (f < 0)
                f = -f;
            if (f > 255)
                f = 255;
            int color = 255 - f;

            if (d1 > 0)
            {
                p.r = color;
                p.g = 0;
                p.b = 0;
            }
            if (d1 < 0)
            {
                p.r = 0;
                p.g = color;
                p.b = 0;
            }

            if (d1 == 0)
            {
                color = int(255 - d2 * 255);
                p.r = color;
                p.g = color;
                p.b = color;
            }
            p.a = 255;
        }

        void setPixel(unsigned char* data, const Pixel& p) const
        {
            *data = p.a;
        }

        void copy(const unsigned char* src, unsigned char* dst) const
        {
            *((unsigned char*)dst) = *((unsigned char*)src);
        }

        unsigned char snap_a(unsigned char alpha) const
        {
            return alpha;
        }
    };

    class PixelL8
    {
    public:
        void getPixel(GET_PIXEL_ARGS) const
        {
            unsigned char color = *data;
            p.r = color;
            p.g = color;
            p.b = color;
            p.a = 255;
        }

        void setPixel(unsigned char* data, const Pixel& p) const
        {
            *data = (p.r + p.g + p.b) / 3;
        }

        void copy(const unsigned char* src, unsigned char* dst) const
        {
            *dst = *src;
        }

        unsigned char snap_a(unsigned char alpha) const
        {
            return 255;
        }
    };


    class PixelR5G6B5
    {
        /*
        in memory: BBBBB_GGGGGG_RRRRR
        in dword:  RRRRR_GGGGGG_BBBBB
        */
    public:
        void getPixel(GET_PIXEL_ARGS) const
        {
            unsigned short color = *((unsigned short*)data);
            p.r = lookupTable5to8[(color & 0xF800) >> 11];
            p.g = lookupTable6to8[(color & 0x7E0) >> 5];
            p.b = lookupTable5to8[(color & 0x1F)];
            p.a = 255;
        }

        void setPixel(unsigned char* data, const Pixel& p) const
        {
            unsigned short* pshort = (unsigned short*)data;
            *pshort = ((p.r >> 3) << 11) | ((p.g >> 2) << 5) | (p.b >> 3);
        }

        void copy(const unsigned char* src, unsigned char* dst) const
        {
            *((unsigned short*)dst) = *((unsigned short*)src);
        }

        unsigned char snap_a(unsigned char alpha) const
        {
            return 255;
        }
    };

    class PixelR8G8B8A8
    {
        /*
        in memory: R8 G8 B8 A8
        in dword: A8 B8 G8 R8
        */

    public:
        void getPixel(GET_PIXEL_ARGS) const
        {
            p.r = data[0];
            p.g = data[1];
            p.b = data[2];
            p.a = data[3];
        }

        void setPixel(unsigned char* data, const Pixel& p) const
        {
            data[0] = p.r;
            data[1] = p.g;
            data[2] = p.b;
            data[3] = p.a;
        }

        void copy(const unsigned char* src, unsigned char* dst) const
        {
            *((unsigned int*)dst) = *((unsigned int*)src);
        }

        unsigned char snap_a(unsigned char alpha) const
        {
            return alpha;
        }
    };

    class PixelB8G8R8A8
    {
        /*
        in memory: B8 G8 R8 A8
        in dword: A8 R8 G8 B8
        */

    public:
        void getPixel(GET_PIXEL_ARGS) const
        {
            p.r = data[2];
            p.g = data[1];
            p.b = data[0];
            p.a = data[3];
        }

        void setPixel(unsigned char* data, const Pixel& p) const
        {
            data[2] = p.r;
            data[1] = p.g;
            data[0] = p.b;
            data[3] = p.a;
        }

        void copy(const unsigned char* src, unsigned char* dst) const
        {
            *((unsigned int*)dst) = *((unsigned int*)src);
        }

        unsigned char snap_a(unsigned char alpha) const
        {
            return alpha;
        }
    };
}
