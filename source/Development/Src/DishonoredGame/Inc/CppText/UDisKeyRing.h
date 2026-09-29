// UDisKeyRing cpptext: included inside the generated class body (DishonoredGameClasses.h).
// DISHONORED(port): agent EF (PHASE11 EF), 2013 rva 0x6dd250 (2012 0x7184e0, byte-identical) - the keys, a flag,
// and the backup set behind it. GameSave (0x6dd200) is a separate retail body and is not ported, for the reason
// agent ED gave: the writing half of the object layer does not exist here. Body in dissavegame.cpp.
public:
	virtual void GameLoad( FArchive& _rArchive, ESaveLoadLocation _Location );
	virtual UBOOL IsSaveable( ESaveLoadLocation Location ) const { return TRUE; }
