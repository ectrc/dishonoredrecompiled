// DishonoredGame/src/dispostprocessmanager.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (21):
//   0x849590  public: static void __cdecl UDisPostProcessManager::InitializePrivateStaticClassUDisPostProcessManager(void)
//   0x8495b0  public: static void __cdecl UDisTweaks_PostProcess::InitializePrivateStaticClassUDisTweaks_PostProcess(void)
//   0x8495d0  public: unsigned int __thiscall UDisPostProcessManager::IsEffectRequired(enum eEffectPp)
//   0x8495f0  public: void __thiscall UDisPostProcessManager::StartEffect(enum eEffectPp, unsigned int)
//   0x849620  public: void __thiscall UDisPostProcessManager::StopEffect(enum eEffectPp)
//   0x849640  public: void __thiscall UDisPostProcessManager::Init(void)
//   0x849660  public: virtual void __thiscall UDisPostProcessManager::ApplyGameSettings(class ArkSettingsParameters const &, enum IArkSettingsListenerInterface::EChangeReason)
//   0x8496b0  public: void __thiscall UDisPostProcessManager::SetKismetPPParams(struct FArkUberPpParameters const &, float, float, float)
//   0x849700  public: void __thiscall UDisPostProcessManager::SetUIPPParams(struct FArkUberPpParameters const &, float, float, float)
//   0x84c1e0  public: void __thiscall UDisPostProcessManager::TickPossession(float)
//   0x84c400  public: virtual void __thiscall UDisPostProcessManager::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x84c5a0  public: virtual void __thiscall UDisPostProcessManager::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x8502e0  public: void __thiscall UDisPostProcessManager::TickZoomLens(float)
//   0x850970  public: void __thiscall UDisPostProcessManager::TickMaskOn(float)
//   0x850ef0  public: void __thiscall UDisPostProcessManager::ApplyKismetPostProcessSettings(struct FArkPpConfig &, float)
//   0x8511a0  public: void __thiscall UDisPostProcessManager::ApplyUIPostProcessSettings(struct FArkPpConfig &, float)
//   0x8556b0  public: static class UClass * __cdecl UDisPostProcessManager::GetPrivateStaticClassUDisPostProcessManager(wchar_t const *)
//   0x857400  public: static class UClass * __cdecl UDisPostProcessManager::StaticClassNoInline(void)
//   0x857430  public: void __thiscall UDisPostProcessManager::Tick(float)
//   0x85c540  public: static class UClass * __cdecl UDisTweaks_PostProcess::GetPrivateStaticClassUDisTweaks_PostProcess(wchar_t const *)
//   0x85d1c0  public: static class UClass * __cdecl UDisTweaks_PostProcess::StaticClassNoInline(void)

#include "DishonoredGame.h"
#include "arkpp.h"

// DISHONORED(port): agent EQ, 2013 rva 0x7e7e30 (2012 0x849660). The anti-aliasing option lands in three
// places: the manager's own m_PCAntialiasingType, GSystemSettings.iType_AntiAlias - which is an ini key, so
// this is the one option on the screen that writes itself to DishonoredEngine.ini - and, if the post-process
// graph has been built, the m_Type of the graph's AA node (m_PpBridge.m_PpNodeAA @324 of the object, the node's
// m_Type @104). An unrecognised value leaves the first two alone and still republishes to the node, which is
// retail's own control flow (its default arm jumps past the global assignment only).
void UDisPostProcessManager::ApplyGameSettings( const ArkSettingsParameters* Parameters, EChangeReason Reason )
{
	UBOOL bKnownMode = TRUE;
	switch( Parameters->m_AntiAliasingMode )
	{
	case 0:		m_PCAntialiasingType = 0; break;
	case 1:		m_PCAntialiasingType = 1; break;
	case 2:		m_PCAntialiasingType = 2; break;
	default:	bKnownMode = FALSE; break;
	}
	if( bKnownMode )
	{
		GSystemSettings.iType_AntiAlias = m_PCAntialiasingType;
	}
	if( m_PpBridge.m_PpNodeAA != NULL )
	{
		m_PpBridge.m_PpNodeAA->m_Type = m_PCAntialiasingType;
	}
}

/*-----------------------------------------------------------------------------
	Agent FA (PHASE12 package FA): the interface's own post-process channel.

	This is what "retail blurs the scene behind a modal" is made of, end to end:
	  UDisGFxMoviePlayerGlobal::UpdateMessageBoxAttributes sets m_bBlurGameWhileActive on the global movie,
	  UDisGlobalUIManager::OnMovieAttributesChanged walks every open movie and ORs that bit,
	  and when it is set it copies m_pBlurTweaks's uber parameters here and starts Epp_UberUI.
	The blur itself is the uber post-process's depth of field: the tweaks asset's m_Parameters carry an
	m_DOFParameters group, and blending it in at weight 1 is what puts the whole scene out of focus.
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x7e7ee0 (2012 0x849700).
void UDisPostProcessManager::SetUIPPParams( const FArkUberPpParameters& _rParameters, FLOAT _fWeight,
											FLOAT _fFadeInTime, FLOAT _fFadeOutTime )
{
	m_UIPPParams = _rParameters;
	m_UIPPWeight = _fWeight;
	m_UIPPFadeInTime = _fFadeInTime;
	m_UIPPFadeOutTime = _fFadeOutTime;
}

// DISHONORED(port): 2013 rva 0x7efca0 (2012 0x8511a0). A four-state fade - 0 off, 1 fading in, 2 full,
// 3 fading out - driven by m_UIStateDuration, with m_RequiredEffects[Epp_UberUI] as the request. The two
// cancel arms (1 while no longer wanted, 3 while wanted again) re-enter the opposite fade at the weight
// already reached rather than at its start, which is what keeps a box that is dismissed and raised again
// from snapping.
void UDisPostProcessManager::ApplyUIPostProcessSettings( FArkPpConfig& _rConfig, FLOAT _fDeltaTime )
{
	BYTE& State = m_EffectStates[Epp_UberUI];
	const INT Required = m_RequiredEffects[Epp_UberUI];
	if( State == 0 && Required != 1 )
	{
		return;
	}
	m_UIStateDuration += _fDeltaTime;
	FArkUberPpParameters& Dest = _rConfig.m_UberPpParameters;
	if( Required == 1 )
	{
		switch( State )
		{
		case 0:
			if( Abs( m_UIPPFadeInTime ) >= 1.0e-8f )
			{
				State = 1;
				m_UIStateDuration = 0.f;
			}
			else
			{
				State = 2;
				ArkUberPpApplyTo( m_UIPPParams, Dest, m_UIPPWeight, TRUE );
			}
			return;
		case 1:
			if( m_UIStateDuration < m_UIPPFadeInTime )
			{
				const FLOAT Alpha = Min( m_UIStateDuration / m_UIPPFadeInTime, 1.f );
				ArkUberPpApplyTo( m_UIPPParams, Dest, m_UIPPWeight * Alpha, TRUE );
				return;
			}
			State = 2;
			break;
		case 3:
			if( Abs( m_UIPPFadeInTime ) >= 1.0e-8f )
			{
				const FLOAT Alpha = Min( m_UIStateDuration / m_UIPPFadeOutTime, 1.f );
				State = 1;
				const FLOAT Weight = ( 1.f - Alpha ) * m_UIPPWeight;
				m_UIStateDuration = ( m_UIPPWeight != 0.f ) ? ( Weight / m_UIPPWeight ) * m_UIPPFadeInTime : 0.f;
				ArkUberPpApplyTo( m_UIPPParams, Dest, Weight, TRUE );
				return;
			}
			State = 2;
			break;
		default:
			break;
		}
		ArkUberPpApplyTo( m_UIPPParams, Dest, m_UIPPWeight, TRUE );
		return;
	}
	if( State == 2 )
	{
		if( Abs( m_UIPPFadeOutTime ) >= 1.0e-8f )
		{
			State = 3;
			m_UIStateDuration = 0.f;
			ArkUberPpApplyTo( m_UIPPParams, Dest, m_UIPPWeight, TRUE );
		}
		else
		{
			State = 0;
		}
		return;
	}
	if( State == 1 )
	{
		if( Abs( m_UIPPFadeOutTime ) < 1.0e-8f )
		{
			State = 0;
			return;
		}
		const FLOAT InAlpha = Min( m_UIStateDuration / m_UIPPFadeInTime, 1.f );
		State = 3;
		m_UIStateDuration = InAlpha * m_UIPPFadeOutTime;
		const FLOAT OutAlpha = Min( m_UIStateDuration / m_UIPPFadeOutTime, 1.f );
		ArkUberPpApplyTo( m_UIPPParams, Dest, ( 1.f - OutAlpha ) * m_UIPPWeight, TRUE );
		return;
	}
	if( m_UIStateDuration >= m_UIPPFadeOutTime )
	{
		State = 0;
		return;
	}
	const FLOAT Alpha = Min( m_UIStateDuration / m_UIPPFadeOutTime, 1.f );
	ArkUberPpApplyTo( m_UIPPParams, Dest, ( 1.f - Alpha ) * m_UIPPWeight, TRUE );
}

/*-----------------------------------------------------------------------------
	Agent FE (PHASE13 package FE): the level's own post-process channel.

	The menu is a level, and the brightness and bloom over it are a Kismet action in that level:
	DisSeqAct_UberPostProcess::Activated hands its FArkUberPpParameters to SetKismetPPParams and asks for
	Epp_UberKismet; this blends them into the frame's config at whatever weight the fade has reached.
	Retail's body is the twin of the UI channel FA ported, on the effect one index below it, with one
	difference that matters: ArkUberPpApplyTo's last argument is FALSE here and TRUE there.
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x7e7e90 (2012 0x8496b0). qmemcpy of the 96-byte parameter block into
// m_KismetPPParams @548, then weight @544, fade in @540, fade out @536 - the argument order is
// (parameters, weight, fadeIn, fadeOut) and retail writes them in the reverse of that order.
void UDisPostProcessManager::SetKismetPPParams( const FArkUberPpParameters& _rParameters, FLOAT _fWeight,
												FLOAT _fFadeInTime, FLOAT _fFadeOutTime )
{
	m_KismetPPParams = _rParameters;
	m_KismetPPWeight = _fWeight;
	m_KismetPPFadeInTime = _fFadeInTime;
	m_KismetPPFadeOutTime = _fFadeOutTime;
}

// DISHONORED(port): 2013 rva 0x7ef9b0 (2012 0x850ef0), 746 bytes. m_EffectStates[Epp_UberKismet] @462 is the
// state, m_RequiredEffects[Epp_UberKismet] @432 the request and m_KismetStateDuration @532 the clock; the two
// cancel arms re-enter the opposite fade at the weight already reached rather than at its start.
void UDisPostProcessManager::ApplyKismetPostProcessSettings( FArkPpConfig& _rConfig, FLOAT _fDeltaTime )
{
	BYTE& State = m_EffectStates[Epp_UberKismet];
	const INT Required = m_RequiredEffects[Epp_UberKismet];
	if( State == 0 && Required != 1 )
	{
		return;
	}
	m_KismetStateDuration += _fDeltaTime;
	FArkUberPpParameters& Dest = _rConfig.m_UberPpParameters;
	if( Required == 1 )
	{
		switch( State )
		{
		case 0:
			if( Abs( m_KismetPPFadeInTime ) >= 1.0e-8f )
			{
				State = 1;
				m_KismetStateDuration = 0.f;
			}
			else
			{
				State = 2;
				ArkUberPpApplyTo( m_KismetPPParams, Dest, m_KismetPPWeight, FALSE );
			}
			return;
		case 1:
			if( m_KismetStateDuration < m_KismetPPFadeInTime )
			{
				const FLOAT Alpha = Min( m_KismetStateDuration / m_KismetPPFadeInTime, 1.f );
				ArkUberPpApplyTo( m_KismetPPParams, Dest, m_KismetPPWeight * Alpha, FALSE );
				return;
			}
			State = 2;
			break;
		case 3:
			if( Abs( m_KismetPPFadeInTime ) >= 1.0e-8f )
			{
				const FLOAT Alpha = Min( m_KismetStateDuration / m_KismetPPFadeOutTime, 1.f );
				State = 1;
				const FLOAT Weight = ( 1.f - Alpha ) * m_KismetPPWeight;
				m_KismetStateDuration = ( m_KismetPPWeight != 0.f )
					? ( Weight / m_KismetPPWeight ) * m_KismetPPFadeInTime : 0.f;
				ArkUberPpApplyTo( m_KismetPPParams, Dest, Weight, FALSE );
				return;
			}
			State = 2;
			break;
		default:
			break;
		}
		ArkUberPpApplyTo( m_KismetPPParams, Dest, m_KismetPPWeight, FALSE );
		return;
	}
	if( State == 2 )
	{
		if( Abs( m_KismetPPFadeOutTime ) >= 1.0e-8f )
		{
			State = 3;
			m_KismetStateDuration = 0.f;
			ArkUberPpApplyTo( m_KismetPPParams, Dest, m_KismetPPWeight, FALSE );
		}
		else
		{
			State = 0;
		}
		return;
	}
	if( State == 1 )
	{
		if( Abs( m_KismetPPFadeOutTime ) < 1.0e-8f )
		{
			State = 0;
			return;
		}
		const FLOAT InAlpha = Min( m_KismetStateDuration / m_KismetPPFadeInTime, 1.f );
		State = 3;
		m_KismetStateDuration = InAlpha * m_KismetPPFadeOutTime;
		const FLOAT OutAlpha = Min( m_KismetStateDuration / m_KismetPPFadeOutTime, 1.f );
		ArkUberPpApplyTo( m_KismetPPParams, Dest, ( 1.f - OutAlpha ) * m_KismetPPWeight, FALSE );
		return;
	}
	if( m_KismetStateDuration >= m_KismetPPFadeOutTime )
	{
		State = 0;
		return;
	}
	const FLOAT Alpha = Min( m_KismetStateDuration / m_KismetPPFadeOutTime, 1.f );
	ArkUberPpApplyTo( m_KismetPPParams, Dest, ( 1.f - Alpha ) * m_KismetPPWeight, FALSE );
}
