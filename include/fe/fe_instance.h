#ifndef FE_INSTANCE_H
#define FE_INSTANCE_H

typedef void* (*fe_malloc)(unsigned int size);
typedef void(*fe_free)(void* data);

typedef struct fe_instance
{
    fe_malloc alloc;
    fe_free free;
} fe_instance;

#endif