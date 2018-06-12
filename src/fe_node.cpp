#include "fe/fe_node.h"
#include "fe/fe_gradient.h"
#include "fe/fe_image.h"
#include "fe/fe_effect.h"
#include "ImageDataOperations.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <string.h>


void* _fe_alloc(size_t size);
void _fe_free(void *ptr);


using namespace fe;

ImageData* asImage(fe_image* im);
const ImageData* asImage(const fe_image* im);


#define ABS(v) (v < 0 ? - v : v)

#ifdef SAVE_NODES
FONT_EFFECT_EXPORT
bool  fe_image_save_tga(const fe_image* src, const char* fname);
#endif

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

        float c = dist;

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

    PixelDist_apply(float rad, float sharp, float S) : _rad(rad), _sharp(1.0f / sharp), _s(S), inv(false)
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


    int cmpWith = 0;

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
    dest->plane = gr->plane;
}


void fe_im_empty(fe_im& empty)
{
    empty.x = 0;
    empty.y = 0;
    empty.image.w = 0;
    empty.image.h = 0;
    empty.image.data = (uint8_t*)1;
    empty.image.bytespp = 4;
    empty.image.pitch = 4;
    empty.image.format = FE_IMG_R8G8B8A8;
    empty.image.free = 0;
}

fe_im get_image(const fe_node* node, const fe_args* args)
{
    fe_im_cache *cached = args->cache.images;
    if (cached && cached[node->index].image.image.data)
    {
        fe_im res = cached[node->index].image;
        res.image.free = 0;
        return res;
    }
    

    fe_im r = node->get_image(node, args);

    r.x += static_cast<int>(node->x * args->scale);
    r.y += static_cast<int>(node->y * args->scale);

#ifdef SAVE_NODES
    char str[255];
    sprintf(str, "d:/temp/im/%d.tga", node->id);
    fe_image_save_tga(&r.image, str);
#endif

    if (cached)
    {
        cached[node->index].image = r;
        r.image.free = 0;
    }

    return r;
}

fe_im get_image(const fe_node* node, int in, const fe_args* args)
{
    const fe_node* n = node->in[in].node;
    return get_image(n, args);
}

static int get_pins(const fe_node* node, const fe_args* args, fe_im* res, int Max)
{
    int num = 0;
    for (int i = FE_MAX_PINS - 1; i >= 0; --i)
    {
        const fe_node* in = node->in[i].node;
        if (in)
        {
            res[num] = get_image(in, args);
            num++;
        }
    }

    return num;
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


    for (int i = 0; i < num ; ++i)
    {
        fe_im& c = res[i];
        r  = MAX(r, c.image.w + c.x);
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


void fe_node_init(fe_node* node, int tp, get_node_image f)
{
    node->get_image = f;
    node->x = 0;
    node->y = 0;
    node->flags = 0;
    node->type = tp;
    node->effect = 0;
    node->index = -1;
    node->name[0] = 0;
    for (int i = 0; i < FE_MAX_PINS; ++i)
        node->in[i].node = 0;

    static int id = 1;
    node->id = id++;
}

fe_im fe_get_fill(const fe_node_fill* node, const fe_args* args)
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
        ag.plane.d *= args->scale;


        operations::op_blit op;
        PixelR8G8B8A8 destPixel;

        PixelDist_GradApply srcPixelFill(ag, args->scale);

        //printf("dist apply\n");
        operations::applyOperationT(op, PremultPixel<PixelDist_GradApply>(srcPixelFill), destPixel, *asImage(&src.image), *asImage(&dest.image));
    }
    else
    {

        float sz = args->size / node->grad.plane.scale;
        int gsize = static_cast<int>(sz * 2); //need more colors for good gradient
        float gscale = gsize / sz;

        create_grad(&ag, &node->grad, gsize);
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




class PixelDist_GradApply4Radial
{
public:
    fe_apply_grad grad;
    float s;
    float rad;

    PixelDist_GradApply4Radial(const fe_apply_grad& Grad, float S, float Rad) : grad(Grad), s(S), rad(Rad)
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


        float dist = pp->d1 + rad;

        int gx = int(dist);
        if (gx >= image.w)
            gx = image.w - 1;
        if (gx < 0)
            gx = 0;

        gp.getPixel(asImage(&image)->getPixelPtr(gx, 0), g, OPERATOR_ARGS_PASS);
        int a = getAlphaRad(-pp->d1, rad, 1.0f);

        p.r = g.r;
        p.g = g.g;
        p.b = g.b;

        p.a = g.a * a / 255;
    }

private:
    PixelDist_GradApply4Radial(const PixelDist_GradApply4Radial&);
    void operator = (const PixelDist_GradApply4Radial&);
};


fe_im fe_get_fill_radial(const fe_node_fill_radial* node, const fe_args* args)
{
    fe_im src = get_mixed_image(&node->base, args);

    fe_im dest;
    dest.x = src.x;
    dest.y = src.y;

    fe_image_create(&dest.image, src.image.w, src.image.h, FE_IMG_R8G8B8A8);


    fe_apply_grad ag;

    
    float rad = node->base.properties_float[fe_const_param_fill_radial_rad] * args->scale;

    create_grad(&ag, &node->grad, rad * 2);
    //ag.plane.d *= args->scale;


    operations::op_blit op;
    PixelR8G8B8A8 destPixel;

    

    PixelDist_GradApply4Radial srcPixelFill(ag, args->scale, rad);

    //printf("dist apply\n");
    operations::applyOperationT(op, PremultPixel<PixelDist_GradApply4Radial>(srcPixelFill), destPixel, *asImage(&src.image), *asImage(&dest.image));
    

    fe_image_free(&src.image);
    fe_image_free(&ag.image);
    //fe_image_safe_tga(&dest.image, "d:/a.tga");
    return dest;
}


fe_im fe_get_image(const fe_node_image* node, const fe_args* args)
{
    fe_im im = args->base;
    im.image.free = 0;
    return im;
}

fe_im fe_get_image_fixed(const fe_node_image_fixed* node, const fe_args* args)
{
    fe_im im = node->im;
    im.image.free = 0;
    return im;
}


fe_im fe_get_out_image(const fe_node_image* node, const fe_args* args)
{
    return get_mixed_image(&node->base, args);
}

fe_im fe_get_mix_image(const fe_node_image* node, const fe_args* args)
{
    return get_mixed_image(&node->base, args);
}


fe_im fe_get_custom_image(const fe_node* node, const fe_args* args);


fe_im fe_get_outline_image(const fe_node_outline* node, const fe_args* args)
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

fe_im fe_get_distance_field(const fe_node_distance_field* node, const fe_args* args)
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

fe_im fe_get_distance_field_auto(const fe_node_distance_field* node, const fe_args* args)
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

fe_im fe_get_subtract(const fe_node* node, const fe_args* args)
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

        ImageData destRC =  asImage(&base.image)->getRect(l - base.x, t - base.y, tw, th);
        ImageData srcRC  =  asImage(&c.image)->getRect(l - c.x,    t - c.y,    tw, th);

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

fe_im fe_get_stroke_simple(const fe_node* node, const fe_args* args)
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

            int* p = data;
            data[qy * nw + qx]     += static_cast<int>(v * f);
            data[qy * nw + qx + 1] += static_cast<int>(v * z);
            data[qy * nw + qx + 2] += static_cast<int>(v * f);

            qy += 1;
            data[qy * nw + qx]     += static_cast<int>(v * z);
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

fe_node_image* fe_node_image_alloc()
{
    fe_node_image* node = (fe_node_image*)_fe_alloc(sizeof(fe_node_image));
    fe_node_init(&node->base, fe_node_type_source_image, (get_node_image)fe_get_image);
    return node;
}

fe_node_image_fixed* fe_node_image_fixed_alloc()
{
    fe_node_image_fixed* node = (fe_node_image_fixed*)_fe_alloc(sizeof(fe_node_image_fixed));
    fe_node_init(&node->base, fe_node_type_image_fixed, (get_node_image)fe_get_image_fixed);
    node->im.x = 0;
    node->im.y = 0;

    return node;
}

fe_node_mix* fe_node_mix_alloc()
{
    fe_node_mix* node = (fe_node_mix*)_fe_alloc(sizeof(fe_node_mix));
    fe_node_init(&node->base, fe_node_type_mix, (get_node_image)fe_get_mix_image);

    return node;
}

fe_node_out*         fe_node_out_alloc()
{
    fe_node_out* node = (fe_node_out*)_fe_alloc(sizeof(fe_node_out));
    fe_node_init(&node->base, fe_node_type_out, (get_node_image)fe_get_out_image);

    return node;
}

fe_node_outline* fe_node_outline_alloc()
{
    fe_node_outline* node = (fe_node_outline*)_fe_alloc(sizeof(fe_node_outline));
    fe_node_init(&node->base, fe_node_type_outline, (get_node_image)fe_get_outline_image);
    node->base.properties_float[fe_const_param_outline_rad] = 1.0f;
    node->base.properties_float[fe_const_param_outline_sharpness] = 1.0f;

    return node;
}

fe_node_distance_field*  fe_node_distance_field_alloc()
{
    fe_node_distance_field* node = (fe_node_distance_field*)_fe_alloc(sizeof(fe_node_distance_field));
    fe_node_init(&node->base, fe_node_type_distance_field, (get_node_image)fe_get_distance_field);
    node->base.properties_float[fe_const_param_distance_field_rad] = 10.0f;
    return node;
}

fe_node*  fe_node_distance_field_auto_alloc()
{
    fe_node* node = (fe_node*)_fe_alloc(sizeof(fe_node));
    fe_node_init(node, fe_node_type_distance_field_auto, (get_node_image)fe_get_distance_field_auto);
    return node;
}

fe_node*  fe_node_stroke_simple_alloc()
{
    fe_node* node = (fe_node*)_fe_alloc(sizeof(fe_node));
    fe_node_init(node, fe_node_type_stroke_simple, (get_node_image)fe_get_stroke_simple);
    return node;
}

fe_node*                 fe_node_subtract_alloc()
{
    fe_node* node = (fe_node*)_fe_alloc(sizeof(fe_node));
    fe_node_init(node, fe_node_type_subtract, (get_node_image)fe_get_subtract);
    return node;
}


fe_node_fill* fe_node_fill_alloc()
{
    fe_node_fill* node = (fe_node_fill*)_fe_alloc(sizeof(fe_node_fill));
    fe_node_init(&node->base, fe_node_type_fill, (get_node_image)fe_get_fill);

    node->grad.colors_num = 1;
    node->grad.colors[0].value = 0xffffffff;
    node->grad.colors_pos[0] = 0;

    node->grad.alpha_num = 1;
    node->grad.alpha[0] = 255;
    node->grad.alpha_pos[0] = 0;


    node->grad.plane.a = 0;
    node->grad.plane.b = 1;
    node->grad.plane.d = 0;
    node->grad.plane.scale = 1;

    return node;
}

fe_node_fill_radial* fe_node_fill_radial_alloc()
{
    fe_node_fill_radial* node = (fe_node_fill_radial*)_fe_alloc(sizeof(fe_node_fill_radial));
    fe_node_init(&node->base, fe_node_type_fill_radial, (get_node_image)fe_get_fill_radial);

    node->grad.colors_num = 1;
    node->grad.colors[0].value = 0xffffffff;
    node->grad.colors_pos[0] = 0;

    node->grad.alpha_num = 1;
    node->grad.alpha[0] = 255;
    node->grad.alpha_pos[0] = 0;


    node->grad.plane.a = 0;
    node->grad.plane.b = 1;
    node->grad.plane.d = 0;
    node->grad.plane.scale = 1;

    return node;
}

fe_node* fe_node_alloc(int node_type)
{
    fe_node_type nt = (fe_node_type)node_type;
    switch (nt)
    {
        case fe_node_type_source_image:
            return (fe_node*)fe_node_image_alloc();
        case fe_node_type_source_text:
        {
            fe_node* node = (fe_node*)_fe_alloc(sizeof(fe_node));
            fe_node_init(node, nt, (get_node_image)fe_get_image);
            return node;
        }
        case fe_node_type_image_fixed:
            return (fe_node*)fe_node_image_fixed_alloc();
        case fe_node_type_fill:
            return (fe_node*)fe_node_fill_alloc();
        case fe_node_type_fill_radial:
            return (fe_node*)fe_node_fill_radial_alloc();
        case fe_node_type_outline:
            return (fe_node*)fe_node_outline_alloc();
        case fe_node_type_mix:
            return (fe_node*)fe_node_mix_alloc();
        case fe_node_type_distance_field:
            return (fe_node*)fe_node_distance_field_alloc();
        case fe_node_type_distance_field_auto:
            return (fe_node*)fe_node_distance_field_auto_alloc();
        case fe_node_type_out:
            return (fe_node*)fe_node_out_alloc();
        case fe_node_type_stroke_simple:
            return fe_node_stroke_simple_alloc();
        case fe_node_type_subtract:
            return fe_node_subtract_alloc();
        default:
        {
            fe_node* node = (fe_node*)_fe_alloc(sizeof(fe_node));
            fe_node_init(node, nt, (get_node_image)fe_get_mix_image);
            return node;
        }
    }
    return 0;
}

void _fe_node_free(fe_node* node)
{
    _fe_free(node);
}

void _fe_node_connect(const fe_node* src, fe_node* dest, int pin)
{
    dest->in[pin].node = src;

    if (src->type == fe_node_type_distance_field)
    {
        float rad = 0;

        if (dest->type == fe_node_type_outline)
        {
            fe_node_outline *nd = (fe_node_outline *)dest;
            //nd->base
            //onode->base.properties_float[0]
        }

        if (dest->type == fe_node_type_fill)
        {
            fe_node_fill *nd = (fe_node_fill*)dest;
            //nd->
            //onode->base.properties_float[0]
        }


        fe_node_distance_field *dfnode = (fe_node_distance_field *)src;
        //dfnode->rad = std::max(rad, dfnode->rad);
    }
}

int fe_node_get_in_node_id(const fe_node* node, int i)
{
    const fe_pin* pin = &node->in[i];
    if (pin->node)
        return pin->node->id;
    return 0;
}

void update_df_rad(const fe_node* node, fe_args* args)
{
    if (node->type == fe_node_type_fill_radial)
    {
        const fe_node* in = node->in[0].node;
        if (in)
        {
            float &rad = args->cache.images[in->index].df_rad;
            rad = std::max(rad, node->properties_float[fe_const_param_fill_radial_rad]);
        }
    }


    for (int i = 0; i < FE_MAX_PINS; ++i)
    {
        const fe_node* in = node->in[i].node;
        if (in)
            update_df_rad(in, args);        
    }
}

bool fe_node_apply_internal(int font_size, const fe_im* gl, const fe_node* node, fe_im* res, int num)
{
    fe_args args;
    args.size = font_size;
    args.base = *gl;
    args.base.y = font_size - args.base.y;
    args.base.image.free = 0;//can't delete not owner
    args.scale = font_size / 100.0f;

    int size = sizeof(fe_im_cache) * num;
    fe_im_cache *imc = (fe_im_cache*)alloca(size);
    args.cache.images = imc;
    args.cache.num = num;

    memset(imc, 0, size);

    for (int i = 0; i < num; ++i)
        imc[i].df_rad = 0.0f;

    update_df_rad(node, &args);

    *res = get_image(node, &args);
    res->y = font_size - res->y;

    res->image.free = imc[node->index].image.image.free;
    imc[node->index].image.image.free = 0;

    for (int i = 0; i < num; ++i)
        fe_image_free(&imc[i].image.image);

    return true;
}

bool fe_node_apply2(int font_size, const fe_im* gl, const fe_node* node,  fe_im* res)
{
    return fe_node_apply_internal(font_size, gl, node, res, node->effect->num);
}

bool fe_node_apply(
    int font_size,
    int x, int y,
    int w, int h, FE_IMAGE_FORMAT format, int pitch, const void *data,
    const fe_node* node, fe_im* res)
{
    fe_im gl;
    gl.x = x;
    gl.y = y;
    gl.image.data = (uint8_t*)data;
    gl.image.bytespp = getBytesPerPixel(format);
    gl.image.format = format;
    gl.image.free = 0;
    gl.image.w = w;
    gl.image.h = h;
    gl.image.pitch = pitch;

    return fe_node_apply2(font_size, &gl, node, res);
}




template<class T>
Pixel getPixel4x(const T& pf, const ImageData *src, int X, int Y)
{
    Pixel p0;
    Pixel p1;
    Pixel p2;
    Pixel p3;

    pf.getPixel(src->getPixelPtr(X, Y), p0, X, Y);
    pf.getPixel(src->getPixelPtr(X + 1, Y), p1, X + 1, Y);
    pf.getPixel(src->getPixelPtr(X, Y + 1), p2, X, Y + 1);
    pf.getPixel(src->getPixelPtr(X + 1, Y + 1), p3, X + 1, Y + 1);

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
    int w = src->w;
    int h = src->h;

    for (int y = 0; y < h / 2; ++y)
    {
        int Y = y * 2;

        for (int x = 0; x < w / 2; ++x)
        {
            int X = x * 2;

            Pixel r = getPixel4x(srcPixel, src, X, Y);
            destPixel.setPixel(dest->getPixelPtr(x, y), r);
        }
    }
}


void fe_convert_result(fe_im* src, fe_im* dest, FE_IMAGE_FORMAT dest_format, int convert_options)
{
    if (dest_format == TF_UNDEFINED)
        dest_format = src->image.format;


    dest->x = src->x;
    dest->y = src->y;

    ImageData *srcImage = asImage(&src->image);
    ImageData *destImage = asImage(&dest->image);

    if (convert_options & fe_convert_option_downsample2x)
    { 
        fe_image_create(&dest->image, src->image.w/2, src->image.h/2, dest_format);

                
        PixelR8G8B8A8 src_pf;

        if (dest_format == FE_IMG_B8G8R8A8)
        {
            PixelB8G8R8A8 dest_pf;
            downsample(src_pf, dest_pf, srcImage, destImage);
        }

        if (dest_format == FE_IMG_R8G8B8A8)
        {
            PixelR8G8B8A8 dest_pf;
            downsample(src_pf, dest_pf, srcImage, destImage);
        }

        dest->x /= 2;
        dest->y /= 2;
    }
    else
    {
        fe_image_create(&dest->image, src->image.w, src->image.h, dest_format);
        fe::operations::blit(*srcImage, *destImage);
    }

    if (convert_options & fe_convert_option_unpremultiply)
    {
        fe_image_unpremultiply(&dest->image);
    }
}
