#include "fe/fe_image.h"
#include "fe/fe_gradient.h"
#include "fe/fe_node.h"
#include "fe/fe_bundle.h"
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "fe/fe_effect.h"
#include "tga.h"


void* _fe_alloc(size_t size)
{
    return malloc(size);
}

void _fe_free(void *ptr)
{
    free(ptr);
}

void _debug_image_created(fe_image *)
{}

void _debug_image_deleted(fe_image *)
{}

int main()
{
    const int BUFF_SIZE = 100000;
    unsigned char buff[BUFF_SIZE];

    FILE* fh = fopen("example.fe", "rb");
    int size = fread(buff, 1, BUFF_SIZE, fh);
    fclose(fh);

    fe_effect_bundle* bundle = fe_bundle_load(buff, size);
    printf("NUM: %d\n", bundle->num);

    fe_effect *effect = fe_bundle_get_effect_by_name(bundle, "lambda");
    fe_node *out_node = fe_effect_find_node_by_type(effect, fe_node_type_out);


    fe_image src_image;
    load_tga(&src_image, "src.tga");

    fe_im im;
    im.image = src_image;
    im.x = 0;
    im.y = 0;

    fe_im res;
    fe_node_apply2(100, &im, out_node, &res);

    //res.image is PREMULTIPLIED ALPHA
    //unpremultiply it
    fe_image_unpremultiply(&res.image);

    save_tga(&res.image, "dest.tga");

    fe_image_free(&res.image);

    fe_bundle_free(bundle);
    fe_image_free(&src_image);
}
