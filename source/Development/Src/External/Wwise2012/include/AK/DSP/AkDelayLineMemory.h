/*=============================================================================
	AkDelayLineMemory.h - AK::DSP::CAkDelayLineMemory<T, N> and CAkDelayLineMemoryStream<T, N> (Wwise 2012.1 SDK public
	header include/ak/dsp/akdelaylinememory.h). Agent IH4 (fork F2).

	One delay buffer per channel, its length rounded up to a multiple of 4 frames; <float,6> is 36 bytes and <float,1>
	16 bytes as the 2012 PDB. Retail instantiates <float,6> in the delay (2012 0x97c490 / 0x97c500 / 0x97c550,
	2013 0x978c90 / 0x978d00 / 0x978d50) and <float,1> in the pitch shifter and the stereo delay (2012 ctor 0x9966e0,
	0x997240 / 0x9972b0 / 0x997300, 2013 0x98df50 / 0x98dfc0 / 0x98e010). The stream variant adds no member in 2012.
=============================================================================*/
#ifndef _AK_DELAYLINEMEMORY_H_
#define _AK_DELAYLINEMEMORY_H_

#include <AK/SoundEngine/Common/IAkPlugin.h>
#include <string.h>

namespace AK
{
	namespace DSP
	{
		template < class T_SAMPLETYPE, AkUInt32 T_MAXNUMCHANNELS >
		class CAkDelayLineMemory
		{
		public:
			CAkDelayLineMemory()
				: m_uDelayLineLength( 0 )
				, m_uOffset( 0 )
				, m_uNumChannels( 0 )
			{
				for( AkUInt32 i = 0; i < T_MAXNUMCHANNELS; ++i )
				{
					m_pDelay[ i ] = NULL;
				}
			}

			AKRESULT Init( AK::IAkPluginMemAlloc * in_pAllocator, AkUInt32 in_uDelayLineLength, AkUInt32 in_uNumChannels )
			{
				m_uNumChannels = in_uNumChannels;
				m_uDelayLineLength = ( in_uDelayLineLength + 3 ) & ~3u;
				m_uOffset = 0;
				if( m_uDelayLineLength )
				{
					for( AkUInt32 i = 0; i < m_uNumChannels; ++i )
					{
						m_pDelay[ i ] = (T_SAMPLETYPE *)AK_PLUGIN_ALLOC( in_pAllocator, sizeof( T_SAMPLETYPE ) * m_uDelayLineLength );
						if( !m_pDelay[ i ] )
						{
							return AK_InsufficientMemory;
						}
					}
				}
				return AK_Success;
			}

			void Term( AK::IAkPluginMemAlloc * in_pAllocator )
			{
				for( AkUInt32 i = 0; i < m_uNumChannels; ++i )
				{
					if( m_pDelay[ i ] )
					{
						AK_PLUGIN_FREE( in_pAllocator, m_pDelay[ i ] );
						m_pDelay[ i ] = NULL;
					}
				}
				m_uDelayLineLength = 0;
			}

			void Reset()
			{
				if( m_uDelayLineLength )
				{
					for( AkUInt32 i = 0; i < m_uNumChannels; ++i )
					{
						if( m_pDelay[ i ] )
						{
							memset( m_pDelay[ i ], 0, sizeof( T_SAMPLETYPE ) * m_uDelayLineLength );
						}
					}
				}
				m_uOffset = 0;
			}

			AkUInt32 GetDelayLength() const { return m_uDelayLineLength; }
			T_SAMPLETYPE * GetCurrentPointer( AkUInt32 in_uOffset, AkUInt32 in_uChannelIndex ) { return m_pDelay[ in_uChannelIndex ] + in_uOffset; }
			T_SAMPLETYPE * GetChannel( AkUInt32 in_uChannelIndex ) { return m_pDelay[ in_uChannelIndex ]; }
			AkUInt32 GetCurrentOffset() const { return m_uOffset; }
			void SetCurrentOffset( AkUInt32 in_uOffset ) { m_uOffset = in_uOffset; }

		protected:
			T_SAMPLETYPE *	m_pDelay[ T_MAXNUMCHANNELS ];
			AkUInt32		m_uDelayLineLength;
			AkUInt32		m_uOffset;
			AkUInt32		m_uNumChannels;
		};

		template < class T_SAMPLETYPE, AkUInt32 T_MAXNUMCHANNELS >
		class CAkDelayLineMemoryStream : public CAkDelayLineMemory< T_SAMPLETYPE, T_MAXNUMCHANNELS >
		{
		};
	}
}

static_assert( sizeof( AK::DSP::CAkDelayLineMemory<float, 6> ) == 36, "CAkDelayLineMemory<float,6>: 2012 PDB sizeof 36" );
static_assert( sizeof( AK::DSP::CAkDelayLineMemory<float, 1> ) == 16, "CAkDelayLineMemory<float,1>: 2012 PDB sizeof 16" );
static_assert( sizeof( AK::DSP::CAkDelayLineMemoryStream<float, 6> ) == 36, "CAkDelayLineMemoryStream<float,6>: 2012 PDB sizeof 36" );

#endif
