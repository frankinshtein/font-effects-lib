#ifndef FE_EFFECT_H
#define FE_EFFECT_H
#include "fe_export.h"
#include "fe_node.h"

typedef struct fe_effect
{
    char id[32];

    char text[32];
    char path_font[256];//todo opt    
    char path_back[256];//todo opt
    float distance;

    struct fe_node* out_node;

	short px, py;

    struct fe_node** nodes;
    int num;
    int size;
} fe_effect;


FONT_EFFECT_EXPORT
struct fe_node* fe_effect_find_node(const fe_effect* ef, int id);

FONT_EFFECT_EXPORT
struct fe_node* fe_effect_find_node_by_type(const fe_effect* ef, int FE_NODE_TYPE);

FONT_EFFECT_EXPORT
struct fe_node* fe_effect_get_node(const fe_effect* ef, int i);

FONT_EFFECT_EXPORT
const char* fe_effect_get_name(const fe_effect* ef);

FONT_EFFECT_EXPORT
const char* fe_effect_get_text(const fe_effect* ef);

FONT_EFFECT_EXPORT
const char* fe_effect_get_path_font(const fe_effect* ef);

FONT_EFFECT_EXPORT
const char* fe_effect_get_path_back(const fe_effect* ef);


#endif