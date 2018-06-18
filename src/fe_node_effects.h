#pragma once

fe_im fe_node_image_fixed_get_image(const fe_node_image_fixed* node, const fe_args* args);
fe_im fe_node_stroke_simple_get_image(const fe_node* node, const fe_args* args);
fe_im fe_node_default_get_image(const fe_node_image* node, const fe_args* args);
fe_im fe_node_outline_get_image(const fe_node_outline* node, const fe_args* args);
fe_im fe_node_out_get_image(const fe_node_image* node, const fe_args* args);
fe_im fe_node_distance_field_get_image(const fe_node_distance_field* node, const fe_args* args);
fe_im fe_node_distance_field_auto_get_image(const fe_node_distance_field* node, const fe_args* args);
fe_im fe_node_subtract_get_image(const fe_node* node, const fe_args* args);
fe_im fe_node_light_get_image(const fe_node* node, const fe_args* args);
fe_im fe_node_fill_radial_get_image(const fe_node_fill_radial* node, const fe_args* args);
fe_im fe_node_fill_get_image(const fe_node_fill* node, const fe_args* args);
fe_im fe_node_light_get_image(const fe_node* node, const fe_args* args);