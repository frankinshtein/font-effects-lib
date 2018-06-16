#define _CRT_SECURE_NO_WARNINGS

#include "fe/fe_bundle.h"
#include "fe/fe_node.h"
#include "fe/fe_effect.h"
#include "fe_parser.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
//#include <windows.h>


/*
FILE *f = fopen("d:/log.txt", "w");

#define LOGF(...) fprintf(f, __VA_ARGS__); fflush(f)
#define LOG(d) fputs(d,f); fflush(f)
*/


void* _fe_alloc(size_t size);
void _fe_free(void *ptr);

template <size_t N>
static void safe_strcpy(char(&dest)[N], const char* src)
{
    for (int i = 0; i < N; ++i)
        dest[i] = src[i];
}

static void safe_strcpy(char *dest, int N, const char* src)
{
    for (int i = 0; i < N; ++i)
        dest[i] = src[i];
}


static void error()
{
    assert(!"fe parse error");
}

#define CHECK_ERR() if (s.error) error()


static void read_token_end_line(fe_state& s)
{
    s.token = s.data;
    while (true)
    {
        if (s.size <= 0)
        {
            //s.error = true;
            return;
        }
        char c = *s.data;
        if (c == '\n')
            break;
        if (c == 0)
            break;
        if (c == '\r')
        {
            *s.data = 0;
            if (s.size > 0)
            {
                s.data++;
                s.size--;
            }
            else
            {
                s.error = true;
                return;
            }

            break;
        }
        s.data++;
        s.size--;
    }

    *s.data = 0;
    s.data++;
    s.size--;
}

static void read_token(fe_state& s)
{
    s.token = s.data;
    while (true)
    {
        if (s.size <= 0)
        {
            //s.error = true;
            return;
        }
        char c = *s.data;
        if (c == ',')
            break;
        if (c == '\n' || c == ':')
            break;
        if (c == 0)
            break;
        if (c == '\r')
        {
            *s.data = 0;
            if (s.size > 0)
            {
                s.data++;
                s.size--;
            }
            else
            {
                s.error = true;
                return;
            }

            break;
        }
        s.data++;
        s.size--;
    }

    *s.data = 0;
    s.data++;
    s.size--;
}

static void read_fixed(fe_state& s, const char* str)
{
    s.token = s.data;
    while (*str)
    {
        char c = *s.data;

        if (s.size <= 0)
        {
            s.error = true;
            return;
        }

        if (c != *str)
        {
            s.error = true;
            break;
        }

        s.data++;
        s.size--;
        str++;
    }
}

static float read_float(fe_state& s)
{
    read_token(s);
    float v = 0;
    sscanf(s.token, "%f", &v);
    return v;
}

static int read_int(fe_state& s)
{
    read_token(s);
    return atoi(s.token);
}

#define READ_FLOAT(state) read_float(state); CHECK_ERR()
#define READ_INT(state)   read_int(s); CHECK_ERR()

/*
static void* read_token_check(fe_state& s, const char* str)
{
    read_token(s);
    CHECK_ERR();

    if (strcmp(s.token, str))
        s.error = true;

    return 0;
}
 */

static void parse_color(const char* str, fe_color* c)
{
    int r, g, b, a;
    sscanf(str, "%02x%02x%02x%02x", &r, &g, &b, &a);

    c->rgba.r = r;
    c->rgba.g = g;
    c->rgba.b = b;
    c->rgba.a = a;
}

static void parse_alpha(const char* str, unsigned char* c)
{
    int a;
    sscanf(str, "%02x", &a);

    *c = a;
}


void fe_node_init(fe_node* node, int tp, get_node_image f);

fe_node* fe_load_node(fe_state& s)
{
    read_token(s);
    CHECK_ERR();

    if (*s.token != '*')
        s.error = 1;
    s.token++;//skip *
    CHECK_ERR();

    fe_node nd;   
    fe_node_init(&nd, 0, 0);

    nd.type = atoi(s.token);
    nd.id = READ_INT(s);
    nd.flags = READ_INT(s);
    nd.x = READ_INT(s);
    nd.y = READ_INT(s);
    nd.vis_x = READ_INT(s);
    nd.vis_y = READ_INT(s);
    

    for (int i = 0; i < FE_MAX_PROPS; ++i)
        nd.properties_float[i] = READ_FLOAT(s);
    for (int i = 0; i < FE_MAX_PROPS; ++i)
        nd.properties_int[i] = READ_INT(s);
    
    read_token(s);
    CHECK_ERR();

    safe_strcpy(nd.name, s.token);


    fe_node* node = 0;
    switch (nd.type)
    {
        case fe_node_type_source_image:
            node = &fe_node_image_alloc()->base;
            break;

        case fe_node_type_image_fixed:
            node = &fe_node_image_fixed_alloc()->base;
            break;

        case fe_node_type_fill:
        {
            fe_node_fill* nf = fe_node_fill_alloc();

            node = &nf->base;

            s.data++;
            fe_grad* grad = &nf->grad;

            //read colors
            int colors = READ_INT(s);
            grad->colors_num = colors;
            for (int i = 0; i < colors; ++i)
            {
                read_token(s);
                CHECK_ERR();

                fe_color* c = &grad->colors[i];

                parse_color(s.token, c);

                grad->colors_pos[i] = READ_FLOAT(s);
            }

            //read alpha
            int alpha_num = READ_INT(s);
            grad->alpha_num = alpha_num;
            for (int i = 0; i < alpha_num; ++i)
            {
                read_token(s);
                CHECK_ERR();

                unsigned char* c = &grad->alpha[i];

                parse_alpha(s.token, c);

                grad->alpha_pos[i] = READ_FLOAT(s);
            }

            fe_plane &plane = nf->plane;
            plane.a = READ_FLOAT(s);
            plane.b = READ_FLOAT(s);
            plane.d = READ_FLOAT(s);
            plane.scale = READ_FLOAT(s);

        } break;

        case fe_node_type_fill_radial:
        {
            fe_node_fill_radial* nf = fe_node_fill_radial_alloc();

            node = &nf->base;

            s.data++;
            fe_grad* grad = &nf->grad;

            //read colors
            int colors = READ_INT(s);
            grad->colors_num = colors;
            for (int i = 0; i < colors; ++i)
            {
                read_token(s);
                CHECK_ERR();

                fe_color* c = &grad->colors[i];

                parse_color(s.token, c);

                grad->colors_pos[i] = READ_FLOAT(s);
            }

            //read alpha
            int alpha_num = READ_INT(s);
            grad->alpha_num = alpha_num;
            for (int i = 0; i < alpha_num; ++i)
            {
                read_token(s);
                CHECK_ERR();

                unsigned char* c = &grad->alpha[i];

                parse_alpha(s.token, c);

                grad->alpha_pos[i] = READ_FLOAT(s);
            }

        } break;

        default:
        {
            node = fe_node_alloc(nd.type);
            break;
        }
    }
        
    nd.get_image = node->get_image;
    memcpy(node, &nd, sizeof(nd));

    return node;
}

static void next_line(fe_state &s)
{
    if (s.size <= 0)
    {
        s.error = true;
        return;
    }
    if (*s.data == '\n')
    {
        s.data++;
        s.size--;
        return;
    }



    if (*s.data == '\r')
    {
        s.data++;
        s.size--;

        if (s.size <= 0)
        {
            s.error = true;
            return;
        }

        if (*s.data == '\n')
        {
            s.data++;
            s.size--;
            return;
        }
    }

    s.error = true;
}

void* fe_load_effect(fe_state& s, fe_effect* effect)
{
    effect->text[0] = 0;
    effect->path_back[0] = 0;
    effect->path_font[0] = 0;
    effect->distance = 1.0f;
	effect->px = -999;
	effect->py = -999;

    read_fixed(s, "#");
    CHECK_ERR();

    read_token(s);
    CHECK_ERR();
    safe_strcpy(effect->id, s.token);

    read_fixed(s, "size:");
    CHECK_ERR();

    effect->size = READ_INT(s);


    while (s.data[0] != '@')
    {
        read_token(s);
        CHECK_ERR();

        if (!strcmp(s.token, "distance"))
        {
            effect->distance = READ_FLOAT(s);
            continue;
        }


		if (!strcmp(s.token, "pos"))
		{
			effect->px = READ_INT(s);
			effect->py = READ_INT(s);
			continue;
		}

        char *param = 0;
        int len = 0;
        if (!strcmp(s.token, "font"))
        {
            len = sizeof(effect->path_font);
            param = effect->path_font;
        }
        if (!strcmp(s.token, "back"))
        {
            len = sizeof(effect->path_back);
            param = effect->path_back;
        }
        if (!strcmp(s.token, "text"))
        {
            len = sizeof(effect->text);
            param = effect->text;
        }

        

        read_token_end_line(s);
        CHECK_ERR();
        safe_strcpy(param, len, s.token);
    }
    
    read_fixed(s, "@nodes");
    CHECK_ERR();

    next_line(s);
    CHECK_ERR();

    int num = 0;
    const char* p = s.data;
    for (int i = 0; i < s.size; ++i)
    {
        if (*p == '*')
            num++;
        if (*p == '@')
            break;
        ++p;
    }

    effect->num = num;
    effect->nodes = (fe_node**)_fe_alloc(sizeof(fe_node*) * num);

    for (int i = 0; i < num; ++i)
    {
        fe_node *node = fe_load_node(s);
        node->effect = effect;
        node->index = i;

        effect->nodes[i] = node;
        CHECK_ERR();        
    }

    read_fixed(s, "@edges");
    CHECK_ERR();

    next_line(s);
    CHECK_ERR();


    num = 0;
    p = s.data;
    for (int i = 0; i < s.size; ++i)
    {
        if (*p == '*')
            num++;
        if (*p == '#')
            break;
        ++p;
    }


    fe_node* srcLast = effect->nodes[0];
    fe_node* destLast = effect->nodes[0];
    while (*s.data == '*')
    {
        s.data++;
        read_token(s);
        CHECK_ERR();

        int sn = atoi(s.token);

        read_token(s);
        CHECK_ERR();

        int dn = atoi(s.token);

        read_token(s);
        CHECK_ERR();

        int dp = atoi(s.token);

        if (srcLast->id != sn)
            srcLast = fe_effect_find_node(effect, sn);

        if (destLast->id != dn)
            destLast = fe_effect_find_node(effect, dn);

        _fe_node_connect(srcLast, destLast, dp);
    }



    return 0;
}

FONT_EFFECT_EXPORT
fe_bundle*  fe_bundle_load(const void* data_, int size)
{
    const unsigned char *data = (const unsigned char *)data_;
    if (size < 4)
        return 0;
    if (!(data[0] == 'F' && data[1] == 'E' && data[2] == 'F'))
        return 0;


    char* copy = (char*)_fe_alloc(size + 2);
    *(copy + size) = 0;
    memcpy(copy, data, size);

    //LOG(copy);

    fe_state s;
    s.token = 0;
    s.data = copy;
    s.size = size + 1;
    s.error = false;

    //next_line(s);

    read_token(s);
    if (strcmp(s.token, "FEF2"))
        return 0;

    char* p = s.data;
    int num_effects = 0;
    for (int i = 0; i < s.size; ++i)
    {
        if (*p == '#')
        {
            //LOGF("AT %d", i);
            num_effects++;
        }
        ++p;
    }

    //LOGF("num %d", num_effects);

    fe_bundle* bundle = (fe_bundle*)_fe_alloc(sizeof(fe_bundle));

    //read_token(s);
    //CHECK_ERR();

    bundle->effect = (fe_effect*)_fe_alloc(sizeof(fe_effect) * num_effects);
    bundle->num = num_effects;

    for (int n = 0; n < num_effects; ++n)
    {
        fe_effect* effect = &bundle->effect[n];
        fe_load_effect(s, effect);
        CHECK_ERR();
    }

    _fe_free(copy);

    return bundle;
}



void fe_effect_free(fe_effect*);

void fe_bundle_free(fe_bundle* bundle)
{
    for (int i = 0; i < bundle->num; ++i)
    {
        fe_effect* ef = &bundle->effect[i];
        fe_effect_free(ef);
    }

    _fe_free(bundle->effect);
    _fe_free(bundle);
}

struct sstate
{
    char* begin;
    char* data;

    int free;
    int capacity;

    char tmp[16];
};

fe_effect* fe_bundle_get_effect(fe_bundle* bundle, int i)
{
    return &bundle->effect[i];
}

fe_effect* fe_bundle_get_effect_by_name(fe_bundle* bundle, const char* name)
{
    int num = bundle->num;
    for (int i = 0; i < num; ++i)
    {
        fe_effect* ef = &bundle->effect[i];
        if (!strcmp(ef->id, name))
            return ef;
    }
    return 0;
}
