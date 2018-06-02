#include "fe/fe.h"
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "tga.h"

//these functions should be defined for fe lib
void* _fe_alloc(size_t size)
{
    return malloc(size);
}

void _fe_free(void *ptr)
{
    free(ptr);
}

int main()
{
    FILE* fh = fopen("example.fe", "rb");

    if (fh == 0)//example file not found, copy it to app working directory
        return -1;

    fe_image src_image;
    if (!load_tga(&src_image, "src.tga")) //source example image not found, copy it to app working directory
        return -1;

    const int BUFF_SIZE = 100000;
    unsigned char buff[BUFF_SIZE];

    //load file content to buffer and close
    int size = fread(buff, 1, BUFF_SIZE, fh);
    fclose(fh);

    //parse loaded file
    fe_bundle* bundle = fe_bundle_load(buff, size);
    printf("NUM effects: %d\n", bundle->num);

    //find effect in bundle by name
    fe_effect *effect = fe_bundle_get_effect_by_name(bundle, "sigma");    

    //find final out node
    fe_node *out_node = fe_effect_find_node_by_type(effect, fe_node_type_out);
        
    
    
    
    //all these values could be taken from FREETYPE library: FT_GlyphSlot and FT_Bitmap
    int bitmap_left = 0;
    int bitmap_top = 0;
    int src_width = src_image.w;
    int src_height = src_image.h;
    int pitch = src_image.pitch;
    const void *data = src_image.data;
    FE_IMAGE_FORMAT src_format = FE_IMG_R8G8B8A8; //use FE_IMG_A8 with freetype

    int font_size = 100;//FT_Set_Pixel_Sizes(_face, 0, font_size);

    fe_im result;
    //apply effect from node to source image
    bool ok = fe_node_apply(100,
        bitmap_left, bitmap_top,
        src_width, src_height, 
        src_format, pitch, data,
        out_node, &result);

    //result image is ready, it has format FE_IMG_R8G8B8A8 and premultiplied by alpha

    /*
    //if you want to convert it to B8G8R8A8 format or unpremultiply, use 'fe_convert_result'
    fe_im result2;
    fe_convert_result(&result, &result2, FE_IMG_B8G8R8A8, fe_convert_option_unpremultiply | fe_convert_option_downsample2x);
    */

    //use 'result' for rendering your real text
    //we just save it to file
    save_tga(&result.image, "result.tga");

    //don't forget to delete result.image
    fe_image_free(&result.image);

    fe_bundle_free(bundle);

    fe_image_free(&src_image);
}
