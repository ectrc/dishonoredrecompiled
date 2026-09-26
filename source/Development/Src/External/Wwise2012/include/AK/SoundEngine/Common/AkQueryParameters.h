/*=============================================================================
	AkQueryParameters.h - Wwise 2012.1 AK::SoundEngine::Query surface.

	DISHONORED(written): the three Query entry points Arkane calls are
	  Query::QueryAudioObjectIDs   (UAkEvent::ComputeMaxRadius, 2013 rva 0xc7aa0, walks the event's
	                                audio-object tree to find the widest attenuation)
	  Query::GetPositioningInfo    (same function: the max distance of each object)
	  Query::GetIsGameObjectActive (UDishonoredAudioSystem)
	The rest of the namespace is declared because the signatures were free in the PDB and the SDK
	header is one unit; the silent backend answers them all from its own bookkeeping.
	AkArray-based overloads (Query::GetActiveGameObjects, the GameObjDst form of GetMaxRadius) are NOT
	written: AkArray is Wwise's own container and nothing in Dishonored's engine-facing code calls them.
=============================================================================*/
#ifndef _AK_QUERY_PARAMETERS_H_
#define _AK_QUERY_PARAMETERS_H_

#include <AK/SoundEngine/Common/AkTypes.h>

namespace AK
{
	namespace SoundEngine
	{
		namespace Query
		{
			/** Where a queried RTPC value came from. */
			enum RTPCValue_type
			{
				RTPCValue_Default		= 0,
				RTPCValue_Global		= 1,
				RTPCValue_GameObject	= 2,
				RTPCValue_Unavailable	= 3
			};

			AKRESULT GetPosition( AkGameObjectID in_GameObjectID, AkSoundPosition & out_rPosition );	///< 2012 rva 0x91b250
			AKRESULT GetListenerPosition( AkUInt32 in_uIndex, AkListenerPosition & out_rPosition );	///< 2012 rva 0x91af20
			AKRESULT GetListenerSpatialization( AkUInt32 in_uIndex, bool & out_rbSpatialized, AkSpeakerVolumes & out_rVolumeOffsets );	///< 2012 rva 0x91af50
			AKRESULT GetActiveListeners( AkGameObjectID in_GameObjectID, AkUInt32 & out_ruListenerMask );	///< 2012 rva 0x91b2f0
			bool GetIsGameObjectActive( AkGameObjectID in_GameObjectID );							///< 2012 rva 0x91b120
			AkReal32 GetMaxRadius( AkGameObjectID in_GameObjectID );								///< 2012 rva 0x91b180

			AKRESULT GetRTPCValue( AkRtpcID in_rtpcID, AkGameObjectID in_gameObjectID, AkRtpcValue & out_rValue, RTPCValue_type & io_rValueType );	///< 2012 rva 0x91b360
			AKRESULT GetRTPCValue( const AkOSChar * in_pszRtpcName, AkGameObjectID in_gameObjectID, AkRtpcValue & out_rValue, RTPCValue_type & io_rValueType );	///< 2012 rva 0x91b440
			AKRESULT GetRTPCValue( const char * in_pszRtpcName, AkGameObjectID in_gameObjectID, AkRtpcValue & out_rValue, RTPCValue_type & io_rValueType );	///< 2012 rva 0x91b460

			AKRESULT GetSwitch( AkSwitchGroupID in_switchGroup, AkGameObjectID in_gameObjectID, AkSwitchStateID & out_rSwitchState );	///< 2012 rva 0x91b480
			AKRESULT GetSwitch( const AkOSChar * in_pstrSwitchGroupName, AkGameObjectID in_GameObj, AkSwitchStateID & out_rSwitchState );	///< 2012 rva 0x91b500
			AKRESULT GetSwitch( const char * in_pstrSwitchGroupName, AkGameObjectID in_GameObj, AkSwitchStateID & out_rSwitchState );	///< 2012 rva 0x91b520

			AKRESULT GetState( AkStateGroupID in_stateGroup, AkStateID & out_rState );			///< 2012 rva 0x91af90
			AKRESULT GetState( const AkOSChar * in_pstrStateGroupName, AkStateID & out_rState );	///< 2012 rva 0x91afd0
			AKRESULT GetState( const char * in_pstrStateGroupName, AkStateID & out_rState );		///< 2012 rva 0x91b010

			AKRESULT GetEnvironmentVolumes( AkAuxBusID in_envID, AkSpeakerVolumes & out_rVolumes );	///< 2012 rva 0x91b050
			AKRESULT GetEnvironmentBypass( AkAuxBusID in_envID, bool & out_rbBypassed );			///< 2012 rva 0x91b0b0
			AKRESULT GetEnvironmentVolume( AkAuxBusID in_envID, AkReal32 & out_rVolume );			///< 2012 rva 0x91b220
			AKRESULT GetGameObjectEnvironmentsValues( AkGameObjectID in_gameObjectID, AkEnvironmentValue * out_paEnvironmentValues, AkUInt32 & io_ruNumEnvValues );	///< 2012 rva 0x91b540
			AKRESULT GetGameObjectDryLevelValue( AkGameObjectID in_gameObjectID, AkReal32 & out_rfControlValue );	///< 2012 rva 0x91b610
			AKRESULT GetObjectObstructionAndOcclusion( AkGameObjectID in_ObjectID, AkUInt32 in_uListener, AkReal32 & out_rfObstructionLevel, AkReal32 & out_rfOcclusionLevel );	///< 2012 rva 0x91b680

			AkUniqueID GetEventIDFromPlayingID( AkPlayingID in_playingID );						///< 2012 rva 0x91b1b0
			AkGameObjectID GetGameObjectFromPlayingID( AkPlayingID in_playingID );				///< 2012 rva 0x91b1d0
			AKRESULT GetPlayingIDsFromGameObject( AkGameObjectID in_GameObjId, AkUInt32 & io_ruNumIDs, AkPlayingID * out_aPlayingIDs );	///< 2012 rva 0x91b1f0

			AKRESULT QueryAudioObjectIDs( AkUniqueID in_eventID, AkUInt32 & io_ruNumItems, AkObjectInfo * out_aObjectInfos );	///< 2012 rva 0x91b720
			AKRESULT QueryAudioObjectIDs( const AkOSChar * in_pszEventName, AkUInt32 & io_ruNumItems, AkObjectInfo * out_aObjectInfos );	///< 2012 rva 0x91b7a0
			AKRESULT QueryAudioObjectIDs( const char * in_pszEventName, AkUInt32 & io_ruNumItems, AkObjectInfo * out_aObjectInfos );	///< 2012 rva 0x91b7c0
			AKRESULT GetPositioningInfo( AkUniqueID in_ObjectID, AkPositioningInfo & out_rPositioningInfo );	///< 2012 rva 0x91b7e0
		}
	}
}

#endif // _AK_QUERY_PARAMETERS_H_
