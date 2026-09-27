#pragma once
/*===========================================================================
    EngineDisFogClasses.h - Arkane's fog classes of the retail Engine package.

    DISHONORED(port): retail has no height fog and no exponential height fog. Every fog in the game is a
    UDisFogComponent on an ADisFog actor: a layer with a height, a near/far/no-fog plane, a density factor, a colour
    and optionally a colour lookup texture. FScene::AddDisFog turns each attached component into an FDisFogSceneInfo
    (FogRendering.h) and FSceneRenderer::RenderFog (2013 rva 0x4370a0) draws up to four of them in one full-screen
    pass with TDisFogPixelShader<FDisFogPolicy<Layers,Luts>>.

    Layouts are the retail SDK dump (retail_sdk_layout.json) and the 2012 PDB (UDisFogComponent 156 bytes,
    UDisFogDisplayComponent 456, ADisFog 596); sizes are asserted below. Bodies: Engine/Src/disfogcomponent.cpp and
    disfogdisplaycomponent.cpp, the retail source files of the 2012 PDB.
===========================================================================*/
#ifndef NAMES_ONLY
#ifndef INCLUDED_ENGINE_DISFOG_CLASSES
#define INCLUDED_ENGINE_DISFOG_CLASSES 1

#if SUPPORTS_PRAGMA_PACK
#pragma pack (push,4)
#endif

// the DECLARE_CLASS family of the script classes, like every generated *Classes.h of the module
#define ENABLE_DECLARECLASS_MACRO 1
#include "UnObjBas.h"
#undef ENABLE_DECLARECLASS_MACRO

// Engine.DisFogComponent: retail sizeof 156, reflected span 84..156
class UDisFogComponent : public UActorComponent
{
public:
    //## BEGIN PROPS DisFogComponent
    BITFIELD bEnabled:1;
    BITFIELD bCustomTransition:1;
    BITFIELD bIsSun:1;
    BITFIELD bInteriorFog:1;
    BITFIELD bUseExponentialAttenuation:1;
    BITFIELD m_IsExclusive:1;
    BITFIELD m_UseWorldOrigin:1;
    FLOAT fCustomTransitionHeight;
    FLOAT SunPower;
    FLOAT Opacity;
    FColor LightColor;
    FLOAT Height;
    FLOAT Origin;
    FLOAT NearPlane;
    FLOAT FarPlane;
    FLOAT NoFogPlane;
    FLOAT HeightDensityFactor;
    FLOAT FarPlaneForExponential;
    FLOAT Density;
    class UTexture2D* FogLUT;
    TArrayNoInit<BYTE> m_RawLut;
    FLOAT m_WorldOrigin;
    //## END PROPS DisFogComponent

    /** DISHONORED(port): disfogcomponent.cpp; 2013 rvas Attach 0xd5ef0, Detach 0xd5f40, UpdateTransform 0xd5f10, SetParentToWorld 0xdc9c0. */
    void SetEnabled(UBOOL bSetEnabled);

    DECLARE_FUNCTION(execSetEnabled)
    {
        P_GET_UBOOL(bSetEnabled);
        P_FINISH;
        this->SetEnabled(bSetEnabled);
    }

protected:
    virtual void SetParentToWorld(const FMatrix& ParentToWorld);
    virtual void Attach();
    virtual void UpdateTransform();
    virtual void Detach(UBOOL bWillReattach = FALSE);

    DECLARE_CLASS(UDisFogComponent,UActorComponent,0,Engine)
};

// Engine.DisFogDisplayComponent: retail sizeof 464, reflected span 452..456. Editor-only ruler drawing; its proxy
// draws nothing in game (2013 rva 0xdc a40 CreateSceneProxy returns NULL outside the editor).
class UDisFogDisplayComponent : public UPrimitiveComponent
{
public:
    //## BEGIN PROPS DisFogDisplayComponent
    BITFIELD mShowRulerOnlyWhenSelected:1;
    //## END PROPS DisFogDisplayComponent

    DECLARE_CLASS(UDisFogDisplayComponent,UPrimitiveComponent,0,Engine)
    NO_DEFAULT_CONSTRUCTOR(UDisFogDisplayComponent)
};

// Engine.DisFog: retail sizeof 608, reflected span 584..596 (new in 2013). The two event wrappers of the generated
// shim block (eventReplicatedEvent / eventPostBeginPlay) are dropped: they used DishonoredGame's FName globals and
// nothing in either module calls them - script reaches those events through ProcessEvent by name.
class ADisFog : public AInfo
{
public:
    //## BEGIN PROPS DisFog
    class UDisFogComponent* Component;
    BITFIELD ShowRulerOnlyWhenSelected:1;
    BITFIELD bEnabled:1;
    class UDisFogDisplayComponent* mDisplay;
    //## END PROPS DisFog

    DECLARE_CLASS(ADisFog,AInfo,0,Engine)
    NO_DEFAULT_CONSTRUCTOR(ADisFog)
};

#if SUPPORTS_PRAGMA_PACK
#pragma pack (pop)
#endif

static_assert(sizeof(UDisFogComponent) == 156, "UDisFogComponent: retail sizeof 156");
static_assert(sizeof(UDisFogDisplayComponent) == 464, "UDisFogDisplayComponent: retail sizeof 464");
static_assert(sizeof(ADisFog) == 608, "ADisFog: retail sizeof 608");
static_assert(STRUCT_OFFSET(UDisFogComponent, fCustomTransitionHeight) == 88, "UDisFogComponent::fCustomTransitionHeight: retail SDK @88");
static_assert(STRUCT_OFFSET(UDisFogComponent, LightColor) == 100, "UDisFogComponent::LightColor: retail SDK @100");
static_assert(STRUCT_OFFSET(UDisFogComponent, FogLUT) == 136, "UDisFogComponent::FogLUT: retail SDK @136");
static_assert(STRUCT_OFFSET(UDisFogComponent, m_WorldOrigin) == 152, "UDisFogComponent::m_WorldOrigin: retail SDK @152");
static_assert(STRUCT_OFFSET(ADisFog, Component) == 584, "ADisFog::Component: retail SDK @584");
static_assert(STRUCT_OFFSET(ADisFog, mDisplay) == 592, "ADisFog::mDisplay: retail SDK @592");

#endif // !INCLUDED_ENGINE_DISFOG_CLASSES
#endif // !NAMES_ONLY

extern FNativeFunctionLookup GEngineUDisFogComponentNatives[];

#define AUTO_INITIALIZE_REGISTRANTS_ENGINE_DISFOG \
	UDisFogComponent::StaticClass(); \
	GNativeLookupFuncs.Set(FName("DisFogComponent"), GEngineUDisFogComponentNatives); \
	UDisFogDisplayComponent::StaticClass(); \
	ADisFog::StaticClass(); \
