#ifndef FONT_EFFECT_BUNDLE_H
#define FONT_EFFECT_BUNDLE_H

#include "fe_export.h"


typedef struct fe_bundle
{
    struct fe_effect* effect;
    int num;
} fe_bundle;

FONT_EFFECT_EXPORT
fe_bundle* fe_bundle_load(const void* data, int size);

FONT_EFFECT_EXPORT
void fe_bundle_free(fe_bundle*);

FONT_EFFECT_EXPORT
struct fe_effect* fe_bundle_get_effect(fe_bundle*, int i);

FONT_EFFECT_EXPORT
struct fe_effect* fe_bundle_get_effect_by_name(fe_bundle*, const char* name);

#endif