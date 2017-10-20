#include <stdio.h>
#include "fe/fe_image.h"
#include <assert.h>
#include <stdlib.h>
#include "ImageDataOperations.h"

using namespace fe;

ImageData* asImage(fe_image *im);
const ImageData* asImage(const fe_image *im);

ImageData* asImage(fe_image *im)
{
    return static_cast<ImageData*>(im);
}

const ImageData* asImage(const fe_image *im)
{
    return static_cast<const ImageData*>(im);
}


void image_free_malloc(fe_image* im)
{
    free(im->data);
    im->data = 0;
}

void fe_image_create(fe_image *im, int w, int h, FE_IMAGE_FORMAT f)
{
    int i = sizeof(*im);
    im->w = w;
    im->h = h;
    im->format = f;
    im->bytespp = getBytesPerPixel(im->format);
    im->pitch = im->bytespp * im->w;
    im->data = (uint8_t*)malloc(im->pitch * im->h);
    im->free = image_free_malloc;
}

void fe_image_free(fe_image *im)
{
    if (im->free)
        im->free(im);
}

fe_image fe_image_get_rect(const fe_image *im, int x, int y, int w, int h)
{
    return asImage(im)->getRect(x, y, w, h);
}

void fe_image_get_rect2(fe_image *p, const fe_image *im, int x, int y, int w, int h)
{
    *p = asImage(im)->getRect(x, y, w, h);
}


void fe_image_copy(const fe_image *src, fe_image *dest)
{
    operations::copy(*asImage(src), *asImage(dest));
}

void fe_image_fill(fe_image *dest, const fe_color *color_)
{
    const Color* color = static_cast<const Color*>(color_);
    operations::fill(*asImage(dest), *color);
}


void fe_image_copy_alloc(const fe_image *src, fe_image *dest)
{
    *dest = *src;
    dest->pitch = dest->w * dest->bytespp;
    dest->data = (uint8_t*)malloc(dest->pitch * dest->h);
    dest->free = image_free_malloc;
    fe_image_copy(src, dest);
}

void fe_image_blit(const fe_image *src, fe_image *dest)
{
    operations::blit(*asImage(src), *asImage(dest));
}

void fe_image_premultiply(fe_image *gl)
{
    operations::premultiply(*asImage(gl));
}


int getBytesPerPixel(FE_IMAGE_FORMAT tf)
{
    switch (tf)
    {
    case FE_IMG_A8:
        return 1;
    case FE_IMG_B8G8R8A8:
    case FE_IMG_R8G8B8A8:
        return 4;
    case FE_IMG_DISTANCE:
        return 8;
    }
    return 0;
}