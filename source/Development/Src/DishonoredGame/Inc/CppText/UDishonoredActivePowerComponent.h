// UDishonoredActivePowerComponent cpptext: included inside the generated class body (DishonoredGamePowerClasses.h).
// DISHONORED(port): agent EF (PHASE11 EF), 2013 rva 0x7e70e0 (2012 0x8249e0, byte-identical, 24 bytes). Retail
// folds GameSave and GameLoad onto this one body - it is nothing but the power's acquired level - and the fold is
// shared only with UDishonoredActivePowerComponent_Blink and _WindBlast, which inherit it here too
// (build/agentEF/folds.txt). Body in dissavegame.cpp.
public:
	virtual void GameSave( FArchive& _rArchive, ESaveLoadLocation _Location );
	virtual void GameLoad( FArchive& _rArchive, ESaveLoadLocation _Location );
	/** Retail declares IsSaveable inline here as `return TRUE`; all such bodies are ICF-folded onto
	    UObject::IsRefSaveable's, which is why the PDB names only the eleven with a body of their own. */
	virtual UBOOL IsSaveable( ESaveLoadLocation Location ) const { return TRUE; }
