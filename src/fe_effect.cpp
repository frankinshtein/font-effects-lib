#include "fe/fe_effect.h"
#include "fe/fe_node.h"
#include <stdlib.h>

void _fe_free(void *ptr);

fe_node* fe_effect_find_node(const fe_effect* ef, int id)
{
    for (int i = 0; i < ef->num; ++i)
    {
        fe_node* node = ef->nodes[i];
        if (node->id == id)
            return node;
    }
    return 0;
}

fe_node* fe_effect_find_node_by_type(const fe_effect* ef, int tp)
{
    for (int i = 0; i < ef->num; ++i)
    {
        fe_node* node = ef->nodes[i];
        if (node->type == tp)
            return node;
    }
    return 0;
}

fe_node* fe_effect_get_node(const fe_effect* ef, int i)
{
    return ef->nodes[i];
}

const char* fe_effect_get_name(const fe_effect* ef)
{
    return ef->id;
}

const char* fe_effect_get_text(const fe_effect* ef)
{
    return ef->text;
}

const char* fe_effect_get_path_font(const fe_effect* ef)
{
    return ef->path_font;
}

const char* fe_effect_get_path_back(const fe_effect* ef)
{
    return ef->path_back;
}

void fe_effect_free(fe_effect* effect)
{
    for (int i = 0; i < effect->num; ++i)
    {
        _fe_free(effect->nodes[i]);
    }

    _fe_free(effect->nodes);
}
