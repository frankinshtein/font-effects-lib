#pragma once
#include "fe-include.h"
#include <stdlib.h>
#include <algorithm>
//#include "ImageData.h"

namespace fe
{
    namespace operations
    {
        //based on memcpy
        void copy(const ImageData& src, const ImageData& dest);

        //based on memmove, could be used for overlapped images, slower than copy
        void move(const ImageData& src, const ImageData& dest);

        void blit(const ImageData& src, const ImageData& dest);
        void blitColored(const ImageData& src, const ImageData& dest, const Color& c);
        void blitPremultiply(const ImageData& src, const ImageData& dest);
        void premultiply(const ImageData& dest);
        void unpremultiply(const ImageData& dest);
        void flipY(const ImageData& src, const ImageData& dest);
        void blend(const ImageData& src, const ImageData& dest);
        void fill(ImageData& dest, const Color& color);

        inline void blend_srcAlpha_invSrcAlpha(const Pixel& pS, Pixel& pD)
        {
            const unsigned int& s = pS.rgba;
            unsigned int& d = pD.rgba;

            unsigned int dst_rb = d        & 0xFF00FF;
            unsigned int dst_ag = (d >> 8) & 0xFF00FF;

            unsigned int src_rb = s        & 0xFF00FF;
            unsigned int src_ag = (s >> 8) & 0xFF00FF;

            unsigned int d_rb = src_rb - dst_rb;
            unsigned int d_ag = src_ag - dst_ag;

            d_rb *= pS.a;
            d_ag *= pS.a;
            d_rb >>= 8;
            d_ag >>= 8;

            const unsigned int rb  = (d_rb + dst_rb)        & 0x00FF00FF;
            const unsigned int ag  = ((d_ag + dst_ag) << 8) & 0xFF00FF00;

            d = rb | ag;
        }


        template <class Op>
        void applyOperation(const Op& op, const ImageData& src, const ImageData& dest);


        class op_fill
        {
        public:
            op_fill() {color.rgba = 0xffffffff;}

            Pixel color;

            template<class Src, class Dest>
            void operator()(const Src& s, Dest& d, const unsigned char* srcData, unsigned char* destData, OPERATOR_ARGS) const
            {
                d.setPixel(destData, color);
            }
        };

        class op_noise
        {
        public:
            op_noise(int v): _v(v) {}

            template<class Src, class Dest>
            void operator()(const Src& srcPixelFormat, Dest& destPixelFormat, const unsigned char* srcData, unsigned char* destData, OPERATOR_ARGS) const
            {
                Pixel p;

                p.r = 255;
                p.g = 255;
                p.b = 255;

                int v = rand() % 1000;
                //p.r = p.g = p.b = p.a = v > 600 ? 255:0;//for add
                p.r = p.g = p.b = p.a = v > _v ? 255 : 0; //for alpha


                destPixelFormat.setPixel(destData, p);
            }

            int _v;
        };

        class op_premultipliedAlpha
        {
        public:
            template<class Src, class Dest>
            void operator()(const Src& srcPixelFormat, Dest& destPixelFormat, const unsigned char* srcData, unsigned char* destData, OPERATOR_ARGS) const
            {
                Pixel p;
                srcPixelFormat.getPixel(srcData, p, OPERATOR_ARGS_PASS);

                //we need correct "snapped" to pixel format alpha
                unsigned char na = destPixelFormat.snap_a(p.a);

                p.r = (p.r * na) / 255;
                p.g = (p.g * na) / 255;
                p.b = (p.b * na) / 255;

                destPixelFormat.setPixel(destData, p);
            }
        };

        class op_unpremultipliedAlpha
        {
        public:
            template<class Src, class Dest>
            void operator()(const Src& srcPixelFormat, Dest& destPixelFormat, const unsigned char* srcData, unsigned char* destData, OPERATOR_ARGS) const
            {
                Pixel p;
                srcPixelFormat.getPixel(srcData, p, OPERATOR_ARGS_PASS);

                //we need correct "snapped" to pixel format alpha
                unsigned char na = destPixelFormat.snap_a(p.a);

                if (na != 0)
                { 
                    p.r = (p.r * 255) / na;
                    p.g = (p.g * 255) / na;
                    p.b = (p.b * 255) / na;
                }
                

                destPixelFormat.setPixel(destData, p);
            }
        };

        class op_blit
        {
        public:
            template<class Src, class Dest>
            void operator()(const Src& srcPixelFormat, Dest& destPixelFormat, const unsigned char* srcData, unsigned char* destData, OPERATOR_ARGS) const
            {
                Pixel p;
                srcPixelFormat.getPixel(srcData, p, OPERATOR_ARGS_PASS);
                destPixelFormat.setPixel(destData, p);
            }
        };

        class op_blit_colored
        {
        public:
            op_blit_colored(const Pixel& clr): color(clr) {}

            template<class Src, class Dest>
            void operator()(const Src& srcPixelFormat, Dest& destPixelFormat, const unsigned char* srcData, unsigned char* destData, OPERATOR_ARGS) const
            {
                Pixel src;
                srcPixelFormat.getPixel(srcData, src, OPERATOR_ARGS_PASS);

                Pixel dest;
                dest.r = (src.r * color.r) / 255;
                dest.g = (src.g * color.g) / 255;
                dest.b = (src.b * color.b) / 255;
                dest.a = (src.a * color.a) / 255;

                destPixelFormat.setPixel(destData, dest);
            }

            Pixel color;
        };



        class op_blend_one_invSrcAlpha
        {
        public:
            template<class Src, class Dest>
            void operator()(const Src& srcPixelFormat, Dest& destPixelFormat, const unsigned char* srcData, unsigned char* destData, OPERATOR_ARGS) const
            {
                Pixel s;
                srcPixelFormat.getPixel(srcData, s, OPERATOR_ARGS_PASS);

                Pixel d;
                destPixelFormat.getPixel(destData, d, OPERATOR_ARGS_PASS);

#define M(v) v < 255 ? v : 255;
                unsigned char ia = 255 - s.a;
                Pixel r;
                r.r = M((d.r * ia) / 255 + s.r);
                r.g = M((d.g * ia) / 255 + s.g);
                r.b = M((d.b * ia) / 255 + s.b);
                r.a = M((d.a * ia) / 255 + s.a);
#undef  M

                destPixelFormat.setPixel(destData, r);
            }
        };


        class op_blend_srcAlpha_invSrcAlpha
        {
        public:
            template<class Src, class Dest>
            void operator()(const Src& srcPixelFormat, Dest& destPixelFormat, const unsigned char* srcData, unsigned char* destData, OPERATOR_ARGS) const
            {
                Pixel pS;
                srcPixelFormat.getPixel(srcData, pS, OPERATOR_ARGS_PASS);

                Pixel pD;
                destPixelFormat.getPixel(destData, pD, OPERATOR_ARGS_PASS);

                blend_srcAlpha_invSrcAlpha(pS, pD);
                destPixelFormat.setPixel(destData, pD);
            }
        };

        class op_blend_subtract
        {
        public:
            template<class Src, class Dest>
            void operator()(const Src& srcPixelFormat, Dest& destPixelFormat, const unsigned char* srcData, unsigned char* destData, OPERATOR_ARGS) const
            {
                Pixel pS;
                srcPixelFormat.getPixel(srcData, pS, OPERATOR_ARGS_PASS);

                Pixel pD;
                destPixelFormat.getPixel(destData, pD, OPERATOR_ARGS_PASS);

                float k = pS.a / float(pD.a);
                pD.r = std::max(0, int(pD.r - pD.r * k));
                pD.g = std::max(0, int(pD.g - pD.g * k));
                pD.b = std::max(0, int(pD.b - pD.b * k));
                pD.a = std::max(0, int(pD.a - pS.a));

                destPixelFormat.setPixel(destData, pD);
            }
        };


        bool check(const ImageData& src, const ImageData& dest);


        template <class Op, class Src, class Dest>
        void applyOperationT(const Op& op, const Src& srcPixelFormat, Dest& destPixelFormat, const ImageData& src, const ImageData& dest)
        {
            if (!check(src, dest))
                return;

            const unsigned char* srcBuffer = (unsigned char*)src.data;
            unsigned char* destBuffer = (unsigned char*)dest.data;

            int w = dest.w;
            int h = dest.h;

            for (int y = 0; y != h; ++y)
            {
                const unsigned char* srcLine = srcBuffer;
                unsigned char* destLine = destBuffer;

                for (int x = 0; x != w; ++x)
                {
                    op(srcPixelFormat, destPixelFormat, srcLine, destLine, x, y);

                    destLine += dest.bytespp;
                    srcLine += src.bytespp;
                }

                srcBuffer += src.pitch;
                destBuffer += dest.pitch;
            }
        }


        template <class Op, class Dest>
        void applyOperationT(const Op& op, Dest& destPixelFormat, const ImageData& dest)
        {
            if (!check(dest, dest))
                return;

            unsigned char* destBuffer = (unsigned char*)dest.data;

            int w = dest.w;
            int h = dest.h;

            for (int y = 0; y != h; ++y)
            {
                unsigned char* destLine = destBuffer;

                for (int x = 0; x != w; ++x)
                {
                    op(destPixelFormat, destPixelFormat, destLine, destLine, x, y);
                    destLine += dest.bytespp;
                }

                destBuffer += dest.pitch;
            }
        }




        template<class T>
        Pixel getPixel4x(const T& pf, const ImageData *src, int X, int Y)
        {
            Pixel p0;
            Pixel p1;
            Pixel p2;
            Pixel p3;

            pf.getPixel(src->getPixelPtr(X, Y), p0, X, Y);

            if (X + 1 < src->w)
                pf.getPixel(src->getPixelPtr(X + 1, Y), p1, X + 1, Y);
            else
                p1.rgba = 0;

            if (Y + 1 < src->h)
                pf.getPixel(src->getPixelPtr(X, Y + 1), p2, X, Y + 1);
            else
                p2.rgba = 0;
            if (((Y + 1) < src->h) && ((X + 1) < src->w))
                pf.getPixel(src->getPixelPtr(X + 1, Y + 1), p3, X + 1, Y + 1);
            else
                p3.rgba = 0;

            Pixel r;
            r.r = (p0.r + p1.r + p2.r + p3.r) / 4;
            r.g = (p0.g + p1.g + p2.g + p3.g) / 4;
            r.b = (p0.b + p1.b + p2.b + p3.b) / 4;
            r.a = (p0.a + p1.a + p2.a + p3.a) / 4;
            return r;
        }




        template <class SrcPixel, class DestPixel>
        void downsample(const SrcPixel &srcPixel, const DestPixel &destPixel, const ImageData *src, const ImageData *dest)
        {
            int w = dest->w;
            int h = dest->h;

            for (int y = 0; y < h; ++y)
            {
                int Y = y * 2;

                for (int x = 0; x < w; ++x)
                {
                    int X = x * 2;

                    Pixel r = getPixel4x(srcPixel, src, X, Y);
                    destPixel.setPixel(dest->getPixelPtr(x, y), r);
                }
            }
        }

#define FORMAT_OP1(format) case FE_IMG_##format: \
        { \
            Pixel##format d; \
            applyOperationT(op, s, d, src, dest); \
        } \
        break;

        template<class Src, class Op>
        void SwitchSrcDestT(const Op& op, const Src& s, const ImageData& src, const ImageData& dest)
        {
#define FORMAT_CASE FORMAT_OP1
            ALL_FORMATS_SWITCH(dest.format);
#undef FORMAT_CASE
        }


#define FORMAT_OP2(format) case FE_IMG_##format: \
        { \
            Pixel##format s; \
            SwitchSrcDestT(op, s, src, dest); \
        } \
        break;


        template <class Op>
        void applyOperation(const Op& op, const ImageData& src, const ImageData& dest)
        {
#define FORMAT_CASE FORMAT_OP2
            ALL_FORMATS_SWITCH(src.format);
#undef FORMAT_CASE
        }


#define FORMAT_OP3(format) case FE_IMG_##format: \
        { \
            Pixel##format d; \
            applyOperationT(op, d, dest); \
        } \
        break;

        template <class Op>
        void applyOperation(const Op& op, const ImageData& dest)
        {
#define FORMAT_CASE FORMAT_OP3
            ALL_FORMATS_SWITCH(dest.format);
#undef FORMAT_CASE
        }
    }
}