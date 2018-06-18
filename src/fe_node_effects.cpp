#include "fe/fe_node.h"
#include "fe/fe_gradient.h"
#include "fe/fe_image.h"
#include "fe/fe_effect.h"
#include "pixel.h"
#include "ImageDataOperations.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <string.h>


using namespace fe;

void* _fe_alloc(size_t size);
void _fe_free(void *ptr);
void fe_im_empty(fe_im& empty);

int get_pins(const fe_node* node, const fe_args* args, fe_im* res, int Max);

fe_im get_mixed_image(const fe_node* node, const fe_args* args);
    
ImageData* asImage(fe_image* im);
const ImageData* asImage(const fe_image* im);



#ifdef MAX
#undef MAX
#endif

#ifdef MIN
#undef MIN
#endif

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))


template <class T>
class PixelR8G8B8A8_GradApply : public T
{
public:
    fe_apply_grad grad;
    float d;
    float s;

    PixelR8G8B8A8_GradApply(const fe_apply_grad& Grad, float D, float S) : grad(Grad), d(D), s(S)
    {
    }

    ~PixelR8G8B8A8_GradApply()
    {

    }

    void getPixel(GET_PIXEL_ARGS) const
    {
        T::getPixel(GET_PIXEL_ARGS_PASS);

        PixelR8G8B8A8 gp;
        Pixel g;

        const fe_plane& plane = grad.plane;
        const fe_image& image = grad.image;

        float dist = x * plane.a + y * plane.b - plane.d + d;

        int gx = int(dist);
        if (gx >= image.w)
            gx = image.w - 1;
        if (gx < 0)
            gx = 0;

        gp.getPixel(asImage(&image)->getPixelPtr(gx, 0), g, OPERATOR_ARGS_PASS);

        p.r = g.r;
        p.g = g.g;
        p.b = g.b;
        p.a = (p.a * g.a) / 255;
    }

private:
    PixelR8G8B8A8_GradApply(const PixelR8G8B8A8_GradApply&);
    void operator = (const PixelR8G8B8A8_GradApply&);
};



class PixelDist_GradApply
{
public:
    fe_apply_grad grad;
    float s;

    PixelDist_GradApply(const fe_apply_grad& Grad, float S) : grad(Grad), s(S)
    {
    }

    ~PixelDist_GradApply()
    {

    }

    void getPixel(GET_PIXEL_ARGS) const
    {
        //PixelDISTANCE::getPixel(GET_PIXEL_ARGS_PASS);

        PixelR8G8B8A8 gp;
        Pixel g;

        const fe_plane& plane = grad.plane;
        const fe_image& image = grad.image;

        const PixDist* pp = (PixDist*)data;

        float d1 = pp->d1;


        float dist = d1;
        dist = 60.0f * s + (dist) * 3.5f;

        int gx = int(dist * plane.scale);
        if (gx >= image.w)
            gx = image.w - 1;
        if (gx < 0)
            gx = 0;

        gp.getPixel(asImage(&image)->getPixelPtr(gx, 0), g, OPERATOR_ARGS_PASS);

        p.r = g.r;
        p.g = g.g;
        p.b = g.b;

        p.a = g.a;
    }

private:
    PixelDist_GradApply(const PixelDist_GradApply&);
    void operator = (const PixelDist_GradApply&);
};



int getAlphaRad(float dist, float _rad, float _sharp)
{
    int z = 0;
    if (dist < 0)
    {
        z = 255;
    } 
    else
    {
        if (dist < _rad)
        {
            z = 255;
        }
        else
        {
            if (dist < _rad + _sharp)
            {
                float a = (_sharp - (dist - _rad)) / _sharp;
                z = int(a * 255.0f);
            }
        }
    }

    return z;
}

class PixelDist_apply
{
public:
    //fe_apply_grad grad;
    float _s;
    float _rad;
    float _sharp;
    bool inv;

    PixelDist_apply(float rad, float sharp, float S) : _s(S), _rad(rad), _sharp(1.0f / sharp), inv(false)
    {
        /*
        if (rad < 0)
        {
        inv = true;
        rad = -rad;
        }
        */
        _rad = rad * _s;
    }

    ~PixelDist_apply()
    {

    }

    void getPixel(GET_PIXEL_ARGS) const
    {
        const PixDist* pp = (PixDist*)data;


        int z = getAlphaRad(-pp->d1, _rad, _sharp);

        p.r = 255;
        p.g = 255;
        p.b = 255;

        p.a = z;
    }

private:
    PixelDist_apply(const PixelDist_apply&);
    void operator = (const PixelDist_apply&);
};



template<class T>
inline T lerp(T a, T b, float v)
{
    return T(a + (b - a) * v);
}

static void buildSDF(const ImageData& src, float rad, float sharp, bool outer, ImageData& dest, bool dist)
{
    const float DX = 1.0f;
    const float DY = 1.0f;

    int x, y;

    int w = src.w;
    int h = src.h;


    //int cmpWith = 0;

    int off = 0;
    if (src.bytespp == 4)
        off = 3;

    /*
    auto I = [ = ](int x, int y)
    {
    assert(x >= 0 && x < src.w);
    assert(y >= 0 && y < src.h);
    unsigned char v = src.data[x * src.bytespp + y * src.pitch + off];
    return v != 0;
    };
    */
#define I(X, Y) (src.data[(X) * src.bytespp + (Y) * src.pitch + off] != 0)

    /*
    auto V = [ = ](int x, int y)
    {
    assert(x >= 0 && x < src.w);
    assert(y >= 0 && y < src.h);
    unsigned char v = src.data[x * src.bytespp + y * src.pitch + off];
    return v;
    };
    */

#define V(X, Y) (src.data[(X) * src.bytespp + (Y) * src.pitch + off])

    PixDist* p = (PixDist*)(dest.data);

    /*
    P* p = (P*)_fe_alloc(h * w * sizeof(P));

    if (dist)
    {
    dest.data = (uint8_t*)p;
    dest.bytespp = sizeof(P);
    dest.pitch = src.w * dest.bytespp;
    }
    */

    //auto sub = [ = ](int x, int y) {return x + y * w; };

#define SUB(X, Y) ((X) + (Y) * w)

    PixDist zero;
    zero.d1 = 1000.0f;
    zero.d2 = 0.0f;
    zero.x = -1;
    zero.y = -1;

    int size = w * h;

    for (int i = 0; i < size; ++i)
    {
        p[i].d1 = 1000.0;
        p[i].d2 = 0.0f;
        p[i].x = -1;
        p[i].y = -1;
    }

    for (y = 1; y < h - 1; y++)
    {
        for (x = 1; x < w - 1; x++)
        {
            bool t = I(x, y);
            if (t)
                if (I(x - 1, y) != I(x, y) || I(x + 1, y) != I(x, y) ||
                    I(x, y - 1) != I(x, y) || I(x, y + 1) != I(x, y))
                {
                    const int i = SUB(x, y);

                    p[i].d1 = 0;

                    float r = 0.0f;

#define ALG 44

#if ALG == 1
                    int s =
                        V(x - 1, y - 1) + V(x, y - 1) + V(x + 1, y - 1) +
                        V(x - 1, y) + V(x, y) + V(x + 1, y) +
                        V(x - 1, y + 1) + V(x, y + 1) + V(x + 1, y + 1);
                    r = s / 9.0;
#elif ALG == 2
                    int s =
                        V(x, y) + V(x - 1, y) + V(x, y - 1) + V(x + 1, y) + V(x, y + 1);
                    r = s / 5.0;
#elif ALG == 3
                    int s =
                        V(x, y) + V(x - 1, y - 1) + V(x - 1, y + 1) + V(x + 1, y + 1) + V(x + 1, y - 1);
                    r = s / 5.0;
#elif ALG == 4

#else
                    r = V(x, y);
#endif
                    p[i].d2 = (255 - r) / 255.0f;
                    p[i].x = x;
                    p[i].y = y;
                }
        }
    }

    const float dxy = sqrtf(2.0);

#define _check(X,Y,Delta)                             \
i1=SUB((X),(Y));                              \
if (p[i1].d1 + (Delta) < p[i2].d1) {          \
    p[i2] = p[i1];                            \
    float  q1 = p[i1].d2;                     \
    float& q2 = p[i2].d2;                     \
    if (q2 == 0 || q2 > q1) q2=q1;            \
    q2=q1;                                    \
    const float deltaX = float(p[i1].x - x);       \
    const float deltaY = float(p[i1].y - y);       \
    p[i2].d1 = sqrtf(deltaX*deltaX + deltaY*deltaY);  \
}

    //First pass
    for (y = 1; y < h - 1; y++)
    {
        for (x = 1; x < w - 1; x++)
        {
            int i1;
            const int i2 = SUB(x, y);


            _check(x - 1, y, DX);
            _check(x - 1, y - 1, dxy);
            _check(x, y - 1, DY);
            _check(x + 1, y - 1, dxy);

#if 0
            //extra:
            _check(x + 1, y, dx);
            _check(x - 1, y + 1, dxy);
            _check(x, y + 1, dy);
            _check(x + 1, y + 1, dxy);
#endif
        }
    }

    //last pass
    for (y = h - 2; y >= 1; y--)
    {
        for (x = w - 2; x >= 1; x--)
        {
            int i1;
            const int i2 = SUB(x, y);

            _check(x + 1, y, DX);
            _check(x + 1, y + 1, dxy);
            _check(x - 1, y + 1, dxy);
            _check(x, y + 1, DY);

#if 0
            //extra:
            _check(x - 1, y - 1, dxy);
            _check(x, y - 1, dy);
            _check(x + 1, y - 1, dxy);
            _check(x - 1, y, dx);
#endif
        }
    }


    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            const int i = SUB(x, y);
            if (I(x, y) == 0)
            {
                p[i].d1 = -p[i].d1;
            }
        }
    }
#undef I
#undef V
}

template <class T>
class PremultPixel
{
public:
    const T& _t;
    PremultPixel(const T& t) : _t(t) {}

    void getPixel(GET_PIXEL_ARGS) const
    {
        _t.getPixel(GET_PIXEL_ARGS_PASS);
        unsigned char a = p.a;
        p.r = (p.r * a) / 255;
        p.g = (p.g * a) / 255;
        p.b = (p.b * a) / 255;
    }
};

static void create_grad(fe_apply_grad* dest, const fe_grad* gr, int size)
{
    fe_gradient_create(&dest->image, size, 1, gr->colors, gr->colors_pos, gr->colors_num, gr->alpha, gr->alpha_pos, gr->alpha_num);
}



fe_im fe_node_stroke_simple_get_image(const fe_node* node, const fe_args* args)
{
    fe_im mixed = get_mixed_image(node, args);
    // return mixed;

    int nw = mixed.image.w + 2;
    int nh = mixed.image.h + 2;

    int* data = (int*)_fe_alloc(nw * nh * sizeof(int));
    memset(data, 0, nw * nh * sizeof(int));

    int w = mixed.image.w;
    int h = mixed.image.h;

    int off = 0;
    if (mixed.image.bytespp == 4)
        off = 3;

    fe_image src = mixed.image;

    float sp = node->properties_float[fe_const_param_stroke_sharpness];
    bool invert = false;
    if (sp < 0)
    {
        invert = true;
        sp = -sp;
    }

    float f = 1.44f / 4 * sp;
    float z = 0.8f / 4 * sp;
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            unsigned char a = src.data[x * src.bytespp + y * src.pitch + off];
            if (invert)
                a = 255 - a;

            int v = a * 255;


            int qx = x;// +1;
            int qy = y;// +1;


            data[qy * nw + qx] += static_cast<int>(v * f);
            data[qy * nw + qx + 1] += static_cast<int>(v * z);
            data[qy * nw + qx + 2] += static_cast<int>(v * f);

            qy += 1;
            data[qy * nw + qx] += static_cast<int>(v * z);
            data[qy * nw + qx + 1] += static_cast<int>(v * z);
            data[qy * nw + qx + 2] += static_cast<int>(v * z);

            qy += 1;
            data[qy * nw + qx] += static_cast<int>(v * f);
            data[qy * nw + qx + 1] += static_cast<int>(v * z);
            data[qy * nw + qx + 2] += static_cast<int>(v * f);
        }
    }

    /*
    for (int x = 0; x < w; ++x)
    {
    int qx = x;
    int qy = 0;
    int v = 25000;

    data[qy * nw + qx] += v*f;
    data[qy * nw + qx + 1] += v*z;
    data[qy * nw + qx + 2] += v*f;

    qy = 1;

    v = 12000;
    data[qy * nw + qx] += v*f;
    data[qy * nw + qx + 1] += v*z;
    data[qy * nw + qx + 2] += v*f;


    qy = nh - 2;
    data[qy * nw + qx] += v*f;
    data[qy * nw + qx + 1] += v*z;
    data[qy * nw + qx + 2] += v*f;

    v = 25000;
    qy = nh - 1;
    data[qy * nw + qx] += v*f;
    data[qy * nw + qx + 1] += v*z;
    data[qy * nw + qx + 2] += v*f;
    }
    */

    fe_image res;
    fe_image_create(&res, nw, nh, FE_IMG_A8);

    for (int y = 0; y < nh; ++y)
    {
        for (int x = 0; x < nw; ++x)
        {
            int& v = data[x + y * nw];
            v /= 255;
            if (v > 255)
                v = 255;
            unsigned char& a = res.data[x + res.pitch * y];
            a = v;
            //if (0)
            if (invert)
            {
                int tx = x - 1;
                int ty = y - 1;
                if (tx < 0)
                    tx = 0;
                if (ty < 0)
                    ty = 0;
                if (tx >= src.w)
                    tx = src.w - 1;
                if (ty >= src.h)
                    ty = src.h - 1;
                unsigned char t = src.data[tx * src.bytespp + ty * src.pitch + off];
                //if (t != 0)
                //  t = 255;
                a = (v * t) / 255;
            }

        }
    }

    _fe_free(data);

    fe_im im;
    im.image = res;
    im.x = mixed.x - 1;
    im.y = mixed.y - 1;

    fe_image_free(&mixed.image);

    return im;
}



fe_im fe_node_fill_get_image(const fe_node_fill* node, const fe_args* args)
{
    fe_im src = get_mixed_image(&node->base, args);

    fe_im dest;
    dest.x = src.x;
    dest.y = src.y;

    fe_image_create(&dest.image, src.image.w, src.image.h, FE_IMG_R8G8B8A8);


    fe_apply_grad ag;


    if (src.image.format == FE_IMG_DISTANCE)
    {

        create_grad(&ag, &node->grad, args->size);
        ag.plane = node->plane;
        ag.plane.d *= args->scale;


        operations::op_blit op;
        PixelR8G8B8A8 destPixel;

        PixelDist_GradApply srcPixelFill(ag, args->scale);

        //printf("dist apply\n");
        operations::applyOperationT(op, PremultPixel<PixelDist_GradApply>(srcPixelFill), destPixel, *asImage(&src.image), *asImage(&dest.image));
    }
    else
    {

        float sz = args->size / node->plane.scale;
        int gsize = static_cast<int>(sz * 2); //need more colors for good gradient
        float gscale = gsize / sz;

        create_grad(&ag, &node->grad, gsize);
        ag.plane = node->plane;
        ag.plane.d *= args->scale;

        float as = gscale;

        ag.plane.a *= as;
        ag.plane.b *= as;
        ag.plane.d *= as;

        float D = src.x * ag.plane.a + src.y * ag.plane.b;




        operations::op_blit op;
        PixelR8G8B8A8 destPixel;
        if (src.image.bytespp == 1)
        {
            PixelR8G8B8A8_GradApply<PixelA8> srcPixelFill(ag, D, args->scale);
            operations::applyOperationT(op, PremultPixel<PixelR8G8B8A8_GradApply<PixelA8> >(srcPixelFill), destPixel, *asImage(&src.image), *asImage(&dest.image));
        }
        else
        {
            PixelR8G8B8A8_GradApply<PixelR8G8B8A8> srcPixelFill(ag, D, args->scale);
            operations::applyOperationT(op, PremultPixel<PixelR8G8B8A8_GradApply<PixelR8G8B8A8> >(srcPixelFill), destPixel, *asImage(&src.image), *asImage(&dest.image));
        }
    }

    fe_image_free(&src.image);
    fe_image_free(&ag.image);
    //fe_image_safe_tga(&dest.image, "d:/a.tga");
    return dest;
}




fe_im get_mixed_image(const fe_node* node, const fe_args* args)
{
    fe_im res[FE_MAX_PINS];
    int num = get_pins(node, args, res, FE_MAX_PINS);
    if (num == 0)
    {
        fe_im empty;
        fe_im_empty(empty);
        return  empty;
    }


    if (num == 1)
    {
        //TODO, optimize, return already mixed image
        //commented because returned value should be freed from editor
        //  return res[0];

        if (res[0].image.format == FE_IMG_DISTANCE)
            return res[0];
    }

    int r = INT_MIN;
    int bt = INT_MIN;

    int l = INT_MAX;
    int t = INT_MAX;


    for (int i = 0; i < num; ++i)
    {
        fe_im& c = res[i];
        r = MAX(r, c.image.w + c.x);
        bt = MAX(bt, c.image.h + c.y);

        l = MIN(l, c.x);
        t = MIN(t, c.y);
    }

    fe_im dest;
    dest.x = l;
    dest.y = t;

    int w = r - l;
    int h = bt - t;

    fe_image_create(&dest.image, w, h, FE_IMG_R8G8B8A8);

    ImageData destIm = *asImage(&dest.image);
    operations::fill(destIm, Color(0, 0, 0, 0));

    operations::op_blend_one_invSrcAlpha op;

    for (int i = 0; i < num; ++i)
    {
        fe_im& c = res[i];

        ImageData destRC = destIm.getRect(c.x - l, c.y - t, c.image.w, c.image.h);
        operations::applyOperation(op, *asImage(&c.image), destRC);
    }


    for (int i = 0; i < num; ++i)
    {
        fe_im& c = res[i];
        fe_image_free(&c.image);
    }

    return dest;
}


class PixelDist_GradApply4Radial
{
public:
    fe_apply_grad grad;
    float s;
    float radOuter;
    float radInner;

    PixelDist_GradApply4Radial(const fe_apply_grad& Grad, float S, float Outer, float Inner) : grad(Grad), s(S), radOuter(Outer), radInner(Inner)
    {
    }

    ~PixelDist_GradApply4Radial()
    {

    }

    void getPixel(GET_PIXEL_ARGS) const
    {
        PixelR8G8B8A8 gp;
        Pixel g;

        const fe_image& image = grad.image;

        const PixDist* pp = (PixDist*)data;


        float dist = pp->d1 + radOuter;

        int gx = int(dist);
        if (gx >= image.w)
            gx = image.w - 1;
        if (gx < 0)
            gx = 0;

        gp.getPixel(asImage(&image)->getPixelPtr(gx, 0), g, OPERATOR_ARGS_PASS);
        int a1 = getAlphaRad(-pp->d1, radOuter, 1.0f);
        int a2 = getAlphaRad(pp->d1, radInner, 1.0f);

        p.r = g.r;
        p.g = g.g;
        p.b = g.b;

        p.a = g.a * a1  * a2 / 255 / 255;
    }

private:
    PixelDist_GradApply4Radial(const PixelDist_GradApply4Radial&);
    void operator = (const PixelDist_GradApply4Radial&);
};


fe_im fe_node_fill_radial_get_image(const fe_node_fill_radial* node, const fe_args* args)
{
    fe_im src = get_mixed_image(&node->base, args);

    fe_im dest;
    dest.x = src.x;
    dest.y = src.y;

    fe_image_create(&dest.image, src.image.w, src.image.h, FE_IMG_R8G8B8A8);


    fe_apply_grad ag;

    const float *props = node->base.properties_float;

    float outer = props[fe_const_param_fill_radial_rad_outer] * args->scale;
    float inner = props[fe_const_param_fill_radial_rad_inner] * args->scale;

    int sz = outer + inner;
    if (sz < 1)
        sz = 1;
    create_grad(&ag, &node->grad, sz);

    operations::op_blit op;
    PixelR8G8B8A8 destPixel;


    PixelDist_GradApply4Radial srcPixelFill(ag, args->scale, outer, inner);
    operations::applyOperationT(op, PremultPixel<PixelDist_GradApply4Radial>(srcPixelFill), destPixel, *asImage(&src.image), *asImage(&dest.image));


    fe_image_free(&src.image);
    fe_image_free(&ag.image);
    //fe_image_safe_tga(&dest.image, "d:/a.tga");
    return dest;
}


fe_im fe_node_image_get_image(const fe_node_image* node, const fe_args* args)
{
    fe_im im = args->base;
    im.image.free = 0;
    return im;
}

fe_im fe_node_image_fixed_get_image(const fe_node_image_fixed* node, const fe_args* args)
{
    fe_im im = node->im;
    im.image.free = 0;
    return im;
}


fe_im fe_node_out_get_image(const fe_node_image* node, const fe_args* args)
{
    return get_mixed_image(&node->base, args);
}

fe_im fe_node_default_get_image(const fe_node_image* node, const fe_args* args)
{
    return get_mixed_image(&node->base, args);
}


fe_im fe_get_custom_image(const fe_node* node, const fe_args* args);


fe_im fe_node_outline_get_image(const fe_node_outline* node, const fe_args* args)
{
    fe_im src = get_mixed_image(&node->base, args);



    if (src.image.format != FE_IMG_DISTANCE)
    {
        return src;
    }


    fe_im dest;
    dest.x = src.x;
    dest.y = src.y;

    fe_image_create(&dest.image, src.image.w, src.image.h, FE_IMG_R8G8B8A8);


    operations::op_blit op;
    PixelR8G8B8A8 destPixel;

    PixelDist_apply srcPixelFill(node->base.properties_float[fe_const_param_outline_rad], node->base.properties_float[fe_const_param_outline_sharpness], args->scale);

    //printf("dist apply\n");
    operations::applyOperationT(op, PremultPixel<PixelDist_apply>(srcPixelFill), destPixel, *asImage(&src.image), *asImage(&dest.image));



    fe_image_free(&src.image);
    //fe_image_safe_tga(&dest.image, "d:/a.tga");
    return dest;
}

fe_im fe_node_distance_field_get_image(const fe_node_distance_field* node, const fe_args* args)
{
    fe_im src = get_mixed_image(&node->base, args);

    float rad = node->base.properties_float[fe_const_param_distance_field_rad] * sqrtf(args->scale);

    bool outer = rad > 0;
    if (!outer)
        rad = -rad;

    int ew = int(rad) + 1;
    int eh = ew;

    ImageData imSrc;
    fe_image_create(&imSrc, src.image.w + ew * 2, src.image.h + eh * 2, FE_IMG_A8);
    operations::fill(imSrc, Color(0, 0, 0, 0));
    operations::blit(*asImage(&src.image), imSrc.getRect(ew, eh, src.image.w, src.image.h));

    ImageData imDist;
    fe_image_create(&imDist, imSrc.w, imSrc.h, FE_IMG_DISTANCE);
    buildSDF(imSrc, rad, 0, outer, imDist, true);

    fe_image_free(&src.image);
    fe_image_free(&imSrc);

    fe_im res;
    res.image = imDist;
    res.x = src.x - ew;
    res.y = src.y - eh;

    return res;
}

fe_im fe_node_distance_field_auto_get_image(const fe_node_distance_field* node, const fe_args* args)
{
    fe_im src = get_mixed_image(&node->base, args);


    float rad = args->cache.images[node->base.index].df_rad * args->scale;

    bool outer = rad > 0;
    if (!outer)
        rad = -rad;

    int ew = int(rad) + 1;
    int eh = ew;

    ImageData imSrc;
    fe_image_create(&imSrc, src.image.w + ew * 2, src.image.h + eh * 2, FE_IMG_A8);
    operations::fill(imSrc, Color(0, 0, 0, 0));
    operations::blit(*asImage(&src.image), imSrc.getRect(ew, eh, src.image.w, src.image.h));

    ImageData imDist;
    fe_image_create(&imDist, imSrc.w, imSrc.h, FE_IMG_DISTANCE);
    buildSDF(imSrc, rad, 0, outer, imDist, true);

    fe_image_free(&src.image);
    fe_image_free(&imSrc);

    fe_im res;
    res.image = imDist;
    res.x = src.x - ew;
    res.y = src.y - eh;

    return res;
}

fe_im fe_node_subtract_get_image(const fe_node* node, const fe_args* args)
{
    fe_im res[FE_MAX_PINS];
    int num = get_pins(node, args, res, FE_MAX_PINS);
    if (num == 0)
    {
        fe_im empty;
        fe_im_empty(empty);
        return  empty;
    }

    fe_im base = res[0];

    for (int i = 1; i < num; ++i)
    {
        fe_im& c = res[i];

        int r = MIN(base.image.w + base.x, c.image.w + c.x);
        int b = MIN(base.image.h + base.y, c.image.h + c.y);

        int t = MAX(base.y, c.y);
        int l = MAX(base.x, c.x);

        int tw = r - l;
        int th = b - t;

        ImageData destRC = asImage(&base.image)->getRect(l - base.x, t - base.y, tw, th);
        ImageData srcRC = asImage(&c.image)->getRect(l - c.x, t - c.y, tw, th);

        operations::op_blend_subtract op;
        operations::applyOperation(op, srcRC, destRC);
    }


    for (int i = 1; i < num; ++i)
    {
        fe_im& c = res[i];
        fe_image_free(&c.image);
    }

    return base;
}



class PixelDist_Light
{
public:
    fe_apply_grad grad;
    float s;
    float radOuter;
    float radInner;

    PixelDist_Light(const fe_apply_grad& Grad, float S/*, float Outer, float Inner*/) : grad(Grad), s(S)//, radOuter(Outer), radInner(Inner)
    {
    }

    ~PixelDist_Light()
    {

    }

    void getPixel(GET_PIXEL_ARGS) const
    {
        PixelR8G8B8A8 gp;
        Pixel g;

        //const fe_image& image = grad.image;

        const PixDist* pp = (PixDist*)data;


        float dist = pp->d1 + radOuter;
        /*
        int gx = int(dist);
        if (gx >= image.w)
            gx = image.w - 1;
        if (gx < 0)
            gx = 0;

        gp.getPixel(asImage(&image)->getPixelPtr(gx, 0), g, OPERATOR_ARGS_PASS);
        */

        float dx = pp->x - x;
        float dy = pp->y - y;

        float len = sqrt(dx * dx + dy * dy);
        dx /= len;
        dy /= len;

        float lx = 0.707f;
        float ly = 0.707f;

        float c = dx * lx + dy * ly;

        //c = (c + 1) / 2;

        //if (c > 1.0f)
        //    c = 1.0f;

        //int a1 = getAlphaRad(-pp->d1, radOuter, 1.0f);
        //int a2 = getAlphaRad(pp->d1, radInner, 1.0f);

        p.r = c * 255;
        p.g = c * 255;
        p.b = c * 255;

        if (pp->d1 == 0.0f)
            p.a = (1.0f - pp->d2) * 255.0f;
        else if (pp->d1 > 0)
            p.a = 255;
        else p.a = 0;
    }

private:
    PixelDist_Light(const PixelDist_Light&);
    void operator = (const PixelDist_Light&);
};

fe_im fe_node_light_get_image(const fe_node* node, const fe_args* args)
{
    fe_im src = get_mixed_image(node, args);

    fe_im dest;
    dest.x = src.x;
    dest.y = src.y;

    fe_image_create(&dest.image, src.image.w, src.image.h, FE_IMG_R8G8B8A8);


    fe_apply_grad ag;

    //const float *props = node->properties_float;

    //float outer = props[fe_const_param_fill_radial_rad_outer] * args->scale;
    //float inner = props[fe_const_param_fill_radial_rad_inner] * args->scale;

    //create_grad(&ag, &node->grad, outer + inner);


    operations::op_blit op;
    PixelR8G8B8A8 destPixel;


    PixelDist_Light srcPixelFill(ag, args->scale);
    operations::applyOperationT(op, PremultPixel<PixelDist_Light>(srcPixelFill), destPixel, *asImage(&src.image), *asImage(&dest.image));


    fe_image_free(&src.image);
    //fe_image_free(&ag.image);
    //fe_image_safe_tga(&dest.image, "d:/a.tga");
    return dest;
}
