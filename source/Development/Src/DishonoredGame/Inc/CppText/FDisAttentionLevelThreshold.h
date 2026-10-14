// FDisAttentionLevelThreshold cpptext: included inside the generated struct body (dishonoredgameclasses.h).
// DISHONORED(port): agent GI (PHASE20 GI). 2012 rva 0x7673e0 / 2013 0x701570 (90 B): the stop threshold is the stop level
// plus a uniform random offset in [-variance, +variance], rolled once per brain at InitBrainProcess_Derived. Body in
// Src/distweaks_pawnattention.cpp.
public:
	FLOAT CalculateStoppingThreshold() const;
