#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

GO(FT_Activate_Size, iFp)
GO(FT_Add_Default_Modules, vFp)
//GO(FT_Add_Module, 
GO(FT_Angle_Diff, lFll)
GO(FT_Atan2, lFll)
GO(FT_Attach_File, iFpp)
GO(FT_Attach_Stream, iFpp)
GO(FT_Bitmap_Convert, iFpppi)
GO(FT_Bitmap_Copy, iFppp)
GO(FT_Bitmap_Done, iFpp)
GO(FT_Bitmap_Embolden, iFppll)
GO(FT_Bitmap_Init, vFp)
GO(FT_Bitmap_New, vFp)
//GO(FT_CeilFix, 
//GO(FT_ClassicKern_Free, 
//GO(FT_ClassicKern_Validate, 
GO(FT_Cos, lFl)
GO(FT_DivFix, lFll)
GO(FT_Done_Face, iFp)
GO(FT_Done_FreeType, iFp)
GO(FT_Done_Glyph, vFp)
GO(FT_Done_Library, iFp)
GO(FT_Done_MM_Var, iFpp)
GO(FT_Done_Size, iFp)
//GO(FT_Face_CheckTrueTypePatents, 
//GO(FT_Face_GetCharsOfVariant, 
GO(FT_Face_GetCharVariantIndex, uFpuu)
//GO(FT_Face_GetCharVariantIsDefault, 
//GO(FT_Face_GetVariantSelectors, 
//GO(FT_Face_GetVariantsOfChar, 
GO(FT_Face_Properties, iFpup)
//GO(FT_Face_SetUnpatentedHinting, 
GO(FT_FloorFix, lFl)
GO(FT_Get_Advance, iFpiup)
GO(FT_Get_Advances, iFpuuip)
//GO(FT_Get_BDF_Charset_ID, 
//GO(FT_Get_BDF_Property, 
GO(FT_Get_Char_Index, uFpL)
GO(FT_Get_Charmap_Index, iFp)
GO(FT_Get_Color_Glyph_Layer, iFpupp)
GO(FT_Get_Color_Glyph_Paint, iFpuip)
GO(FT_Get_Color_Glyph_ClipBox, iFpup)
GO(FT_Get_Colorline_Stops, iFppp)
//GO(FT_Get_CID_From_Glyph_Index, 
//GO(FT_Get_CID_Is_Internally_CID_Keyed, 
//GO(FT_Get_CID_Registry_Ordering_Supplement, 
//GO(FT_Get_CMap_Format, 
//GO(FT_Get_CMap_Language_ID, 
GO(FT_Get_First_Char, LFpp)
GO(FT_Get_Font_Format, pFp)
GO(FT_Get_FSType_Flags, WFp)
//GO(FT_Get_Gasp, 
GO(FT_Get_Glyph, iFpp)
GO(FT_Get_Glyph_Name, iFpupu)
GO(FT_Get_Kerning, iFpuuup)
//GO(FT_Get_MM_Blend_Coordinates, 
GO(FT_Get_MM_Var, iFpp)
GO(FT_Get_Module, pFpp)
//GO(FT_Get_Multi_Master, 
GO(FT_Get_Name_Index, uFpp)
GO(FT_Get_Next_Char, LFpLp)
GO(FT_Get_Paint, iFppip)    // FT_OpaquePaint is a struct with "pi"
GO(FT_Get_Paint_Layers, iFppp)
//GO(FT_Get_PFR_Advance, 
//GO(FT_Get_PFR_Kerning, 
//GO(FT_Get_PFR_Metrics, 
GO(FT_Get_Postscript_Name, pFp)
GO(FT_Get_PS_Font_Info, iFpp)
//GO(FT_Get_PS_Font_Private, 
//GO(FT_Get_PS_Font_Value, 
GO(FT_Get_Renderer, pFpu)
//GO(FT_Get_Sfnt_LangTag, 
GO(FT_Get_Sfnt_Name, iFpip)
GO(FT_Get_Sfnt_Name_Count, uFp)
GO(FT_Get_Sfnt_Table, pFpi)
GO(FT_Get_SubGlyph_Info, iFpuppppp)
GO(FT_Get_Track_Kerning, iFplip)
GO(FT_Get_Transform, vFppp)
GO(FT_Get_TrueType_Engine_Type, uFp)
GO(FT_Get_Var_Blend_Coordinates, iFpup)
//GO(FT_Get_Var_Design_Coordinates, 
GO(FT_Get_WinFNT_Header, iFpp)  // does FT_WinFNT_HeaderRec_ needs alignment?
GO(FT_Get_X11_Font_Format, pFp)
GO(FT_Glyph_Copy, iFpp)
GO(FT_Glyph_Get_CBox, vFpup)
GO(FT_GlyphSlot_Embolden, vFp)
GO(FT_GlyphSlot_Oblique, vFp)
GO(FT_GlyphSlot_Own_Bitmap, iFp)
GO(FT_Glyph_Stroke, iFppC)
GO(FT_Glyph_StrokeBorder, iFppCC)
GO(FT_Glyph_To_Bitmap, iFpupC)
GO(FT_Glyph_Transform, iFppp)
//GO(FT_Gzip_Uncompress, 
//GO(FT_Has_PS_Glyph_Names, 
GO(FT_Init_FreeType, iFp)
GO(FT_Library_SetLcdFilter, iFpu)
GO(FT_Library_SetLcdFilterWeights, iFpp)
GO(FT_Library_Version, vFpppp)
//GO(FT_List_Add, 
//GO(FT_List_Finalize, 
GO(FT_List_Find, pFpp)
//GO(FT_List_Insert, 
//GO(FT_List_Iterate, 
//GO(FT_List_Remove, 
GO(FT_List_Up, vFpp)
GO(FT_Load_Char, iFpLi)
GO(FT_Load_Glyph, iFpui)
GO(FT_Load_Sfnt_Table, iFpLlpp)
GO(FT_Matrix_Invert, iFp)
GO(FT_Matrix_Multiply, vFpp)
GO(FT_MulDiv, lFlll)
GO(FT_MulFix, lFll)
GO(FT_New_Face, iFpplp)
GOM(FT_New_Library, iFEpp)
GO(FT_New_Memory_Face, iFppllp)
GO(FT_New_Size, iFpp)
GOM(FT_Open_Face, iFEpplp)
GO(FT_OpenType_Free, vFpp)
GO(FT_OpenType_Validate, iFpuppppp)
GO(FT_Outline_Check, iFp)
GO(FT_Outline_Copy, iFpp)
GOM(FT_Outline_Decompose, iFEppp)
GO(FT_Outline_Done, iFpp)
//GO(FT_Outline_Done_Internal, 
GO(FT_Outline_Embolden, iFpl)
GO(FT_Outline_EmboldenXY, iFpll)
GO(FT_Outline_Get_BBox, iFpp)
GO(FT_Outline_Get_Bitmap, iFppp)
GO(FT_Outline_Get_CBox, vFpp)
//GO(FT_Outline_GetInsideBorder, 
GO(FT_Outline_Get_Orientation, uFp)
GO(FT_Outline_GetOutsideBorder, uFp)
GO(FT_Outline_New, iFpuip)
//GO(FT_Outline_New_Internal, 
GO(FT_Outline_Render, iFppp)
GO(FT_Outline_Reverse, vFp)
GO(FT_Outline_Transform, vFpp)
GO(FT_Outline_Translate, vFpll)
GO(FT_Palette_Select, iFpWp)
GO(FT_Property_Get, iFpppp)
GO(FT_Property_Set, iFpppp)
GO(FT_Reference_Face, iFp)
//GO(FT_Reference_Library, 
GO(FT_Remove_Module, iFpp)
GO(FT_Render_Glyph, iFpu)
GO(FT_Request_Size, iFpp)
//GO(FT_RoundFix, 
GO(FT_Select_Charmap, iFpi)
GO(FT_Select_Size, iFpi)
GO(FT_Set_Charmap, iFpp)
GO(FT_Set_Char_Size, iFplluu)
//GO(FT_Set_Debug_Hook, 
GO(FT_Set_Default_Properties, vFp)
//GO(FT_Set_MM_Blend_Coordinates, 
GO(FT_Set_MM_Design_Coordinates, iFpup)
GO(FT_Set_Pixel_Sizes, iFpuu)
GO(FT_Set_Renderer, iFppup)
GO(FT_Set_Transform, vFppp)
//GO(FT_Set_Var_Blend_Coordinates, 
GO(FT_Set_Var_Design_Coordinates, iFpup)
GO(FT_Sfnt_Table_Info, iFpupp)
GO(FT_Sin, lFl)
//GO(FT_Stream_OpenBzip2, 
//GO(FT_Stream_OpenGzip, 
//GO(FT_Stream_OpenLZW, 
GO(FT_Stroker_BeginSubPath, iFppC)
GO(FT_Stroker_ConicTo, iFppp)
GO(FT_Stroker_CubicTo, iFpppp)
GO(FT_Stroker_Done, vFp)
GO(FT_Stroker_EndSubPath, iFp)
GO(FT_Stroker_Export, vFpp)
GO(FT_Stroker_ExportBorder, vFpup)
GO(FT_Stroker_GetBorderCounts, iFpupp)
GO(FT_Stroker_GetCounts, iFppp)
GO(FT_Stroker_LineTo, iFpp)
GO(FT_Stroker_New, iFpp)
GO(FT_Stroker_ParseOutline, iFppC)
GO(FT_Stroker_Rewind, vFp)
GO(FT_Stroker_Set, vFpluul)
GO(FT_Tan, lFl)
//GO(FT_TrueTypeGX_Free, 
//GO(FT_TrueTypeGX_Validate, 
GO(FT_Vector_From_Polar, vFpll)
GO(FT_Vector_Length, lFp)
GO(FT_Vector_Polarize, vFppp)
GO(FT_Vector_Rotate, vFpl)
GO(FT_Vector_Transform, vFpp)
GO(FT_Vector_Unit, vFpl)

GO(FTC_CMapCache_Lookup, uFppiu)
GO(FTC_CMapCache_New, iFpp)
GO(FTC_ImageCache_Lookup, iFppupp)
GO(FTC_ImageCache_LookupScaler, iFppLupp)
GO(FTC_ImageCache_New, iFpp)
GO(FTC_Manager_Done, vFp)
GO(FTC_Manager_LookupFace, iFppp)
GO(FTC_Manager_LookupSize, iFppp)
GOM(FTC_Manager_New, iFEpuuLppp)
GO(FTC_Manager_RemoveFaceID, vFpp)
GO(FTC_Manager_Reset, vFp)
GO(FTC_Node_Unref, vFpp)
GO(FTC_SBitCache_Lookup, iFppupp)
GO(FTC_SBitCache_LookupScaler, iFppLupp)
GO(FTC_SBitCache_New, iFpp)

//GO(TT_New_Context, 
//GO(TT_RunIns, 
