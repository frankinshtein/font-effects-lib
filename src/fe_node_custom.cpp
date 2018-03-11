#include "fe/fe_gradient.h"
#include <stdio.h>
#include "fe/fe_image.h"
#include "fe/fe_node.h"
#include <assert.h>
#include <memory.h>
#include <stdlib.h>
#include "ImageDataOperations.h"
#include <math.h>
#include <algorithm>
#include <float.h>
#include <limits.h>

#include "fe/fe_effect.h"
using namespace fe;

ImageData* asImage(fe_image* im);
const ImageData* asImage(const fe_image* im);

fe_im get_mixed_image(const fe_node* node, const fe_args* args);



fe_im fe_get_custom_image(const fe_node* node, const fe_args* args)
{
    fe_im mixed = get_mixed_image(node, args);
    if (mixed.image.w == 0)
        return mixed;
    // return mixed;

    int nw = mixed.image.w + 2;
    int nh = mixed.image.h + 2;

    int* data = (int*)malloc(nw * nh * sizeof(int));
    memset(data, 0, nw * nh * sizeof(int));

    int w = mixed.image.w;
    int h = mixed.image.h;

    int off = 0;
    if (mixed.image.bytespp == 4)
        off = 3;

    fe_image src = mixed.image;


    float f = 1.44f / 4;
    float z = 0.8f / 4;
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            unsigned char a = 255 - src.data[x * src.bytespp + y * src.pitch + off];
            int v = a * 255;

            int qx = x;// +1;
            int qy = y;// +1;

            int* p = data;
            data[qy * nw + qx    ] += static_cast<int>(v * f);
            data[qy * nw + qx + 1] += static_cast<int>(v * z);
            data[qy * nw + qx + 2] += static_cast<int>(v * f);

            qy += 1;
            data[qy * nw + qx    ] += static_cast<int>(v * z);
            data[qy * nw + qx + 1] += static_cast<int>(v * z);
            data[qy * nw + qx + 2] += static_cast<int>(v * z);

            qy += 1;
            data[qy * nw + qx    ] += static_cast<int>(v * f);
            data[qy * nw + qx + 1] += static_cast<int>(v * z);
            data[qy * nw + qx + 2] += static_cast<int>(v * f);
        }
    }


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
            int tx = x - 1;
            int ty = y - 1;
            if (tx >= w)
                tx = w;
            if (ty >= h)
                ty = h;
            if (tx < 0)
                tx = 0;
            if (ty < 0)
                ty = 0;
            unsigned char oa = src.data[tx * src.bytespp + ty * src.pitch + off];
            a = (v * oa) / 255;
        }
    }

    free(data);

    fe_im im;
    im.image = res;
    im.x = mixed.x - 1;
    im.y = mixed.y - 1;

    return im;
}

/*
fe_im fe_get_simple_image(const fe_node_custom *node, const fe_args *args)
{}
*/