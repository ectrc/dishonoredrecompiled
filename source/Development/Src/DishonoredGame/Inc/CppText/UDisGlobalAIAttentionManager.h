// UDisGlobalAIAttentionManager cpptext: included inside the generated class body (dishonoredgameclasses.h).
// DISHONORED(port): agent GI (PHASE20 GI). The pool of UDisAttentionInfo_Simple every attention process draws its non-player
// infos from, an intrusive free list through UDisAttentionInfo_Simple::m_pSimpleAttnInfoFreeList_Next. GetSimpleAttnInfoFromPool
// 2012 rva 0x890e30 / 2013 0x846070 (198 B), ReleaseSimpleAttnInfoToPool 0x890f00 / 0x846140 (30 B), PostCommitMapChange
// 0x88b7f0 (12 B), DisGlobalAIAttentionManager::AttnInfoFreeListGetNext 0x888440. Bodies in Src/disglobalaiattentionmanager.cpp.
public:
	// -gidiag: the destruction order at exit (the GI2 exit crash hunt).
	virtual void FinishDestroy();
	class UDisAttentionInfo_Simple* GetSimpleAttnInfoFromPool();
	void ReleaseSimpleAttnInfoToPool( class UDisAttentionInfo_Simple* _pReleaseMe );
	void PostCommitMapChange();
