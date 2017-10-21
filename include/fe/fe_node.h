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

typedef struct fe_args
{
    int size;
    fe_im base;
    float scale;
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
    int type;
    int x;
    int y;

    int vis_x;
    int vis_y;

    get_node_image get_image;
    struct fe_pin in[FE_MAX_PINS];

    float properties[FE_MAX_PROPS];

    //void *node_data;
    //int size;
} fe_node;


enum fe_node_type
{
    fe_node_type_image = 1,
    fe_node_type_image_fixed = 2,
    fe_node_type_fill = 3,
    fe_node_type_outline = 4,
    fe_node_type_mix = 5,
    fe_node_type_distance_field = 6,
    fe_node_type_subtract = 7,
    fe_node_type_out = 50,

    fe_node_type_custom = 100,
    fe_node_type_stroke_simple = 101,
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

} fe_node_fill;

typedef struct  fe_node_outline
{
    fe_node base;
    float rad;
    float sharpness;
} fe_node_outline;


typedef struct  fe_node_custom
{
    fe_node base;
    int tp;
    float p1;
    float p2;
    float p3;
    float p4;
} fe_node_custom;

typedef struct  fe_node_mix
{
    fe_node base;
} fe_node_mix;

typedef struct  fe_node_out
{
    fe_node base;
    char name[16];
} fe_node_out;

typedef struct  fe_node_distance_field
{
    fe_node base;
    float rad;
} fe_node_distance_field;


typedef struct fe_node_data_fill
{
    fe_grad grad;
} fe_node_data_fill;

typedef struct fe_node_data_distance_field
{
    float rad;
} fe_node_data_distance_field;

typedef struct fe_node_data_out
{
    char* name;
} fe_node_data_out;

FONT_EFFECT_EXPORT fe_node_fill*            fe_node_fill_alloc();
FONT_EFFECT_EXPORT fe_node_image*           fe_node_image_alloc();
FONT_EFFECT_EXPORT fe_node_image_fixed*     fe_node_image_fixed_alloc();
FONT_EFFECT_EXPORT fe_node_mix*             fe_node_mix_alloc();
FONT_EFFECT_EXPORT fe_node*                 fe_node_stroke_simple_alloc();
FONT_EFFECT_EXPORT fe_node*                 fe_node_subtract_alloc();
FONT_EFFECT_EXPORT fe_node_out*             fe_node_out_alloc();
FONT_EFFECT_EXPORT fe_node_outline*         fe_node_outline_alloc();
FONT_EFFECT_EXPORT fe_node_custom*          fe_node_custom_alloc();
FONT_EFFECT_EXPORT fe_node_distance_field*  fe_node_distance_field_alloc();
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
void fe_node_apply(float scale, const fe_im* gl, const fe_node* node, int size, fe_im* res);


/*

typedef struct fe_nodec
{
    int num;
    fe_nodec* left;
};

typedef struct fe_nodec_fill
{
    fe_nodec base;
};
*/

#endif
