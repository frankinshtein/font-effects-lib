#ifndef FONT_GRADIENT_H
#define FONT_GRADIENT_H

#include <stdint.h>
#include "fe_export.h"


typedef struct fe_vec2
{
    int x;
    int y;

} fe_vec2;

#define FE_GRAD_MAX_COLORS  16

typedef struct fe_plane
{
    float a;
    float b;
    float d;
    float scale;
} fe_plane;


typedef struct fe_color
{
    union
    {
        struct
        {
            uint8_t r;
            uint8_t g;
            uint8_t b;
            uint8_t a;
        };
        //uint32_t argb;
        uint32_t value;
    };
} fe_color;


typedef struct fe_grad
{
    fe_color colors[FE_GRAD_MAX_COLORS];
    float colors_pos[FE_GRAD_MAX_COLORS];
    int colors_num;

    unsigned char alpha[FE_GRAD_MAX_COLORS];
    float alpha_pos[FE_GRAD_MAX_COLORS];
    int alpha_num;

    fe_plane plane;
} fe_grad;

FONT_EFFECT_EXPORT
void fe_gradient_create(struct fe_image* im, int width, int height,
                        const struct fe_color* colors, const float* positions, int num,
                        const unsigned char* alpha, const float* alphaPositions, int alphaNum);

#endif //FONT_GRADIENT_H
