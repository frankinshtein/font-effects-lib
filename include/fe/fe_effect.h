#ifndef FE_EFFECT_H
#define FE_EFFECT_H
#include "fe_export.h"

typedef struct fe_effect
{
    char id[24];

    struct fe_node** nodes;
    int num;
    int size;

} fe_effect;


FONT_EFFECT_EXPORT
struct fe_node* fe_effect_find_node(const fe_effect *ef, int id);

FONT_EFFECT_EXPORT
struct fe_node* fe_effect_find_node_by_type(const fe_effect *ef, int tp);

FONT_EFFECT_EXPORT
struct fe_node* fe_effect_get_node(const fe_effect *ef, int i);

FONT_EFFECT_EXPORT
const char* fe_effect_get_name(const fe_effect *ef);


void fe_effect_free(fe_effect *);

#endif