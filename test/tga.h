#pragma once
#include "fe/fe.h"
bool  safe_tga(const fe_image *src, const char* fname);
bool  load_tga(fe_image *dest, const char* fname);