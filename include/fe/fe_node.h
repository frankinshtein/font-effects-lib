#ifndef FONT_EFFECT_H
#define FONT_EFFECT_H

#include <stdint.h>
#include "fe_image.h"
#include "fe_export.h"
#include "fe_gradient.h"


typedef struct fe_apply_grad
{
    fe_plane plane;
    fe_image image;
} fe_apply_grad;


typedef struct fe_im
{
    fe_image image;
    int x;
    int y;
} fe_im;


typedef struct fe_im_cache
{
    fe_im image;
    float df_rad;
} fe_im_cache;



typedef struct fe_nodes_cache
{
    fe_im_cache* images;
    int num;
} fe_nodes_cache;

typedef struct fe_args
{
    int size;
    fe_im base;
    float scale;
    fe_nodes_cache cache;    
} fe_args;

typedef fe_im(*get_node_image)(const struct fe_node*, const struct fe_args*);



typedef struct fe_pin
{
    const struct fe_node* node;
} fe_pin;

#define FE_MAX_PINS 5
#define FE_MAX_PROPS 4

typedef struct fe_node
{
    int id;
    int flags;
    int type;
    int x;
    int y;            

    get_node_image get_image;
    struct fe_pin in[FE_MAX_PINS];
    float properties_float[FE_MAX_PROPS];
    int   properties_int[FE_MAX_PROPS];

    struct fe_effect *effect;
    int index;

    char name[16];    
    int vis_x;
    int vis_y;
} fe_node;


enum fe_node_type
{
    fe_node_type_source_image = 1,
    fe_node_type_image_fixed = 2,
    fe_node_type_fill = 3,
    fe_node_type_outline = 4,
    fe_node_type_mix = 5,
    fe_node_type_distance_field = 6,
    fe_node_type_subtract = 7,
    fe_node_type_stroke_simple = 10,
    fe_node_type_distance_field_auto = 11,
    fe_node_type_fill_radial = 12,
    fe_node_type_out = 50,
    fe_node_type_source_text = 60,
    fe_node_type_out_group = 61,
    fe_node_type_custom = 100,
};


enum fe_const_param
{
    fe_const_param_distance_field_rad = 0,

    fe_const_param_outline_rad = 0,
    fe_const_param_outline_sharpness = 1,

    fe_const_param_df_fill_rad = 0,
    fe_const_param_stroke_sharpness = 0,

    fe_const_param_fill_radial_rad_outer = 0,
    fe_const_param_fill_radial_rad_inner = 1,
};

typedef struct  fe_node_image
{
    fe_node base;
} fe_node_image;


typedef struct  fe_node_image_fixed
{
    fe_node base;
    fe_im im;
} fe_node_image_fixed;


typedef struct  fe_node_fill
{
    fe_node base;

    fe_grad grad;
    fe_plane plane;

} fe_node_fill;

typedef struct  fe_node_fill_radial
{
    fe_node base;
    fe_grad grad;

} fe_node_fill_radial;

typedef struct  fe_node_outline
{
    fe_node base;
} fe_node_outline;

typedef struct  fe_node_mix
{
    fe_node base;
} fe_node_mix;

typedef struct  fe_node_out
{
    fe_node base;
} fe_node_out;

typedef struct  fe_node_distance_field
{
    fe_node base;
    //float rad;
} fe_node_distance_field;


typedef struct fe_node_data_fill
{
    fe_grad grad;
} fe_node_data_fill;


FONT_EFFECT_EXPORT fe_node_fill*            fe_node_fill_alloc();
FONT_EFFECT_EXPORT fe_node_image*           fe_node_image_alloc();
FONT_EFFECT_EXPORT fe_node_image_fixed*     fe_node_image_fixed_alloc();
FONT_EFFECT_EXPORT fe_node_mix*             fe_node_mix_alloc();
FONT_EFFECT_EXPORT fe_node*                 fe_node_stroke_simple_alloc();
FONT_EFFECT_EXPORT fe_node*                 fe_node_subtract_alloc();
FONT_EFFECT_EXPORT fe_node_out*             fe_node_out_alloc();
FONT_EFFECT_EXPORT fe_node_outline*         fe_node_outline_alloc();
FONT_EFFECT_EXPORT fe_node_distance_field*  fe_node_distance_field_alloc();
FONT_EFFECT_EXPORT fe_node*                 fe_node_distance_field_auto_alloc();
FONT_EFFECT_EXPORT fe_node_fill_radial*     fe_node_fill_radial_alloc();


FONT_EFFECT_EXPORT fe_node*                 fe_node_alloc(int node_type);

FONT_EFFECT_EXPORT
void _fe_node_free(fe_node*);

FONT_EFFECT_EXPORT
void _fe_node_connect(const fe_node*, fe_node*, int pin);

#define fe_node_free(node)              _fe_node_free(&node->node)
#define fe_node_connect(src, dest, pin) _fe_node_connect(&(src)->base, &(dest)->base, pin)

FONT_EFFECT_EXPORT int     fe_node_get_in_node_id(const fe_node*, int);

FONT_EFFECT_EXPORT
fe_im fe_get_fill(const fe_node_fill* node, const fe_args* args);

FONT_EFFECT_EXPORT
fe_im fe_get_image(const fe_node_image* node, const fe_args* args);

FONT_EFFECT_EXPORT
bool fe_node_apply(
    int font_size, 
    int x, int y, 
    int w, int h, FE_IMAGE_FORMAT format, int pitch, const void *data,
    const fe_node* node, fe_im* res);


//same as fe_node_apply, but using struct for source glyph image
FONT_EFFECT_EXPORT
bool fe_node_apply2(int font_size, const fe_im* gl, const fe_node* node, fe_im* res);



enum fe_convert_option
{
    fe_convert_option_downsample2x = 1 << 1,
    fe_convert_option_unpremultiply = 1 << 2,
};

FONT_EFFECT_EXPORT
void fe_convert_result(fe_im* src, fe_im* dest, FE_IMAGE_FORMAT dest_format, int convert_options);

#endif
