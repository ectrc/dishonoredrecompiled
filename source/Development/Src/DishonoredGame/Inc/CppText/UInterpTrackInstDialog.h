// UInterpTrackInstDialog cpptext: included inside the generated class body (DishonoredGameSoireeClasses.h).
// DISHONORED(port): agent IE (PHASE23 IE). The per-matinee state of a dialog track: m_DialogKeysBlurbs holds each key's main
// and alternative blurbs, resolved once from the soiree node that started the matinee. 2013 rvas (2012 in parentheses):
// InitTrackInst 0x882330 (0x8e9a90), UpdateTrackInst 0x873750 (0x8ec050), NeedsSynchronizing 0x8735c0 (0x8e0100),
// TermTrackInst 0x873ab0 (0x8e0290), GetBlurb 0x86ee10 (0x8dc6e0), GetAltBlurb 0x86ee60 (0x8dc730). Bodies in
// interptrackinstdialog.cpp (Init/Term/GetBlurb/GetAltBlurb) and interptrackdialog.cpp (Update/NeedsSynchronizing), as the PDB
// attributes them.
public:
	virtual void InitTrackInst( UInterpTrack* Track );
	virtual void TermTrackInst( UInterpTrack* Track );
	virtual UBOOL NeedsSynchronizing( FLOAT OldPosition, FLOAT NewPosition, FLOAT& OutClampedPosition, UBOOL bPreview );
	virtual void UpdateTrackInst( FLOAT NewPosition, UBOOL bJump );
	class UDisConv_Blurb* GetBlurb( INT iKeyIndex ) const;
	class UDisConv_Blurb* GetAltBlurb( INT iKeyIndex, INT iBlurbIndex ) const;
