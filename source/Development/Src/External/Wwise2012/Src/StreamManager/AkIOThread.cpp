/*=============================================================================
	AkIOThread.cpp - CAkIOThread ("AK::IOThread") and CAkClientThreadAware.

	DISHONORED(port): agent IH1 (wave 26); wwise\sdk\source\streammanager\win32\akiothread.cpp (18 functions,
	2012 0x974010..0x974470) plus AKPLATFORM::AkCreateThread / AkSetThreadName (akplatformfuncs.h,
	2012 0x9297e0 / 0x929760).
	The thread waits on {stop, below-max-IO gate}, then on {stop, std semaphore, auto semaphore} (all manual
	reset events), and calls PerformIO while fewer than m_uMaxConcurrentIO transfers are in flight. On stop it
	retries ClearStreams every 100 ms until every task has been destroyed. Thread priority, stack size and
	affinity come from AkDeviceSettings::threadProperties exactly as retail passes them.
=============================================================================*/
#include "AkStreamMgrPrivate.h"

namespace AKPLATFORM
{
	/** 2012 0x929760: the MSVC thread-naming exception (0x406D1388), swallowed when no debugger listens. */
	void AkSetThreadName( DWORD in_dwThreadID, const char * in_szThreadName )
	{
#pragma pack(push, 8)
		struct THREADNAME_INFO
		{
			DWORD	dwType;
			LPCSTR	szName;
			DWORD	dwThreadID;
			DWORD	dwFlags;
		};
#pragma pack(pop)
		THREADNAME_INFO info;
		info.dwType = 0x1000;
		info.szName = in_szThreadName;
		info.dwThreadID = in_dwThreadID;
		info.dwFlags = 0;
		__try
		{
			RaiseException( 0x406D1388, 0, sizeof( info ) / sizeof( ULONG_PTR ), (ULONG_PTR *)&info );
		}
		__except( EXCEPTION_EXECUTE_HANDLER )
		{
		}
	}

	/** 2012 0x9297e0. */
	void AkCreateThread( DWORD ( WINAPI * in_pStartRoutine )( LPVOID ), void * in_pParams, const AkThreadProperties & in_threadProperties, AkThread * out_pThread, const char * in_szThreadName )
	{
		DWORD dwThreadID = 0;
		*out_pThread = CreateThread( NULL, in_threadProperties.uStackSize, in_pStartRoutine, in_pParams, 0, &dwThreadID );
		if( !*out_pThread )
		{
			*out_pThread = NULL;
			return;
		}
		AkSetThreadName( dwThreadID, in_szThreadName );
		if( !SetThreadPriority( *out_pThread, in_threadProperties.nPriority ) )
		{
			CloseHandle( *out_pThread );
			*out_pThread = NULL;
			return;
		}
		if( in_threadProperties.dwAffinityMask && !SetThreadAffinityMask( *out_pThread, in_threadProperties.dwAffinityMask ) )
		{
			CloseHandle( *out_pThread );
			*out_pThread = NULL;
		}
	}
}

namespace AK
{
	namespace StreamMgr
	{
		CAkClientThreadAware::CAkClientThreadAware()
			: m_hBlockEvent( NULL )
			, m_bIsBlocked( false )
		{
		}

		CAkClientThreadAware::~CAkClientThreadAware()
		{
			if( m_hBlockEvent )
			{
				CloseHandle( m_hBlockEvent );
				m_hBlockEvent = NULL;
			}
		}

		void CAkClientThreadAware::SetBlockedStatus()
		{
			if( m_hBlockEvent )
			{
				ResetEvent( m_hBlockEvent );
			}
			else
			{
				m_hBlockEvent = CreateEventW( NULL, TRUE, FALSE, NULL );
			}
			m_bIsBlocked = true;
		}

		CAkIOThread::CAkIOThread()
			: m_uMaxConcurrentIO( 0 )
			, m_hIOThread( NULL )
			, m_hIOThreadStopEvent( NULL )
			, m_hMaxIOGate( NULL )
			, m_hStdSem( NULL )
			, m_hAutoSem( NULL )
			, m_cPendingStdStms( 0 )
			, m_cRunningAutoStms( 0 )
			, m_bDoWaitMemoryChange( false )
			, m_uNumConcurrentIO( 0 )
		{
		}

		CAkIOThread::~CAkIOThread()
		{
		}

		AKRESULT CAkIOThread::Init( const AkThreadProperties & in_threadProperties )
		{
			m_hStdSem = CreateEventW( NULL, TRUE, FALSE, NULL );
			m_hAutoSem = CreateEventW( NULL, TRUE, FALSE, NULL );
			m_hIOThreadStopEvent = CreateEventW( NULL, TRUE, FALSE, NULL );
			m_hMaxIOGate = CreateEventW( NULL, TRUE, TRUE, NULL );
			m_cPendingStdStms = 0;
			m_cRunningAutoStms = 0;
			m_uNumConcurrentIO = 0;
			AKPLATFORM::AkCreateThread( IOSchedThread, this, in_threadProperties, &m_hIOThread, "AK::IOThread" );
			if( !m_hIOThread || !m_hIOThreadStopEvent || !m_hStdSem || !m_hAutoSem )
			{
				return AK_Fail;
			}
			return AK_Success;
		}

		void CAkIOThread::Term()
		{
			if( m_hIOThreadStopEvent )
			{
				SetEvent( m_hIOThreadStopEvent );
				if( m_hIOThread )
				{
					WaitForSingleObject( m_hIOThread, INFINITE );
					CloseHandle( m_hIOThread );
					m_hIOThread = NULL;
				}
				CloseHandle( m_hIOThreadStopEvent );
				m_hIOThreadStopEvent = NULL;
			}
			if( m_hIOThread )
			{
				CloseHandle( m_hIOThread );
				m_hIOThread = NULL;
			}
			if( m_hStdSem )
			{
				CloseHandle( m_hStdSem );
				m_hStdSem = NULL;
			}
			m_cPendingStdStms = 0;
			if( m_hAutoSem )
			{
				CloseHandle( m_hAutoSem );
				m_hAutoSem = NULL;
			}
			m_cRunningAutoStms = 0;
			if( m_hMaxIOGate )
			{
				CloseHandle( m_hMaxIOGate );
				m_hMaxIOGate = NULL;
			}
		}

		/** 2012 0x974250. */
		DWORD WINAPI CAkIOThread::IOSchedThread( LPVOID lpParameter )
		{
			CAkIOThread * pIOThread = (CAkIOThread *)lpParameter;
			HANDLE pEventStopOrBelowMaxIO[ 2 ] = { pIOThread->m_hIOThreadStopEvent, pIOThread->m_hMaxIOGate };
			HANDLE pEvents[ 3 ] = { pIOThread->m_hIOThreadStopEvent, pIOThread->m_hStdSem, pIOThread->m_hAutoSem };
			pIOThread->OnThreadStart();
			for( ;; )
			{
				DWORD dwWaitRes = WaitForMultipleObjectsEx( 2, pEventStopOrBelowMaxIO, FALSE, INFINITE, TRUE );
				if( dwWaitRes != WAIT_OBJECT_0 )
				{
					dwWaitRes = WaitForMultipleObjectsEx( 3, pEvents, FALSE, INFINITE, TRUE );
				}
				if( dwWaitRes == WAIT_IO_COMPLETION )
				{
					continue;
				}
				if( dwWaitRes == WAIT_OBJECT_0 )
				{
					if( pIOThread->ClearStreams() )
					{
						return 0;
					}
					SleepEx( 100, TRUE );
					continue;
				}
				if( dwWaitRes > WAIT_OBJECT_0 + 2 && dwWaitRes != WAIT_TIMEOUT )
				{
					return 1;
				}
				pIOThread->m_lockSems.Lock();
				const bool bBelowMaxIO = pIOThread->m_uNumConcurrentIO < pIOThread->m_uMaxConcurrentIO;
				pIOThread->m_lockSems.Unlock();
				if( bBelowMaxIO )
				{
					pIOThread->PerformIO();
				}
			}
		}

		void CAkIOThread::NotifyMemChange()
		{
			if( m_bDoWaitMemoryChange )
			{
				m_bDoWaitMemoryChange = false;
				if( m_cRunningAutoStms > 0 )
				{
					SetEvent( m_hAutoSem );
				}
			}
		}

		void CAkIOThread::NotifyMemIdle()
		{
			m_bDoWaitMemoryChange = true;
			if( m_cRunningAutoStms > 0 )
			{
				ResetEvent( m_hAutoSem );
			}
		}

		void CAkIOThread::WaitForIOCompletion( CAkClientThreadAware * in_pWaitingTask )
		{
			WaitForSingleObject( in_pWaitingTask->m_hBlockEvent, INFINITE );
		}

		void CAkIOThread::SignalIOCompleted( CAkClientThreadAware * in_pWaitingTask )
		{
			in_pWaitingTask->ClearBlockedStatus();
			SetEvent( in_pWaitingTask->m_hBlockEvent );
		}

		void CAkIOThread::StdSemIncr()
		{
			AkAutoLock<CAkLock> gate( m_lockSems );
			if( ++m_cPendingStdStms == 1 )
			{
				SetEvent( m_hStdSem );
			}
		}

		void CAkIOThread::StdSemDecr()
		{
			AkAutoLock<CAkLock> gate( m_lockSems );
			if( m_cPendingStdStms-- == 1 )
			{
				ResetEvent( m_hStdSem );
			}
		}

		void CAkIOThread::AutoSemIncr()
		{
			AkAutoLock<CAkLock> gate( m_lockSems );
			if( ++m_cRunningAutoStms == 1 && !m_bDoWaitMemoryChange )
			{
				SetEvent( m_hAutoSem );
			}
		}

		void CAkIOThread::AutoSemDecr()
		{
			AkAutoLock<CAkLock> gate( m_lockSems );
			if( m_cRunningAutoStms-- == 1 )
			{
				ResetEvent( m_hAutoSem );
			}
		}

		void CAkIOThread::IncrementIOCount()
		{
			AkAutoLock<CAkLock> gate( m_lockSems );
			if( ++m_uNumConcurrentIO >= m_uMaxConcurrentIO )
			{
				ResetEvent( m_hMaxIOGate );
			}
		}

		void CAkIOThread::DecrementIOCount()
		{
			AkAutoLock<CAkLock> gate( m_lockSems );
			if( --m_uNumConcurrentIO == m_uMaxConcurrentIO - 1 )
			{
				SetEvent( m_hMaxIOGate );
			}
		}
	}
}
