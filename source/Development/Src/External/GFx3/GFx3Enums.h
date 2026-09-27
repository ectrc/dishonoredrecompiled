// Scaleform GFx 3.3.89 - GENERATED top-level enums, do not edit by hand.
// build/agentBB_gen_gfx3.py from the 2012 Shipping PDB. Nested enums live inside their class in
// GFx3Gen.h; these are the ones declared at file scope.
#ifndef INC_GFX3_ENUMS_H
#define INC_GFX3_ENUMS_H

enum GAME_INSTALL_SCOPE
{
    GIS_NOT_INSTALLED = 0x1,
    GIS_CURRENT_USER = 0x2,
    GIS_ALL_USERS = 0x3
};

enum GFxRenderTextureMode
{
    RTM_Opaque = 0x0,
    RTM_Alpha = 0x1,
    RTM_AlphaComposite = 0x2,
    RTM_MAX = 0x3
};

enum GFxStatFontCache
{
    GFxStatFC_Default = 0xC0,
    GFxStatFC_Mem = 0xC1,
    GFxStatFC_Batch_Mem = 0xC2,
    GFxStatFC_GlyphCache_Mem = 0xC3,
    GFxStatFC_Other_Mem = 0xC4
};

enum GFxStatIME
{
    GFxStatIME_Default = 0x200,
    GFxStatIME_Mem = 0x201
};

enum GFxStatMovieData
{
    GFxStatMD_Default = 0x100,
    GFxStatMD_Mem = 0x101,
    GFxStatMD_CharDefs_Mem = 0x102,
    GFxStatMD_ShapeData_Mem = 0x103,
    GFxStatMD_Tags_Mem = 0x104,
    GFxStatMD_Fonts_Mem = 0x105,
    GFxStatMD_Images_Mem = 0x106,
    GFxStatMD_ActionOps_Mem = 0x107,
    GFxStatMD_Other_Mem = 0x108,
    GFxStatMD_Time = 0x109,
    GFxStatMD_Load_Tks = 0x10A,
    GFxStatMD_Bind_Tks = 0x10B
};

enum GFxStatMovieView
{
    GFxStatMV_Default = 0x140,
    GFxStatMV_Mem = 0x141,
    GFxStatMV_MovieClip_Mem = 0x142,
    GFxStatMV_ActionScript_Mem = 0x143,
    GFxStatMV_Text_Mem = 0x144,
    GFxStatMV_XML_Mem = 0x145,
    GFxStatMV_Other_Mem = 0x146,
    GFxStatMV_Tks = 0x147,
    GFxStatMV_Advance_Tks = 0x148,
    GFxStatMV_Action_Tks = 0x149,
    GFxStatMV_Timeline_Tks = 0x14A,
    GFxStatMV_Input_Tks = 0x14B,
    GFxStatMV_Mouse_Tks = 0x14C,
    GFxStatMV_ScriptCommunication_Tks = 0x14D,
    GFxStatMV_GetVariable_Tks = 0x14E,
    GFxStatMV_SetVariable_Tks = 0x14F,
    GFxStatMV_Invoke_Tks = 0x150,
    GFxStatMV_Display_Tks = 0x151,
    GFxStatMV_Tessellate_Tks = 0x152,
    GFxStatMV_GradientGen_Tks = 0x153
};

enum GFxTimingMode
{
    TM_Game = 0x0,
    TM_Real = 0x1,
    TM_MAX = 0x2
};

enum GHeapId
{
    GHeapId_Global = 0x1,
    GHeapId_MovieDef = 0x2,
    GHeapId_MovieView = 0x3,
    GHeapId_MovieData = 0x4,
    GHeapId_MeshCache = 0x5,
    GHeapId_FontCache = 0x6,
    GHeapId_Images = 0x7,
    GHeapId_OtherHeaps = 0x8,
    GHeapId_HUDHeaps = 0x9
};

enum GStatGroup
{
    GStatGroup_Default = 0x0,
    GStatGroup_Core = 0x10,
    GStatGroup_Renderer = 0x40,
    GStatGroup_RenderGen = 0x80,
    GStatGroup_GFxFontCache = 0xC0,
    GStatGroup_GFxMovieData = 0x100,
    GStatGroup_GFxMovieView = 0x140,
    GStatGroup_GFxRenderCache = 0x180,
    GStatGroup_GFxPlayer = 0x1C0,
    GStatGroup_GFxIME = 0x200,
    GStat_Mem = 0x1,
    GStat_Default_Mem = 0x2,
    GStat_Image_Mem = 0x3,
    GStat_String_Mem = 0x4,
    GStat_Debug_Mem = 0x5,
    GStat_DebugHUD_Mem = 0x6,
    GStat_DebugTracker_Mem = 0x7,
    GStat_StatBag_Mem = 0x8,
    GStatHeap_Start = 0x10,
    GStat_MaxId = 0x1000,
    GStat_EntryCount = 0x200
};

enum GStatRenderer
{
    GStatRender_Default = 0x40,
    GStatRender_Mem = 0x41,
    GStatRender_VMem = 0x42,
    GStatRender_Texture_VMem = 0x43,
    GStatRender_Buffer_VMem = 0x44,
    GStatRender_Counters = 0x45,
    GStatRender_TextureUpload_Cnt = 0x46,
    GStatRender_TextureUpdate_Cnt = 0x47,
    GStatRender_DP_Cnt = 0x48,
    GStatRender_DP_Line_Cnt = 0x49,
    GStatRender_DP_Triangle_Cnt = 0x4A,
    GStatRender_Triangle_Cnt = 0x4B,
    GStatRender_Line_Cnt = 0x4C,
    GStatRender_Mask_Cnt = 0x4D,
    GStatRender_Filter_Cnt = 0x4E
};

#endif // INC_GFX3_ENUMS_H