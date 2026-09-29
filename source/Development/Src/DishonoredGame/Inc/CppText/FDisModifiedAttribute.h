// FDisModifiedAttribute cpptext: included inside the generated struct body.
public:
	// DISHONORED(port): agent ED (PHASE11 ED), 2012 rva 0x8ef4d0 (disattributes.cpp:431). Body in
	// dissavegame.cpp. UDisAttributes::GameLoad reaches it through TMapBase's own operator<<.
	friend FArchive& operator<<( FArchive& _rArchive, FDisModifiedAttribute& _rModifiedAttribute );
