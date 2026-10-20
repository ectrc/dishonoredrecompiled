/*=============================================================================
	AkFXParameterChangeHandler.h - the SDK public plug-in service include/AK/Plugin/PluginServices/
	AkFXParameterChangeHandler.h (2012.1): one bit per parameter id, set by SetParam, read and cleared by Execute.

	DISHONORED(port): agent IH4 fork F1 (wave 26). AK::AkFXParameterChangeHandler<11> is 2 bytes in the PDB;
	HasChanged 2012 0x9904a0 (the other members inline).
=============================================================================*/
#ifndef _AK_FX_PARAMETER_CHANGE_HANDLER_H_
#define _AK_FX_PARAMETER_CHANGE_HANDLER_H_

#include <AK/SoundEngine/Common/IAkPlugin.h>
#include <string.h>

namespace AK
{
	template <AkUInt32 T_MAXNUMPARAMS>
	class AkFXParameterChangeHandler
	{
	public:
		AkFXParameterChangeHandler() { ResetAllParamChanges(); }

		void SetParamChange( AkPluginParamID in_ID )
		{
			m_uParamBitArray[ in_ID / 8 ] |= ( 1 << ( in_ID % 8 ) );
		}

		/** 2012 0x9904a0. */
		bool HasChanged( AkPluginParamID in_ID )
		{
			return ( m_uParamBitArray[ in_ID / 8 ] & ( 1 << ( in_ID % 8 ) ) ) != 0;
		}

		bool HasAnyChanged()
		{
			for( AkUInt32 i = 0; i < sizeof( m_uParamBitArray ); ++i )
			{
				if( m_uParamBitArray[ i ] )
				{
					return true;
				}
			}
			return false;
		}

		void ResetParamChange( AkPluginParamID in_ID )
		{
			m_uParamBitArray[ in_ID / 8 ] &= ~( 1 << ( in_ID % 8 ) );
		}

		void ResetAllParamChanges() { memset( m_uParamBitArray, 0, sizeof( m_uParamBitArray ) ); }
		void SetAllParamChanges() { memset( m_uParamBitArray, 0xFF, sizeof( m_uParamBitArray ) ); }

		AkUInt8 m_uParamBitArray[ ( T_MAXNUMPARAMS + 7 ) / 8 ];
	};
}

#endif
