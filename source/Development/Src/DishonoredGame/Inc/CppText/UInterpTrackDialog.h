// UInterpTrackDialog cpptext: included inside the generated class body (DishonoredGameSoireeClasses.h).
// DISHONORED(port): agent IE (PHASE23 IE). The spoken lines of a scripted scene: each DialogKeys entry names a blurb of the
// soiree's conversation by GUID. 2013 rvas (2012 in parentheses), resolved by bytes and the ??_7UInterpTrackDialog vtable at
// 0x118bf70: GetKeyProperties 0x86ebd0 (0x8e9960), IsPlayingASkippableItem 0x8699a0 (0x8da7a0), FindNextSkipAnchor 0x86ed00
// (0x8dc5d0), FindLastSkipAnchor 0x86eda0 (0x8dc670). UInterpTrack declares no skip surface in this tree (retail's matinee
// skip calls these through the vtable), so the three skip functions have no caller yet. Bodies in interptrackdialog.cpp.
public:
	virtual UObject* GetKeyProperties( INT KeyIndex );
	virtual UBOOL IsPlayingASkippableItem( UInterpTrackInst* _pInst ) const;
	virtual UBOOL FindNextSkipAnchor( FLOAT CurrentPosition, FLOAT& OutAnchor );
	virtual UBOOL FindLastSkipAnchor( FLOAT& OutAnchor );
