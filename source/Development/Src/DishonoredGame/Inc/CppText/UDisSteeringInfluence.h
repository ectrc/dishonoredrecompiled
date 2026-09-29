// UDisSteeringInfluence cpptext: included inside the generated class body (DishonoredGameSteeringClasses.h).
// DISHONORED(port): agent EC (PHASE11 EC), 2012 rva 0x630910 - retail's linker folds this class's GameSave
// and GameLoad onto UDisAttentionInfo_Base's, which is the object's own script properties, binary and
// untagged. The fold crosses unrelated class trees, so the body has to be declared here as well or this
// tree's dispatch lands on UObject::GameLoad and reads none of the bytes retail wrote.
// 5 classes reach it through this one. Bodies in dissavegame.cpp.
public:
	virtual void GameSave( FArchive& _rArchive, ESaveLoadLocation _Location );
	virtual void GameLoad( FArchive& _rArchive, ESaveLoadLocation _Location );
