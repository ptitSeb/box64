#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

GO(pango_cairo_context_get_font_options, pFp)
GO(pango_cairo_context_get_resolution, dFp)
//GOM(pango_cairo_context_get_shape_renderer, pFEpp)
GO(pango_cairo_context_set_font_options, vFpp)
GO(pango_cairo_context_set_resolution, vFpd)
//GOM(pango_cairo_context_set_shape_renderer, vFEpppp)
GO(pango_cairo_create_context, pFp)
GO(pango_cairo_create_layout, pFp)
GO(pango_cairo_error_underline_path, vFpdddd)
GO(pango_cairo_fc_font_get_type, iFv)
GO(pango_cairo_fc_font_map_get_type, iFv)
GO(pango_cairo_font_get_scaled_font, pFp)
GO(pango_cairo_font_get_type, LFv)
GO(pango_cairo_font_map_create_context, pFp)
GO(pango_cairo_font_map_get_default, pFv)
GO(pango_cairo_font_map_get_font_type, uFp)
GO(pango_cairo_font_map_get_resolution, dFp)
GO(pango_cairo_font_map_get_type, iFv)
GO(pango_cairo_font_map_new, pFv)
GO(pango_cairo_font_map_new_for_font_type, pFu)
GO(pango_cairo_font_map_set_default, vFp)
GO(pango_cairo_font_map_set_resolution, vFpd)
GO(pango_cairo_glyph_string_path, vFppp)
GO(pango_cairo_layout_line_path, vFpp)
GO(pango_cairo_layout_path, vFpp)
GO(pango_cairo_renderer_get_type, iFv)
GO(pango_cairo_show_error_underline, vFpdddd)
GO(pango_cairo_show_glyph_item, vFppp)
GO(pango_cairo_show_glyph_string, vFppp)
GO(pango_cairo_show_layout, vFpp)
GO(pango_cairo_show_layout_line, vFpp)
GO(pango_cairo_update_context, vFpp)
GO(pango_cairo_update_layout, vFpp)
