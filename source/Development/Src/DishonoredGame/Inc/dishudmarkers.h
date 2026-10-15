/*=============================================================================
	dishudmarkers.h: the HUD's world markers (agent HI, PHASE22).

	DISHONORED(port): native-only classes of the 2012 PDB (types.json: FDisHUDMarker 48 bytes, FDisTaskMarker 128,
	FDisHUDMarkerPosInfo 24) as the 2013 bodies use them: 2013's marker is 56 bytes, the 4 new bytes at +40 being the
	flag UpdateTaskMarkerInstances (2013 rva 0x7ad550) clears for a hidden pawn, and the task marker is 136 bytes
	(its target name is localised from the target's file and section). Vtable order, both builds: the deleting
	destructor, CreateGFxInstance, DestroyGFxInstance, Tick (empty in the base), GetLocation, Update.
	Bodies in dishudmarkers.cpp; the HUD keeps the instances in m_TaskMarkers and m_SortedMarkers (FPointer arrays).
=============================================================================*/
#ifndef DISHUDMARKERS_H
#define DISHUDMARKERS_H

#include "gfxui_gfx3.h"

/** where PreRender_Markers put a marker this frame (2012 PDB FDisHUDMarkerPosInfo) */
struct FDisHUDMarkerPosInfo
{
	FVector2D m_Position;
	FVector2D m_TrackedPosition;
	FLOAT m_fDistance;
	UINT m_Flags;
};

/** FDisHUDMarkerPosInfo::m_Flags as Update reads them */
enum EDisHUDMarkerFlags
{
	DHMF_Visible = 0x1,
	DHMF_Focused = 0x2,
	DHMF_ClampedToArea = 0x4,
	DHMF_OffScreen = 0x8,
};

class FDisHUDMarker
{
public:
	FDisHUDMarker( const AActor* _pTrackedActor, const FDisHUDMarkerSettings* _pSettings, FGFxMovie* _pGFxMovie, UBOOL _bCreateGFxInstance ); // 2013 rva 0x7ce380
	virtual ~FDisHUDMarker();                                                                // 2013 rva 0x7ce410
	virtual void CreateGFxInstance();                                                        // 2013 rva 0x7c54f0
	virtual void DestroyGFxInstance();                                                       // 2013 rva 0x7c5730
	virtual void Tick( FLOAT _fDeltaTime ) {}
	virtual FVector GetLocation() const;                                                     // 2013 rva 0x7b9f70
	virtual void Update( FVector2D _Position, FVector2D _TrackedPosition, FLOAT _fDistance, UINT _Flags ); // 2013 rva 0x7bd430
	UBOOL SwapDepth( FDisHUDMarker* _pOther );                                               // 2013 rva 0x7bd4c0

	const AActor* m_pTrackedActor;
	const FDisHUDMarkerSettings* m_pSettings;
	FGFxMovie* m_pGFxMovie;
	INT m_Depth;
	GFxValue m_GFxInstance;
	UBOOL m_bTrackedActorVisible;
	UBOOL m_bNeedsSorting;
	UINT m_VisibilityFlags;
};

class FDisTaskMarker : public FDisHUDMarker
{
public:
	FDisTaskMarker( const FDisTaskTarget* _pTaskTarget, const FDisHUDMarkerSettings* _pSettings, FGFxMovie* _pGFxMovie ); // 2013 rva 0x7ce750
	virtual ~FDisTaskMarker();                                                               // 2013 rva 0x7ce680
	virtual FVector GetLocation() const;                                                     // 2013 rva 0x7b9fa0
	virtual void Update( FVector2D _Position, FVector2D _TrackedPosition, FLOAT _fDistance, UINT _Flags ); // 2013 rva 0x7c57f0

	FString m_TargetName;
	const FDisTaskMarkerSettings* m_pSpecificSettings;
	const FDisHUDMarkerVariation* m_pVanishPreset;
	GFxValue m_GFxTaskIcon;
	GFxValue m_GFxTaskDesc;
	GFxValue m_GFxTaskDescText;
	INT m_Distance;
	UINT m_FrameNumber;
};

/** 2013 rva 0x7be1a0 (2012 ComputeMarkerScreenPosition 0x8264b0, which had the canvas size and no zoom): the marker's
	tracked position on the movie-space screen and, off screen or outside the marker area, its position clamped to the
	area's border. Returns the on-screen flag set (0 or DHMF_OffScreen >> 3). */
UINT DisComputeMarkerScreenPosition( const FVector& _Location, UCanvas* _pCanvas, FLOAT _fZoom, FLOAT _fAreaRatio, FLOAT _fSizeX, FLOAT _fSizeY,
	FVector2D& _rOutTrackedPosition, FVector2D& _rOutPosition, UBOOL _bTripleScreen );

#endif
