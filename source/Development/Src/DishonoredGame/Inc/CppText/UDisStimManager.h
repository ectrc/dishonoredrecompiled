// UDisStimManager cpptext: included inside the generated class body (DishonoredGameClasses.h).
// DISHONORED(port): the fixed-size pool every FAIStimStruct the AI brain queues is allocated out of. m_pStimData is one
// appMalloc of m_MaxNumStims blocks of m_MaxStimSize_bytes; a free block carries an FDisStimHeader (prev/next, 8 bytes) in
// its first eight bytes, so the free list lives inside the pool itself. A stim larger than a block, or a request made when
// the pool is exhausted, falls back to the heap and ReleaseBlock tells the two apart by address range.
// Bodies in Src/disstimmanager.cpp: InitStimManager 2013 rva 0x724990 (2012 0x767a90), TermStimManager 0x73d3f0
// (0x777080), AllocateBlock 0x724b80 (0x767c80), AllocateBlock_Common 0x7275b0 (0x764e70), ReleaseBlock 0x724ba0
// (0x767ca0), GetResourceSize 0x727610 (0x764ed0).
public:
	void InitStimManager( INT _MaxStimSize_bytes, INT _MaxNumStims );
	void TermStimManager();
	virtual INT GetResourceSize();

	void* AllocateBlock( INT _StimSize_bytes, const FName& _rDebugStimName );
	void ReleaseBlock( void* _pReleaseMe );

	INT GetNumAllocatedStims() const { return m_NumAllocatedStims; }
	INT GetMaxStimSize() const { return m_MaxStimSize_bytes; }

private:
	void* AllocateBlock_Common( void* _pWhere, INT _StimSize_bytes );
	UBOOL IsInPool( const void* _pBlock ) const;
