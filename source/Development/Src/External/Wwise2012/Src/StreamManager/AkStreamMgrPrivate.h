/*=============================================================================
	AkStreamMgrPrivate.h - the classes of the Wwise 2012.1 default stream manager (module akstreammgr).

	DISHONORED(port): agent IH1 (wave 26), replacing AkSilentStreamMgr.cpp. Layouts are the 2012 PDB's
	(resources/docs/types/all_types.h): CAkIOThread 68 bytes, CAkDeviceBase/Blocking 0xD0 (CAkStreamMgr::
	CreateDevice mallocs 0xD0), CAkStmTask 72 with the IAkStdStream/IAkAutoStream vptr at 72 (the
	"adjustor{72}" thunks), CAkStdStmBlocking 0x88, CAkAutoStmBlocking 0x90, AkMemBlock 32, CAkStmMemView 16.
	Virtual slot orders are the PDB's *_vtbl records:
	  CAkIOThread_vtbl     dtor, PerformIO, ClearStreams, OnThreadStart
	  CAkDeviceBase_vtbl   + Init, Destroy, CreateStd, CreateAuto
	  CAkStmTask_vtbl      dtor, CanBeDestroyed, Kill, OnFileDeferredOpen, PrepareTransfer, Update, EffectiveDeadline
	  CAkAutoStmBase_vtbl  + GetVirtualFilePosition, CancelPendingTransfers, FlushSmallBuffersAndPendingTransfers,
	                         RecomputeVirtualBuffering
	Only the blocking device is implemented: retail's AkAudio creates its device with AK_SCHEDULER_BLOCKING
	(UAkAudioDevice::EnsureInitialized, 2013 0x5b1430), so CAkDeviceDeferredLinedUp (19 functions) and
	CAkTransferDeferred (4) are never instantiated in Dishonored; CreateDevice refuses the deferred flag with
	AK_INVALID_DEVICE_ID instead of building them (named deviation, see AkStreamMgr.cpp).
=============================================================================*/
#pragma once

#include <AkPrivateTypes.h>
#include <AK/SoundEngine/Common/IAkStreamMgr.h>
#include <AK/SoundEngine/Common/AkStreamMgrModule.h>

namespace AK
{
	namespace StreamMgr
	{
		class CAkDeviceBase;
		class CAkStmTask;
		class CAkLowLevelTransfer;

		/** AkDeferredOpenData: 32 bytes; the file name (bByString) or id, plus the flags copy. */
		class AkDeferredOpenData
		{
		public:
			static AkDeferredOpenData * Create( AkFileID in_fileID, AkFileSystemFlags * in_pFSFlags, AkOpenMode in_eOpenMode );
			static AkDeferredOpenData * Create( const AkOSChar * in_pszFileName, AkFileSystemFlags * in_pFSFlags, AkOpenMode in_eOpenMode );
			void Destroy();

			union
			{
				AkOSChar *	pszFileName;
				AkFileID	fileID;
			};
			AkFileSystemFlags	flags;
			AkOpenMode			eOpenMode;
			AkUInt32			bByString : 1;
			AkUInt32			bUseFlags : 1;

		private:
			AKRESULT Init( const AkOSChar * in_pszFileName, AkFileSystemFlags * in_pFSFlags, AkOpenMode in_eOpenMode );
			void Init( AkFileID in_fileID, AkFileSystemFlags * in_pFSFlags, AkOpenMode in_eOpenMode );
		};

		/** One IO memory block (AkMemBlock, 32 bytes, align 8). */
		struct AkMemBlock
		{
			AkMemBlock() : uPosition( 0 ), pData( NULL ), pNextBlock( NULL ), uAvailableSize( 0 ), fileID( AK_INVALID_FILE_ID ), uRefCount( 0 ) {}
			AkMemBlock( void * in_pData ) : uPosition( 0 ), pData( in_pData ), pNextBlock( NULL ), uAvailableSize( 0 ), fileID( AK_INVALID_FILE_ID ), uRefCount( 0 ) {}

			AkUInt64	uPosition;
			void *		pData;
			union
			{
				AkMemBlock *			pNextBlock;
				CAkLowLevelTransfer *	pTransfer;
			};
			AkUInt32	uAvailableSize;
			AkFileID	fileID;
			AkUInt16	uRefCount;
		};

		struct AkListBareNextMemBlock
		{
			static AkMemBlock *& Get( AkMemBlock * in_pItem ) { return in_pItem->pNextBlock; }
		};

		/** CAkStmMemView: one view on a block (16 bytes); m_eStatus 3 bits, m_bCachedAlloc 1 bit. */
		class CAkStmMemView
		{
		public:
			enum TransferStatusType
			{
				TransferStatus_Pending		= 0,
				TransferStatus_Cancelled	= 2,
				TransferStatus_Ready		= 3
			};

			CAkStmMemView() : pNextView( NULL ), m_pBlock( NULL ), m_uOffsetInBlock( 0 ), m_eStatus( TransferStatus_Ready ), m_bCachedAlloc( 1 ) {}

			void Attach( AkMemBlock * in_pBlock, AkUInt32 in_uOffset ) { m_eStatus = TransferStatus_Pending; m_pBlock = in_pBlock; m_uOffsetInBlock = in_uOffset; }
			AkMemBlock * Detach() { AkMemBlock * pBlock = m_pBlock; m_pBlock = NULL; return pBlock; }
			AkMemBlock * Block() const { return m_pBlock; }
			AkUInt64 StartPosition() const { return m_pBlock->uPosition + m_uOffsetInBlock; }
			AkUInt64 EndPosition() const { return m_pBlock->uPosition + m_pBlock->uAvailableSize; }
			AkUInt32 Size() const { return m_pBlock->uAvailableSize - m_uOffsetInBlock; }
			void * Buffer() const { return (AkUInt8 *)m_pBlock->pData + m_uOffsetInBlock; }
			TransferStatusType Status() const { return (TransferStatusType)m_eStatus; }
			void TagAsCancelled() { m_eStatus = TransferStatus_Cancelled; }
			void TagAsReady() { m_eStatus = TransferStatus_Ready; }

			CAkStmMemView *	pNextView;
			AkMemBlock *	m_pBlock;
			AkUInt32		m_uOffsetInBlock;
			AkInt32			m_eStatus : 3;
			AkUInt32		m_bCachedAlloc : 1;
		};

		struct AkListBareNextMemView
		{
			static CAkStmMemView *& Get( CAkStmMemView * in_pItem ) { return in_pItem->pNextView; }
		};

		/** CAkLowLevelTransfer (empty base) / CAkLowLevelTransferBlocking: the transfer the blocking device executes. */
		class CAkLowLevelTransfer
		{
		};

		class CAkLowLevelTransferBlocking : public CAkLowLevelTransfer
		{
		public:
			AkIOTransferInfo	info;
			void *				pAddress;
		};

		struct ArrayPoolLocal
		{
			static AkMemPoolId Get();
		};

		/** AkMemBlocksDictionnary: block indices sorted by (fileID, uPosition, pData). */
		class AkMemBlocksDictionnary : public AkArray<AkUInt16, AkUInt16, ArrayPoolLocal, 0>
		{
		public:
			void Move( AkUInt32 in_uIndexSource, AkUInt32 in_uIndexDestination );
		};

		/** CAkIOMemMgr: the IO memory (one fixed-block pool) cut into granularity-sized AkMemBlocks. */
		class CAkIOMemMgr
		{
		public:
			CAkIOMemMgr();
			virtual ~CAkIOMemMgr();

			AKRESULT Init( const AkDeviceSettings & in_settings );
			void Term();

			AkUInt32 ReleaseBlock( AkMemBlock * in_pMemBlock );
			void GetOldestFreeBlock( AkMemBlock *& out_pMemBlock );
			AkUInt32 GetCachedBlock( AkFileID in_fileID, AkUInt64 in_uPosition, AkUInt32 in_uMinSize, AkUInt32 in_uRequiredAlign, AkUInt32 & io_uRequestedSize, AkMemBlock *& out_pMemBlock );
			void TagBlock( AkMemBlock * in_pMemBlock, CAkLowLevelTransfer * in_pTransfer, AkFileID in_fileID, AkUInt64 in_uPosition, AkUInt32 in_uDataSize );
			void UntagBlock( AkMemBlock * in_pMemBlock );
			void CloneTempBlock( AkMemBlock * in_pMemBlockBase, AkMemBlock *& out_pMemBlock );
			void DestroyTempBlock( AkMemBlock * in_pMemBlockBase, AkMemBlock * in_pMemBlock );
			AkUInt32 NumViewsAvailable() const { return m_uNumViewsAvail; }
			bool UseCache() const { return m_bUseCache; }

			void *										m_pIOMemory;
			AkMemBlock *								m_pMemBlocks;
			AkListBare<AkMemBlock, AkListBareNextMemBlock>	m_listFreeBuffers;
			AkMemBlocksDictionnary						m_arMemBlocks;
			AkMemPoolId									m_streamIOPoolId;
			AkUInt32									m_uNumViewsAvail;
			bool										m_bUseCache;

		private:
			AkInt32 FindBlockIndex( const AkMemBlock & in_block ) const;
			AkInt32 FindInsertionIndex( AkFileID in_fileID, AkUInt64 in_uPosition, void * in_pData ) const;
		};

		/** CAkClientThreadAware: the manual-reset event a client thread blocks on (12 bytes). */
		class CAkClientThreadAware
		{
		public:
			CAkClientThreadAware();
			virtual ~CAkClientThreadAware();

			void SetBlockedStatus();
			bool IsBlocked() const { return m_bIsBlocked; }
			void ClearBlockedStatus() { m_bIsBlocked = false; }

			HANDLE	m_hBlockEvent;
			bool	m_bIsBlocked;
		};

		/** CAkIOThread: "AK::IOThread" and its semaphores (68 bytes). */
		class CAkIOThread
		{
		public:
			CAkIOThread();
			virtual ~CAkIOThread();

			void NotifyMemChange();
			void NotifyMemIdle();
			void WaitForIOCompletion( CAkClientThreadAware * in_pWaitingTask );
			void SignalIOCompleted( CAkClientThreadAware * in_pWaitingTask );
			void StdSemIncr();
			void StdSemDecr();
			void AutoSemIncr();
			void AutoSemDecr();
			void IncrementIOCount();
			void DecrementIOCount();

			CAkLock & LockMem() { return m_lockSems; }

		protected:
			virtual void PerformIO() = 0;
			virtual bool ClearStreams() = 0;
			virtual void OnThreadStart() = 0;

			AKRESULT Init( const AkThreadProperties & in_threadProperties );
			void Term();
			static DWORD WINAPI IOSchedThread( LPVOID lpParameter );

		public:
			AkUInt32	m_uMaxConcurrentIO;
			HANDLE		m_hIOThread;
			CAkLock		m_lockSems;
			HANDLE		m_hIOThreadStopEvent;
			HANDLE		m_hMaxIOGate;
			HANDLE		m_hStdSem;
			HANDLE		m_hAutoSem;
			AkInt32		m_cPendingStdStms;
			AkInt32		m_cRunningAutoStms;
			bool		m_bDoWaitMemoryChange;
			AkUInt32	m_uNumConcurrentIO;
		};

		/** CAkStmTask: a stream as the scheduler sees it. The status flags are the PDB's bit order (byte 69). */
		class CAkStmTask : public CAkClientThreadAware
		{
		public:
			CAkStmTask();
			virtual ~CAkStmTask();

			virtual bool CanBeDestroyed() = 0;
			virtual void Kill() = 0;
			virtual void OnFileDeferredOpen();
			virtual CAkStmMemView * PrepareTransfer( AkFileDesc *& out_pFileDesc, CAkLowLevelTransfer *& out_pLowLevelXfer ) = 0;
			virtual void Update( CAkStmMemView * in_pTransfer, AKRESULT in_eIOResult, bool in_bRequiredLowLevelXfer ) = 0;
			virtual AkReal32 EffectiveDeadline() = 0;

			AKRESULT SetDeferredFileOpen( AkFileDesc * in_pFileDesc, const AkOSChar * in_pszFileName, AkFileSystemFlags * in_pFSFlags, AkOpenMode in_eOpenMode );
			AKRESULT SetDeferredFileOpen( AkFileDesc * in_pFileDesc, AkFileID in_fileID, AkFileSystemFlags * in_pFSFlags, AkOpenMode in_eOpenMode );
			AKRESULT EnsureFileIsOpen();
			void SetFileOpen( AkFileDesc * in_pFileDesc ) { m_pFileDesc = in_pFileDesc; m_bIsFileOpen = true; }
			AkReal32 TimeSinceLastTransfer( const AkInt64 & in_liNow ) const;

			bool IsToBeDestroyed() const { return m_bIsToBeDestroyed; }
			bool ReadyForIO() const { return m_bIsReadyForIO; }
			bool RequiresScheduling() const { return m_bRequiresScheduling; }
			bool IsWriteOp() const { return m_bIsWriteOp; }
			bool StdSemIncr_Guard();
			AkPriority Priority() const { return m_priority; }
			void SetToBeDestroyed() { m_bIsReadyForIO = false; m_bIsToBeDestroyed = true; }

		protected:
			AkUInt32 ClampRequestSizeToEof( AkUInt64 in_uPosition, AkUInt32 in_uDesiredSize );

		public:
			CAkStmTask *			pNextLightItem;
			AkDeferredOpenData *	m_pDeferredOpenData;
			AkFileDesc *			m_pFileDesc;
			CAkLock					m_lockStatus;
			AkInt64					m_iIOStartTime;
			CAkDeviceBase *			m_pDevice;
			AkOSChar *				m_pszStreamName;
			AkUInt32				m_uLLBlockSize;
			AkPriority				m_priority;
			AkUInt8					m_bIsAutoStm : 1;
			AkUInt8					m_bIsWriteOp : 1;
			AkUInt8					m_bHasReachedEof : 1;
			AkUInt8					m_bIsToBeDestroyed : 1;
			AkUInt8					m_bIsFileOpen : 1;
			AkUInt8					m_bRequiresScheduling : 1;
			AkUInt8					m_bIsReadyForIO : 1;
		};

		/** CAkStdStmBase: an explicit Read/Write stream (the bank reader's). */
		class CAkStdStmBase : public CAkStmTask, public AK::IAkStdStream
		{
		public:
			CAkStdStmBase();
			virtual ~CAkStdStmBase();

			AKRESULT Init( CAkDeviceBase * in_pDevice, AkFileDesc * in_pFileDesc, AkOpenMode in_eOpenMode );

			// IAkStdStream
			virtual void GetInfo( AkStreamInfo & out_info );
			virtual void * GetFileDescriptor();
			virtual AKRESULT SetStreamName( const AkOSChar * in_pszStreamName );
			virtual AkUInt32 GetBlockSize();
			virtual AKRESULT Read( void * in_pBuffer, AkUInt32 in_uReqSize, bool in_bWait, AkPriority in_priority, AkReal32 in_fDeadline, AkUInt32 & out_uSize );
			virtual AKRESULT Write( void * in_pBuffer, AkUInt32 in_uReqSize, bool in_bWait, AkPriority in_priority, AkReal32 in_fDeadline, AkUInt32 & out_uSize );
			virtual AkUInt64 GetPosition( bool * out_pbEndOfStream );
			virtual AKRESULT SetPosition( AkInt64 in_iMoveOffset, AkMoveMethod in_eMoveMethod, AkInt64 * out_piRealOffset );
			virtual void * GetData( AkUInt32 & out_uSize );
			virtual AkStmStatus GetStatus();

			// CAkStmTask
			virtual void Kill();
			virtual AkReal32 EffectiveDeadline();

		protected:
			AKRESULT ExecuteOp( bool in_bWrite, void * in_pBuffer, AkUInt32 in_uReqSize, bool in_bWait, AkPriority in_priority, AkReal32 in_fDeadline, AkUInt32 & out_uSize );
			void SetStatus( AkStmStatus in_eStatus );
			void AddMemView( CAkStmMemView * in_pMemView, bool in_bStoreData );
			void UpdateTaskStatus( AKRESULT in_eIOResult );

		public:
			AkMemBlock		m_memBlock;
			AkUInt32		m_uTotalScheduledSize;
			AkReal32		m_fDeadline;
			AkInt32			m_eStmStatus : 4;
			AkUInt32		m_bIsOpComplete : 1;
		};

		/** CAkAutoStmBase: a double-buffered read-ahead stream (streamed media; IH2 is its first client). */
		class CAkAutoStmBase : public CAkStmTask, public AK::IAkAutoStream
		{
		public:
			CAkAutoStmBase();
			virtual ~CAkAutoStmBase();

			AKRESULT Init( CAkDeviceBase * in_pDevice, AkFileDesc * in_pFileDesc, AkFileID in_fileID, const AkAutoStmHeuristics & in_heuristics, AkAutoStmBufSettings * in_pBufferSettings, AkUInt32 in_uGranularity );

			// IAkAutoStream
			virtual void Destroy();
			virtual void GetInfo( AkStreamInfo & out_info );
			virtual void * GetFileDescriptor();
			virtual void GetHeuristics( AkAutoStmHeuristics & out_heuristics );
			virtual AKRESULT SetHeuristics( const AkAutoStmHeuristics & in_heuristics );
			virtual AKRESULT SetMinimalBufferSize( AkUInt32 in_uMinBufferSize );
			virtual AKRESULT SetStreamName( const AkOSChar * in_pszStreamName );
			virtual AkUInt32 GetBlockSize();
			virtual AKRESULT QueryBufferingStatus( AkUInt32 & out_uNumBytesAvailable );
			virtual AkUInt32 GetNominalBuffering();
			virtual AKRESULT Start();
			virtual AKRESULT Stop();
			virtual AkUInt64 GetPosition( bool * out_pbEndOfStream );
			virtual AKRESULT SetPosition( AkInt64 in_iMoveOffset, AkMoveMethod in_eMoveMethod, AkInt64 * out_piRealOffset );
			virtual AKRESULT GetBuffer( void *& out_pBuffer, AkUInt32 & out_uSize, bool in_bWait );
			virtual AKRESULT ReleaseBuffer();

			// CAkStmTask
			virtual void Kill();
			virtual void OnFileDeferredOpen();
			virtual AkReal32 EffectiveDeadline();

		protected:
			virtual AkUInt64 GetVirtualFilePosition() = 0;
			virtual void CancelPendingTransfers( AkUInt64 in_uMaxKeepPosition ) = 0;
			virtual void FlushSmallBuffersAndPendingTransfers( AkUInt32 in_uMinBufferSize ) = 0;
			virtual void RecomputeVirtualBuffering() = 0;

			AkUInt32 GetEffectiveViewSize( CAkStmMemView * in_pStmMemView ) const;
			AKRESULT SetBufferingSettings( const AkAutoStmBufSettings * in_pBufferSettings, AkUInt32 in_uGranularity );
			void AddMemView( CAkStmMemView * in_pMemView, bool in_bStoreData );
			void UpdateSchedulingStatus();
			void UpdateTaskStatus( AKRESULT in_eIOResult );
			void Flush();
			void ForceFilePosition( AkUInt64 in_uNewPosition );
			void * GetReadBuffer( AkUInt32 & out_uSize );

		public:
			AkUInt64		m_uNextExpectedUserPosition;
			AkFileID		m_fileID;
			AkReal32		m_fThroughput;
			AkUInt32		m_uLoopStart;
			AkUInt32		m_uLoopEnd;
			AkUInt32		m_uBufferSize;
			AkUInt32		m_uVirtualBufferingSize;
			AkUInt32		m_uMinBufferSize;
			AkUInt32		m_uBufferAlignment;
			AkListBare<CAkStmMemView, AkListBareNextMemView>	m_listBuffers;
			AkUInt8			m_uNextToGrant;
			AkUInt8			m_uMinNumBuffers;
			AkUInt8			m_bIsRunning : 1;
			AkUInt8			m_bIOError : 1;
		};

		/** CAkDeviceBase: one streaming device = one IO thread, one IO memory manager, the task list. */
		class CAkDeviceBase : public CAkIOThread
		{
		public:
			CAkDeviceBase( IAkLowLevelIOHook * in_pLowLevelHook );
			virtual ~CAkDeviceBase();

			virtual AKRESULT Init( const AkDeviceSettings & in_settings, AkDeviceID in_deviceID );
			virtual void Destroy();
			virtual CAkStmTask * CreateStd( AkFileDesc * in_pFileDesc, AkOpenMode in_eOpenMode, IAkStdStream *& out_pStream ) = 0;
			virtual CAkStmTask * CreateAuto( AkFileDesc * in_pFileDesc, AkFileID in_fileID, const AkAutoStmHeuristics & in_heuristics, AkAutoStmBufSettings * in_pBufferSettings, IAkAutoStream *& out_pStream ) = 0;

			void ForceCleanup( bool in_bKillLowestPriorityTask, AkPriority in_priority );
			AkUInt32 DestroyMemView( CAkStmMemView * in_pMemView );

			IAkLowLevelIOHook * GetLowLevelHook() const { return m_pLowLevelHook; }
			AkUInt32 GetGranularity() const { return m_uGranularity; }
			AkReal32 GetTargetAutoStmBufferLength() const { return m_fTargetAutoStmBufferLength; }
			AkDeviceID GetDeviceID() const { return m_deviceID; }
			const AkInt64 & GetTime() const { return m_time; }
			CAkIOMemMgr & GetIOMemMgr() { return m_mgrMemIO; }

		protected:
			virtual bool ClearStreams();
			virtual void OnThreadStart();

			void AddTask( CAkStmTask * in_pStmTask );
			CAkStmTask * SchedulerFindNextTask( AkReal32 & out_fOpDeadline );
			CAkStmTask * ScheduleStdStmOnly( AkReal32 & out_fOpDeadline );
			void DestroyTask( CAkStmTask * in_pTask );

		public:
			AkInt64																m_time;
			AkListBareLight<CAkStmTask, AkListBareLightNextItem<CAkStmTask> >	m_listTasks;
			CAkLock																m_lockTasksList;
			CAkIOMemMgr															m_mgrMemIO;
			IAkLowLevelIOHook *													m_pLowLevelHook;
			AkUInt32															m_uGranularity;
			AkReal32															m_fTargetAutoStmBufferLength;
			AkDeviceID															m_deviceID;
			AkListBareLight<CAkStmMemView, AkListBareNextMemView>				m_poolStmMemView;
			CAkStmMemView *														m_pStmMemViewMem;
		};

		/** CAkDeviceBlocking: the device retail uses; one transfer at a time through IAkIOHookBlocking. */
		class CAkDeviceBlocking : public CAkDeviceBase
		{
		public:
			CAkDeviceBlocking( IAkLowLevelIOHook * in_pLowLevelHook );
			virtual ~CAkDeviceBlocking() {}

			virtual AKRESULT Init( const AkDeviceSettings & in_settings, AkDeviceID in_deviceID );
			virtual CAkStmTask * CreateStd( AkFileDesc * in_pFileDesc, AkOpenMode in_eOpenMode, IAkStdStream *& out_pStream );
			virtual CAkStmTask * CreateAuto( AkFileDesc * in_pFileDesc, AkFileID in_fileID, const AkAutoStmHeuristics & in_heuristics, AkAutoStmBufSettings * in_pBufferSettings, IAkAutoStream *& out_pStream );

			CAkStmMemView * CreateMemViewStd( CAkStmTask * in_pOwner, AkMemBlock * in_pMemBlock, AkUInt32 in_uDataOffset, AkUInt64 in_uPosition, AkUInt32 in_uBufferSize, AkUInt32 in_uRequestedSize, CAkLowLevelTransfer *& out_pLowLevelXfer );
			CAkStmMemView * CreateMemViewAuto( CAkStmTask * in_pOwner, AkFileID in_fileID, AkUInt64 in_uPosition, AkUInt32 in_uMinSize, AkUInt32 in_uRequiredAlign, AkUInt32 & io_uRequestedSize, CAkLowLevelTransfer *& out_pLowLevelXfer );

		protected:
			virtual void PerformIO();
			void ExecuteTask( CAkStmTask * in_pTask, AkReal32 in_fOpDeadline );

			CAkLowLevelTransferBlocking m_currentTransfer;
		};

		class CAkStdStmBlocking : public CAkStdStmBase
		{
		public:
			CAkStdStmBlocking() : m_pCurrentTransfer( NULL ), m_bTransferCancelling( false ) {}
			virtual ~CAkStdStmBlocking() {}

			virtual void Destroy();
			virtual void Cancel();
			virtual bool CanBeDestroyed();
			virtual CAkStmMemView * PrepareTransfer( AkFileDesc *& out_pFileDesc, CAkLowLevelTransfer *& out_pLowLevelXfer );
			virtual void Update( CAkStmMemView * in_pTransfer, AKRESULT in_eIOResult, bool in_bRequiredLowLevelXfer );

			CAkStmMemView *	m_pCurrentTransfer;
			bool			m_bTransferCancelling;
		};

		class CAkAutoStmBlocking : public CAkAutoStmBase
		{
		public:
			CAkAutoStmBlocking() : m_pCurrentTransfer( NULL ), m_bTransferCancelled( false ) {}
			virtual ~CAkAutoStmBlocking() {}

			virtual bool CanBeDestroyed();
			virtual CAkStmMemView * PrepareTransfer( AkFileDesc *& out_pFileDesc, CAkLowLevelTransfer *& out_pLowLevelXfer );
			virtual void Update( CAkStmMemView * in_pTransfer, AKRESULT in_eIOResult, bool in_bRequiredLowLevelXfer );

		protected:
			virtual AkUInt64 GetVirtualFilePosition();
			virtual void CancelPendingTransfers( AkUInt64 in_uMaxKeepPosition );
			virtual void FlushSmallBuffersAndPendingTransfers( AkUInt32 in_uMinBufferSize );
			virtual void RecomputeVirtualBuffering();
			void CancelCurrentTransfer();
			AkUInt32 ComputeVirtualBuffering();

			CAkStmMemView *	m_pCurrentTransfer;
			bool			m_bTransferCancelled;
		};

		/** CAkStreamMgr: the IAkStreamMgr singleton; the devices and observers are statics as in retail. */
		class CAkStreamMgr : public AK::IAkStreamMgr
		{
		public:
			struct LangChgObserver
			{
				AkLanguageChangeHandler	handler;
				void *					pCookie;
			};

			struct ArrayPoolLocal
			{
				static AkMemPoolId Get() { return m_streamMgrPoolId; }
			};

			static IAkStreamMgr * Create( const AkStreamMgrSettings & in_settings );
			virtual ~CAkStreamMgr() {}
			virtual void Destroy();
			virtual IAkStreamMgrProfile * GetStreamMgrProfile() { return NULL; }
			virtual AKRESULT CreateStd( AkFileID in_fileID, AkFileSystemFlags * in_pFSFlags, AkOpenMode in_eOpenMode, IAkStdStream *& out_pStream, bool in_bSyncOpen );
			virtual AKRESULT CreateStd( const AkOSChar * in_pszFileName, AkFileSystemFlags * in_pFSFlags, AkOpenMode in_eOpenMode, IAkStdStream *& out_pStream, bool in_bSyncOpen );
			virtual AKRESULT CreateAuto( AkFileID in_fileID, AkFileSystemFlags * in_pFSFlags, const AkAutoStmHeuristics & in_heuristics, AkAutoStmBufSettings * in_pBufferSettings, IAkAutoStream *& out_pStream, bool in_bSyncOpen );
			virtual AKRESULT CreateAuto( const AkOSChar * in_pszFileName, AkFileSystemFlags * in_pFSFlags, const AkAutoStmHeuristics & in_heuristics, AkAutoStmBufSettings * in_pBufferSettings, IAkAutoStream *& out_pStream, bool in_bSyncOpen );

			AkDeviceID CreateDevice( const AkDeviceSettings & in_settings, IAkLowLevelIOHook * in_pLowLevelHook );
			AKRESULT DestroyDevice( AkDeviceID in_deviceID );
			AKRESULT SetCurrentLanguage( const AkOSChar * in_pszLanguageName );
			AKRESULT AddLanguageChangeObserver( AkLanguageChangeHandler in_handler, void * in_pCookie );
			void RemoveLanguageChangeObserver( void * in_pCookie );
			static void ForceCleanup( CAkDeviceBase * in_pCallingDevice, AkPriority in_priority );

			static AkMemPoolId						m_streamMgrPoolId;
			static IAkFileLocationResolver *		m_pFileLocationResolver;
			static AkArray<CAkDeviceBase *, CAkDeviceBase *, ArrayPoolLocal, 1>			m_arDevices;
			static AkArray<LangChgObserver, LangChgObserver &, ArrayPoolLocal, 1>		m_arLangChgObserver;

		private:
			CAkDeviceBase * GetDevice( AkDeviceID in_deviceID );
			void Term();
		};

		inline AkMemPoolId ArrayPoolLocal::Get() { return CAkStreamMgr::m_streamMgrPoolId; }

		/** -akdiag: the stream manager's file-open log, routed through the backend's log hook. */
		void DiagFileOpen( const AkOSChar * in_pszFileName, AkFileID in_fileID, const AkFileSystemFlags * in_pFlags, AKRESULT in_eResult, const AkFileDesc * in_pDesc );
	}
}
