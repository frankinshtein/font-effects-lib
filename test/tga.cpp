#include "tga.h"
#include "fe/fe.h"
#include "ImageDataOperations.h"
#include <stdio.h>

void to_bgra(fe::ImageData* src)
{
    fe::ImageData dest = *src;
    dest.format = FE_IMG_B8G8R8A8;
    fe::operations::blit(*src, dest);
    //src->format = dest.format;
    //(fe::ImageData*)
}

bool save_tga(const fe_image *src_, const char* fname)
{
    FILE *fh = fopen(fname, "wb");
    if (!fh)
        return false;

    fe_image dest;
    fe_image_create(&dest, src_->w, src_->h, FE_IMG_B8G8R8A8);

    fe::operations::blit(*(fe::ImageData*)(src_), *(fe::ImageData*)(&dest));
    //fe_image_to_bgra(&dest);

    const fe_image *src = &dest;

    unsigned char header[18] = { 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    header[12] = src->w & 0xFF;
    header[13] = (src->w >> 8) & 0xFF;
    header[14] = (src->h) & 0xFF;
    header[15] = (src->h >> 8) & 0xFF;
    header[16] = src->bytespp * 8;
    fwrite(header, 1, sizeof(header), fh);


    const uint8_t* end = src->data + src->w * src->h * src->bytespp;
    int line = src->w * src->bytespp;

    int h = src->h;

    for (int y = 0; y < h; ++y)
    {
        end -= line;
        if (fwrite(end, 1, line, fh) != line)
            break;
    }

    fe_image_free(&dest);

    fclose(fh);

    return true;
}

bool load_tga(fe_image *dest, const char* fname)
{
    FILE *fh = fopen(fname, "rb");
    if (!fh)
        return false;

    unsigned char header[18];

    fread(header, 1, sizeof(header), fh);

    int f = header[16];

    int w = header[12] + (header[13] << 8);
    int h = header[14] + (header[15] << 8);

    unsigned char flags = header[16];
    //1 2 4 8 16
    bool b = flags & 32;
    // w = 100;
    //h = 100;

    fe_image_create(dest, w, h, f == 8 ? FE_IMG_A8 : FE_IMG_R8G8B8A8);


    int size = dest->w * dest->h * dest->bytespp;

    uint8_t* end = dest->data;// +size;

    int line = dest->w * dest->bytespp;
    int step = line;

    if (b)
    {
        step = -step;
        end += size - line;
    }


    for (int y = 0; y < h; ++y)
    {

        int t = fread(end, 1, line, fh);
        end += step;
    }


    //fe_image_to_bgra(dest);

    fclose(fh);

    return true;
}
