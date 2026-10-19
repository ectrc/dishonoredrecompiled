// UDisAudioPropagationInfo cpptext: included inside the generated class body (dishonoredgameclasses.h).
// DISHONORED(port): agent IE (PHASE23 IE). PostLoad 2013 rva 0x7a3070 (vtable 0x115c380 slot 6; 2012 0x807670). Agent IH3
// (fork G): FindShortestPath 2013 0x792320 (2012 0x7f7f30), FinalizeAudioPath 2013 0x79d5e0 (2012 0x801dd0); the path types
// are in disaudiopropagation.h. Bodies in disaudiopropagationinfo.cpp.
public:
	virtual void PostLoad();
	UBOOL FindShortestPath( const struct FDisAudioPathEndPair& _rPathEndPair, FLOAT _fMaxAudibleDistance, UBOOL _bSwapEnds, struct FDisAudioPortalPath& _rOutPath );
	UBOOL FinalizeAudioPath( FDisAudioPropagationCache& _rPropagationCache, const struct FDisAudioPortalPath& _rPath, INT _SkipFirstPortal, INT _SkipLastPortal,
		const FVector& _rFromPoint, const FVector& _rToPoint, FLOAT _fRawDistSq, UBOOL _bForAINoise, TArray<FDisAudioDebugPath>* _pDebugPaths );
