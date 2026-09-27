// Scaleform GFx 3.3.89 - the concrete character definitions the 31 unported tag loaders produce,
// and the load process the loaders run against. Package CD.
//
// Retail registers one loader per tag code through GFxLoaderImpl::GetTagLoader (2012 0x9badf0), which
// is two flat tables: GFx_SWF_TagLoaderTable indexed 0..0x54 and a second table indexed 1000..1009
// for GFx's own extensions. Both tables were read out of the database rather than reconstructed from
// the SWF specification, and GFxGetTagLoader below is that dump (GFxTagLoaders.cpp, section 1).
//
// What is ported here is every loader that defines a character, which is what makes a dictionary
// entry real instead of a GFxPlaceholderDef:
//
//   GFx_DefineShapeLoader           2012 0xa36850   tags 2, 22, 32, 83
//   GFx_DefineShapeMorphLoader           0xa35710   tags 46, 84
//   GFx_DefineEditTextLoader             0xa272a0   tag 37
//   GFx_DefineTextLoader                 0xa8d500   tags 11, 33
//   GFx_ButtonCharacterLoader            0xa35ac0   tags 7, 34
//   GFx_DefineFontLoader                 0xa357d0   tags 10, 48, 75, 1005
//   GFx_DefineExternalImageLoader        0xa35fc0   tag 1001
//   GFx_DefineExternalImageLoader2       0xa36210   tag 1009
//   GFx_DefineExternalGradientImageLoader 0xa364d0  tag 1003
//   GFx_DefineSubImageLoader             0xa369d0   tag 1008
//   GFx_DefineBitsJpegLoader             0xa35420   tag 6
//   GFx_DefineBitsJpeg2Loader            0xa35560   tag 21
//   GFx_DefineBitsJpeg3Loader            0xa37080   tag 35
//   GFx_DefineBitsLossless2Loader        0xa37260   tags 20, 36
//
// plus the loaders that modify a definition already in the dictionary (GFx_Scale9GridLoader 0xa36680,
// GFx_CSMTextSettings 0xa26ca0, GFx_DefineFontInfoLoader 0xa35960) and the ones that carry file-level
// state (GFx_FileAttributesLoader 0xa352f0, GFx_MetadataLoader 0xa35d90, GFx_DebugIDLoader 0xa35240,
// GFx_SetTabIndexLoader 0xa35b80, GFx_JpegTablesLoader 0xa353b0).
//
// One deliberate deviation, stated here because it is the only one: retail's shape definition is
// GFxConstShapeCharacterDef, whose Read (GFxConstShapeNoStyles::Read, 0xa42ab0) does not decode the
// shape records at all. It copies the raw SWF bit stream into a packed block out of a
// GFxPathAllocator, scans it once to count paths and edges, and writes those two counts into a
// variable-length header; GFxShapeBase::PathsIterator decodes the block lazily at tessellation time.
// The grammar of that scan is the SWF shape-record grammar and is transcribed exactly below, but the
// compacted byte encoding is not reproduced: the paths are decoded once, into the explicit path and
// edge list that the iterator would have handed the tessellator. Nothing here is ABI-visible (we
// replace libgfx rather than call into it) and the observable results - the bounds, the styles, the
// path count and the edge count - are the same, which is what the harness checks.
// DISHONORED(port): 2013 rvas in GFxTagLoaders.cpp and GFxShape.cpp; see agentCD.md.
#ifndef GFX3_CHARACTERDEFS_H
#define GFX3_CHARACTERDEFS_H

#include "GFxPlayer.h"

// ---------------------------------------------------------------------------------------------
// Every colour in a shape record goes through this, and the threshold is measured rather than
// assumed. GFxLoadProcess::ReadRgbaTag (2012 0xa22460, 57 bytes) is
//     if (tagType > 22) ReadRgba(); else ReadRgb();
// so DefineShape (2) and DefineShape2 (22) carry three-byte colours and DefineShape3 (32) and above
// carry four. Using `> 2` instead - which is what the published tag numbering suggests, because
// DefineShape2 is the second shape tag - reads one byte too many for every colour in a DefineShape2
// and misaligns everything after it in the style array. That is a silent corruption: the shape still
// parses, the fill types come out of the wrong bytes, and the only thing that shows is bitmap fills
// whose character id resolves to nothing. It cost 42 fills in DishonoredGame.Note before the harness
// cross-check of agentCD.md 2.2 caught it.
void GFxReadRgbaTag(GFxStream* s, GColor* out, unsigned int tagType);

// ---------------------------------------------------------------------------------------------
// The image-binding hook. A bitmap reference is resolved through the movie definition's
// State_ImageLoader, which is an engine object (FGFxImageLoader, 2013 0x586020); routing it through a
// function pointer the loader installs is what keeps this unit free of any engine header, which is the
// property that lets the whole directory compile with no engine at all (agentBC.md 7).
typedef GPtr<GImageInfoBase> (*GFxImageResolveFn)(class GFxMovieDataDef* dataDef, const char* name);
extern GFxImageResolveFn GFxImageResolveHook;

// ---------------------------------------------------------------------------------------------
// The styles. GFxFillStyle::Read is 2012 0xa90290, GFxLineStyle::Read 0xa907d0 and
// GFxGradientRecord::Read 0xa8dfc0.

class GFxGradientRecord
{
public:
    GFxGradientRecord() : Ratio(0) {}

    void Read(GFxStream* s, unsigned int tagType);                     // 2012 0xa8dfc0

    unsigned char Ratio;
    GColor        Color;
};

// The fill types are the values GFxFillStyle::Read branches on: 0 solid, the 0x10 group gradient
// (0x10 linear, 0x12 radial, 0x13 focal - the body reads the focal point when the type is exactly
// 19) and the 0x40 group bitmap (0x40 tiled, 0x41 clipped, 0x42 and 0x43 their non-smoothed forms).
enum GFxFillType
{
    GFxFill_Solid              = 0x00,
    GFxFill_LinearGradient     = 0x10,
    GFxFill_RadialGradient     = 0x12,
    GFxFill_FocalGradient      = 0x13,
    GFxFill_TiledImage         = 0x40,
    GFxFill_ClippedImage       = 0x41,
    GFxFill_TiledSmoothImage   = 0x42,
    GFxFill_ClippedSmoothImage = 0x43
};

class GFxFillStyle
{
public:
    enum { MaxGradientRecords = 15 };
    enum Flags { Flag_Interpolation = 0x1, Flag_HasAlpha = 0x2 };

    GFxFillStyle()
        : Type(GFxFill_Solid), Flags(0), GradientCount(0), FocalPoint(0.f),
          ImageId(GFxResourceId::InvalidId) {}

    void Read(GFxStream* s, unsigned int tagType);                     // 2012 0xa90290

    bool IsGradient() const { return (Type & 0x10) != 0; }
    bool IsImage() const { return (Type & 0x40) != 0; }

    unsigned char     Type;
    unsigned char     Flags;
    GColor            Color;                 // solid fills
    unsigned int      GradientCount;
    GFxGradientRecord Gradient[MaxGradientRecords];
    float             FocalPoint;            // type 0x13 only
    unsigned int      ImageId;               // the 0x40 group
    GMatrix2D         Matrix;                // gradient and image fills
};

class GFxLineStyle
{
public:
    enum Flags2 { Flag2_HasFill = 0x8, Flag2_HasMiterLimit = 0x20 };

    GFxLineStyle() : Width(0), Flags(0), MiterLimit(0.f), pFill(0) {}
    ~GFxLineStyle() { delete pFill; }

    void Read(GFxStream* s, unsigned int tagType);                     // 2012 0xa907d0

    unsigned short Width;                    // twips
    unsigned short Flags;                    // DefineShape4's StyleParam word
    GColor         Color;
    float          MiterLimit;
    GFxFillStyle*  pFill;                    // DefineShape4 stroke fill (flag 0x8)

private:
    GFxLineStyle(const GFxLineStyle&);
    GFxLineStyle& operator=(const GFxLineStyle&);
};

// ---------------------------------------------------------------------------------------------
// GFxShapeRecord: the decoded SWF SHAPE record, which is retail's GFxConstShapeNoStyles
// (Read 2012 0xa42ab0) with the edge list decoded once instead of the raw bytes kept and walked
// lazily - the deviation this file's header comment states.
//
// Package CB ports the same retail function for the glyph path as GFxConstShapeNoStyles in
// GFxShape.h. The two are the same decompile and one of them should go: this one is here because the
// merge order puts CD before CB, so a CD-only HEAD has to compile on its own. The hand-over in
// agentCD.md says which reader to keep and why (this one follows retail's field order in two places
// where the other does not).

class GFxShapeEdgeCD
{
public:
    GFxShapeEdgeCD() : Cx(0), Cy(0), Ax(0), Ay(0), bCurve(false) {}

    int  Cx, Cy;                             // the quadratic control point; equals the anchor on a line
    int  Ax, Ay;                             // the anchor, i.e. the edge's end point
    bool bCurve;
};

// One path: a run of edges sharing one (fill0, fill1, line) triple. The indices are the 1-based SWF
// indices, so 0 means "no style", which is what AddForTessellation's `index - 1` relies on.
class GFxShapePathCD
{
public:
    GFxShapePathCD()
        : Fill0(0), Fill1(0), Line(0), StartX(0), StartY(0), Edges(0), EdgeCount(0), EdgeCapacity(0) {}
    ~GFxShapePathCD();

    void AddEdge(const GFxShapeEdgeCD& e);

    unsigned int    Fill0, Fill1, Line;
    int             StartX, StartY;
    GFxShapeEdgeCD* Edges;
    unsigned int    EdgeCount;
    unsigned int    EdgeCapacity;

private:
    GFxShapePathCD(const GFxShapePathCD&);
    GFxShapePathCD& operator=(const GFxShapePathCD&);
};

class GFxShapeRecord
{
public:
    GFxShapeRecord()
        : Paths(0), PathCount(0), PathCapacity(0), ShapeCount(0), bTwentyTimesScale(false),
          bCorrupt(false) {}
    ~GFxShapeRecord();

    // 2012 0xa42ab0. styleOwner takes the appended style arrays a StateNewStyles record carries; it
    // is null for a glyph outline, which has no style arrays at all.
    void Read(GFxStream* s, unsigned int tagType, unsigned int endPos,
              class GFxShapeCharacterDef* styleOwner);

    unsigned int GetPathCount() const { return PathCount; }
    unsigned int GetEdgeCount() const;
    const GFxShapePathCD* GetPath(unsigned int i) const { return i < PathCount ? Paths[i] : 0; }
    GRect<int>   ComputeBound() const;                                 // 2012 0xa3eea0

    GFxShapePathCD** Paths;
    unsigned int     PathCount, PathCapacity;
    unsigned int     ShapeCount;             // how many StateNewStyles groups the record carried
    bool             bTwentyTimesScale;      // DefineFont3: the glyph EM square is 1024 * 20 units
    bool             bCorrupt;

private:
    void AddPath(GFxShapePathCD* p);
    GFxShapeRecord(const GFxShapeRecord&);
    GFxShapeRecord& operator=(const GFxShapeRecord&);
};

// ---------------------------------------------------------------------------------------------
// GFxShapeCharacterDef: DefineShape, DefineShape2, DefineShape3, DefineShape4.
//
// Retail's class hierarchy is GFxConstShapeCharacterDef -> GFxConstShapeWithStyles ->
// GFxConstShapeNoStyles, and Read splits exactly on that line: GFxConstShapeWithStyles::Read
// (2012 0xa43610) reads the bounds and the two style arrays and then calls
// GFxConstShapeNoStyles::Read (0xa42ab0) for the record walk.
class GFxShapeCharacterDef : public GFxCharacterDef
{
public:
    GFxShapeCharacterDef(unsigned int tagCode);
    virtual ~GFxShapeCharacterDef();

    virtual GFxCharacter* CreateCharacterInstance(GFxASCharacter* parent, GFxResourceId id,
                                                  GFxMovieDefImpl* defImpl);
    virtual unsigned int GetResourceTypeCode() const { return GFxResource::RT_ShapeDef; }
    virtual const char* GetDefTypeName() const { return "Shape"; }
    // DISHONORED(port): 2012 0xa3cf40 -> 0xa3bb80. Body in GFxDisplay.cpp.
    virtual void Display(GFxDisplayContext& ctx, GFxCharacter* ch);

    void Read(GFxStream* s, unsigned int tagType, unsigned int endPos);  // 2012 0xa43610

    void SetScale9Grid(const GRect<float>& r) { Scale9Grid = r; bHasScale9Grid = true; }

    const GRect<int>& GetBoundsTwips() const { return Bounds; }
    unsigned int GetPathCount() const { return Shape.GetPathCount(); }
    unsigned int GetEdgeCount() const { return Shape.GetEdgeCount(); }
    unsigned int GetFillStyleCount() const { return FillCount; }
    unsigned int GetLineStyleCount() const { return LineCount; }
    const GFxFillStyle* GetFillStyle(unsigned int i) const
        { return i < FillCount ? &Fills[i] : 0; }
    const GFxLineStyle* GetLineStyle(unsigned int i) const
        { return i < LineCount ? LineArray[i] : 0; }

    void ReadFillStyles(GFxStream* s, unsigned int tagType);            // 2012 0xa429c0
    void ReadLineStyles(GFxStream* s, unsigned int tagType);            // 2012 0xa41270

    void AddFill(const GFxFillStyle& f);
    void AddLine(GFxLineStyle* l);

    unsigned int TagCode;
    GRect<int>   Bounds;                     // twips
    GRect<int>   EdgeBounds;                 // DefineShape4 only
    bool         bUsesNonScalingStrokes;
    bool         bUsesScalingStrokes;
    bool         bUsesFillWinding;
    bool         bHasScale9Grid;
    GRect<float> Scale9Grid;

    GFxShapeRecord Shape;                    // the record walk, 2012 0xa42ab0
    GFxFillStyle*  Fills;
    unsigned int   FillCount, FillCapacity;
    GFxLineStyle** LineArray;
    unsigned int   LineCount, LineCapacity;
};

// GFxMorphCharacterDef (2012 Read 0xab1df0): DefineMorphShape and DefineMorphShape2. The two shape
// records share one style list of interpolated pairs, which is why the fill styles are read through
// ReadMorphFillStyle (0xab1370) rather than GFxFillStyle::Read.
class GFxMorphCharacterDef : public GFxCharacterDef
{
public:
    GFxMorphCharacterDef(unsigned int tagCode);
    virtual ~GFxMorphCharacterDef();

    virtual GFxCharacter* CreateCharacterInstance(GFxASCharacter* parent, GFxResourceId id,
                                                  GFxMovieDefImpl* defImpl);
    virtual unsigned int GetResourceTypeCode() const { return GFxResource::RT_ShapeDef; }
    virtual const char* GetDefTypeName() const { return "MorphShape"; }

    void Read(GFxStream* s, unsigned int tagType, unsigned int endPos);  // 2012 0xab1df0

    unsigned int          TagCode;
    GFxShapeCharacterDef* pStart;
    GFxShapeCharacterDef* pEnd;
};

// ---------------------------------------------------------------------------------------------
// GFxEditTextCharacterDef (2012 Read 0xa26190, ctor 0xa27200, InitEmptyTextDef 0xa26720). The flag
// bits are the ones the retail body sets in the word at +80, in the order it reads them; three of
// the fifteen bits are stored inverted, which is noted at the site.
class GFxEditTextCharacterDef : public GFxCharacterDef
{
public:
    enum Flags
    {
        Flag_WordWrap      = 0x0001,
        Flag_Multiline     = 0x0002,
        Flag_Password      = 0x0004,
        Flag_ReadOnly      = 0x0008,
        Flag_AutoSize      = 0x0010,
        Flag_Selectable    = 0x0020,   // the stream bit is NoSelect and is stored inverted
        Flag_Border        = 0x0040,
        Flag_Html          = 0x0080,
        Flag_UseDeviceFont = 0x0100,   // the stream bit is UseOutlines and is stored inverted
        Flag_HasLayout     = 0x0200,
        Flag_UseFlashType  = 0x0400    // set by GFx_CSMTextSettings, 0xa26ca0
    };
    enum Align { Align_Left = 0, Align_Right = 1, Align_Center = 2, Align_Justify = 3 };

    GFxEditTextCharacterDef();

    virtual GFxCharacter* CreateCharacterInstance(GFxASCharacter* parent, GFxResourceId id,
                                                  GFxMovieDefImpl* defImpl);
    virtual unsigned int GetResourceTypeCode() const { return GFxResource::RT_EditTextDef; }
    virtual const char* GetDefTypeName() const { return "EditText"; }

    void Read(GFxStream* s, unsigned int tagType);                     // 2012 0xa26190
    void InitEmptyTextDef();                                           // 2012 0xa26720

    unsigned int   FontId;
    GRect<int>     TextRect;                 // twips
    float          FontHeight;               // twips
    GColor         TextColor;
    unsigned int   MaxLength;
    unsigned int   Alignment;
    float          LeftMargin, RightMargin, Indent, Leading;
    unsigned short Flags;
    char           VariableName[128];
    char           InitialText[512];
};

// GFxStaticTextCharacterDef (2012 Read 0xa8c130): DefineText and DefineText2. No asset in the retail
// cook carries one - every text field is a DefineEditText - so the glyph runs are counted and the
// record walk is verified against the tag length rather than kept, which the report states.
class GFxStaticTextCharacterDef : public GFxCharacterDef
{
public:
    GFxStaticTextCharacterDef() : GlyphCount(0), RecordCount(0) { Matrix.SetIdentity(); }

    virtual GFxCharacter* CreateCharacterInstance(GFxASCharacter* parent, GFxResourceId id,
                                                  GFxMovieDefImpl* defImpl);
    virtual unsigned int GetResourceTypeCode() const { return GFxResource::RT_TextDef; }
    virtual const char* GetDefTypeName() const { return "Text"; }

    void Read(GFxStream* s, unsigned int tagType, unsigned int endPos);  // 2012 0xa8c130

    GRect<int>   Bounds;
    GMatrix2D    Matrix;
    unsigned int GlyphCount;
    unsigned int RecordCount;
    bool         bUseFlashType;              // set by GFx_CSMTextSettings, 0xa26ca0
};

// ---------------------------------------------------------------------------------------------
// GFxButtonCharacterDef (2012 Read 0xa6a0c0, GFxButtonRecord::Read 0xa69bb0, SetScale9Grid
// 0xa35150). The button's own state machine (GFxButtonCharacter, 38 retail functions, and
// GFx_GenerateMouseButtonEvents 0xa66a90) needs the mouse path and is not in this package: the
// definition is complete, the instance is a display object with the up-state records placed.

class GFxButtonRecord
{
public:
    enum StateBits
    {
        State_HitTest = 0x8, State_Down = 0x4, State_Over = 0x2, State_Up = 0x1,
        Has_BlendMode = 0x20, Has_FilterList = 0x10
    };

    GFxButtonRecord() : StateFlags(0), CharacterId(0), Depth(0), BlendMode(0) {}

    bool Read(GFxStream* s, unsigned int tagType, unsigned int endPos);  // 2012 0xa69bb0

    unsigned int      StateFlags;
    unsigned int      CharacterId;
    int               Depth;
    GMatrix2D         Matrix;
    GRenderer::Cxform ColorTransform;
    unsigned char     BlendMode;
};

class GFxButtonCharacterDef : public GFxCharacterDef
{
public:
    enum { MaxRecords = 64 };

    GFxButtonCharacterDef()
        : RecordCount(0), CondActionCount(0), bTrackAsMenu(false), bHasScale9Grid(false) {}

    virtual GFxCharacter* CreateCharacterInstance(GFxASCharacter* parent, GFxResourceId id,
                                                  GFxMovieDefImpl* defImpl);
    virtual unsigned int GetResourceTypeCode() const { return GFxResource::RT_ButtonDef; }
    virtual const char* GetDefTypeName() const { return "Button"; }
    // DISHONORED(port): 2012 0xa69000 GFxButtonCharacter::Display, up state only. GFxDisplay.cpp.
    virtual void Display(GFxDisplayContext& ctx, GFxCharacter* ch);

    void Read(GFxStream* s, unsigned int tagType, unsigned int endPos); // 2012 0xa6a0c0
    void SetScale9Grid(const GRect<float>& r) { Scale9Grid = r; bHasScale9Grid = true; }

    GFxButtonRecord Records[MaxRecords];
    unsigned int    RecordCount;
    unsigned int    CondActionCount;
    bool            bTrackAsMenu;
    bool            bHasScale9Grid;
    GRect<float>    Scale9Grid;
};

// ---------------------------------------------------------------------------------------------
// GFxImageCharacterDef: what a bitmap tag leaves in the dictionary. Agent BB established that this
// cook's bitmaps are stripped to external image references - the whole game has exactly three
// non-standard tag codes (1000 ExporterInfo, 1008 DefineSubImage, 1009 DefineExternalImage2) and no
// embedded DefineBits tag anywhere - so this is a reference record, which is exactly what retail
// builds too: GFx_DefineExternalImageLoader2 (0xa36210) makes a GFxImageFileInfo and hands it to
// GFxImageFileResourceCreator (0xa22830), and the file name is resolved at bind time by
// FGFxImageLoader. Nothing decodes pixels here and nothing needs to: the renderer's texture path is
// package CC's.
class GFxImageCharacterDef : public GFxCharacterDef
{
public:
    GFxImageCharacterDef(unsigned int tagCode)
        : TagCode(tagCode), Format(0), TargetWidth(0), TargetHeight(0), BaseImageId(0),
          bIsSubImage(false), bResolveTried(false)
    { ExportName[0] = 0; FileName[0] = 0; }

    virtual GFxCharacter* CreateCharacterInstance(GFxASCharacter* parent, GFxResourceId id,
                                                  GFxMovieDefImpl* defImpl);
    virtual unsigned int GetResourceTypeCode() const { return GFxResource::RT_Image; }
    virtual const char* GetDefTypeName() const { return bIsSubImage ? "SubImage" : "Image"; }
    virtual void Display(GFxDisplayContext& ctx, GFxCharacter* ch);

    // The texture this reference resolves to, bound on first use through the movie definition's
    // State_ImageLoader - FGFxImageLoader in the engine (2013 0x586020). Retail binds at load time in
    // GFxImageFileResourceCreator (0xa22830); binding lazily costs one lookup on the first frame a
    // bitmap is drawn and needs no bind pass. Stated in agentDC.md.
    GTexture*   GetTexture(GRenderer* renderer, class GFxMovieDataDef* dataDef);
    const char* GetResolveName() const;
    // The texture's own pixel size, which the fill matrix has to be divided by. The cooked size is the
    // tag's rounded up to a multiple of four (agentBB.md 3.3: HUD_I2 is 606x232 in the tag and 608x232
    // in the texture), so it is asked of the resolved image rather than taken from the tag.
    bool        GetImageSize(GRenderer* renderer, class GFxMovieDataDef* dataDef,
                             unsigned int* outWidth, unsigned int* outHeight);

    unsigned int  TagCode;
    unsigned int  Format;                    // the u32 the loader masks with 0x9FFFF
    unsigned int  TargetWidth, TargetHeight;
    unsigned int  BaseImageId;               // tag 1008: the atlas this is a rectangle of
    GRect<int>    SubRect;                   // tag 1008: the rectangle, in pixels
    bool          bIsSubImage;
    char          ExportName[128];
    char          FileName[192];

private:
    GPtr<GImageInfoBase> pImageInfo;
    bool                 bResolveTried;
};

// ---------------------------------------------------------------------------------------------
// GFxFontCharacterDef: what GFx_DefineFontLoader (2012 0xa357d0) leaves in the dictionary for
// DefineFont (10), DefineFont2 (48), DefineFont3 (75) and GFx's compacted form (1005). The glyph
// outlines are read with the shape reader above, which is what makes this cheap: agent BB proved
// the two game fonts are outlines with no font-texture tag anywhere in the cook, so this def is the
// input the glyph rasteriser needs. The rasteriser, the caches and GFxFontData's 60-odd accessors
// are package CB's; this is the loader's product and nothing more.
class GFxFontCharacterDef : public GFxCharacterDef
{
public:
    enum Flags
    {
        Flag_Bold = 0x1, Flag_Italic = 0x2, Flag_WideCodes = 0x4, Flag_WideOffsets = 0x8,
        Flag_Ansi = 0x10, Flag_ShiftJis = 0x20, Flag_SmallText = 0x40, Flag_HasLayout = 0x80
    };

    GFxFontCharacterDef(unsigned int tagCode);
    virtual ~GFxFontCharacterDef();

    virtual GFxCharacter* CreateCharacterInstance(GFxASCharacter* parent, GFxResourceId id,
                                                  GFxMovieDefImpl* defImpl);
    virtual unsigned int GetResourceTypeCode() const { return GFxResource::RT_Font; }
    virtual const char* GetDefTypeName() const { return "Font"; }

    void Read(GFxStream* s, unsigned int tagType, unsigned int endPos); // 2012 0xa357d0
    void ReadFontInfo(GFxStream* s, unsigned int tagType, unsigned int endPos); // 0xa35960

    unsigned int GetGlyphCount() const { return GlyphCount; }
    const GFxShapeRecord* GetGlyphShape(unsigned int i) const
        { return i < GlyphCount ? Glyphs[i] : 0; }
    int  GetGlyphIndexForCode(unsigned int code) const;
    float GetAdvance(unsigned int glyph) const
        { return Advances && glyph < GlyphCount ? Advances[glyph] : 0.f; }

    unsigned int TagCode;
    char         Name[128];
    unsigned int Flags;
    unsigned int GlyphCount;
    float        Ascent, Descent, Leading;
    unsigned int KerningPairCount;

private:
    GFxShapeRecord** Glyphs;
    unsigned short*        CodeTable;
    float*                 Advances;
    GRect<int>*            GlyphBounds;

    void Alloc(unsigned int count);
};

// ---------------------------------------------------------------------------------------------
// The load process. Retail's GFxLoadProcess (ctor 2012 0xa23440) is a loader task carrying the
// stream, the movie data def being built, the frame's tag array and the bind states; a loader takes
// one plus a GFxTagInfo. Ours carries the three things a loader here needs, which keeps the loader
// signatures identical to retail's and the tag walk a dispatch rather than a switch with bodies in
// it.

struct GFxTagInfo
{
    unsigned int TagType;
    unsigned int TagOffset;                  // the start of the tag header
    unsigned int TagDataOffset;              // the first byte of the body
    unsigned int TagLength;                  // the body length

    unsigned int GetEndPos() const { return TagDataOffset + TagLength; }
};

class GFxLoadProcess
{
public:
    GFxLoadProcess(GFxMovieDataDef* def, GFxStream* s, GFxTimelineDef* timeline)
        : pDataDef(def), pStream(s), pTimeline(timeline), Frame(0) {}

    GFxStream*       GetStream() const { return pStream; }              // 2012 0x9deea0
    unsigned int     Tell() const { return pStream->Tell(); }           // 2012 0x9deeb0
    GFxMovieDataDef* GetDataDef() const { return pDataDef; }
    GFxTimelineDef*  GetTimeline() const { return pTimeline; }

    unsigned char  ReadU8() { return pStream->ReadU8(); }               // 2012 0xa35020
    unsigned short ReadU16() { return pStream->ReadU16(); }             // 2012 0xa8df70
    unsigned int   ReadU32() { return pStream->ReadU32(); }             // 2012 0xa35060

    // ReadRgbaTag (2012 0xa22460): RGBA for DefineShape3 and above, RGB with an implied 255 alpha
    // below it. The threshold is the tag type, which is why every style reader is given one.
    void ReadRgbaTag(GColor* out, unsigned int tagType);                // 2012 0xa22460

    void AddCharacter(unsigned int id, GFxCharacterDef* def);
    GFxCharacterDef* GetCharacterDef(unsigned int id) const;
    void AddExecuteTag(GASExecuteTag* tag);                             // 2012 0x9e2810
    void AddInitActionTag(GASExecuteTag* tag);

    GFxMovieDataDef* pDataDef;
    GFxStream*       pStream;
    GFxTimelineDef*  pTimeline;
    unsigned int     Frame;
};

typedef void (*GFxTagLoaderFn)(GFxLoadProcess* p, const GFxTagInfo& info);

// The two tables of GFxLoaderImpl::GetTagLoader (2012 0x9badf0), read out of the database.
GFxTagLoaderFn GFxGetTagLoader(unsigned int tagCode);

#endif // GFX3_CHARACTERDEFS_H
