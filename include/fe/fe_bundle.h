#ifndef FONT_EFFECT_BUNDLE_H
#define FONT_EFFECT_BUNDLE_H

#include "fe_export.h"


typedef struct fe_effect_bundle
{
    struct fe_effect* effect;
    int num;
} fe_effect_bundle;

FONT_EFFECT_EXPORT
fe_effect_bundle* fe_bundle_load(const unsigned char* data, int size);

FONT_EFFECT_EXPORT
void fe_bundle_free(fe_effect_bundle*);

FONT_EFFECT_EXPORT
struct fe_effect* fe_bundle_get_effect(fe_effect_bundle*, int i);

FONT_EFFECT_EXPORT
struct fe_effect* fe_bundle_get_effect_by_name(fe_effect_bundle*, const char* name);

#endif