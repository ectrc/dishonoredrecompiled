/*=============================================================================
	arkanimnodeblendpose_dis.cpp: agent FZ (PHASE18 FZ) - the two UArkAnimNodeBlendPose calls the player MAT_* contract
	makes on ADishonoredPlayerPawn::m_pMatineeBlender. The class is an Engine-package class declared in
	DishonoredGameEngineShims.h (Engine/Src/arkanimnodeblendpose.cpp stays a skeleton for the same reason FK's
	interpgroupplayer.cpp does: an Engine unit cannot name it), so the bodies live here.

	SetEnable 2012 0x54a250 (56 B), BlendNow 0x54ffb0 (201 B); both unnamed in the 2013 database. The node's own
	TickAnim (0x550080, 729 B), SetActiveChild (0x550360, 1001 B) and InitAnim (0x1c3040) - the pose capture and the blend
	from it - are not ported, so these two only arm the node: m_bEnabled / m_fDisableTimer and the blend request.
=============================================================================*/
#include "DishonoredGame.h"

// the external-linkage anchor DishonoredGameRegistrants.cpp names (a static-library member nothing refers to is dropped whole)
void DishonoredLinkFZ_ArkAnimNodeBlendPose()
{
}

// enabling arms the node at once; disabling starts a one-second timer the (unported) TickAnim counts down
void UArkAnimNodeBlendPose::SetEnable( UBOOL bEnable )
{
	if( bEnable )
	{
		m_bEnabled = TRUE;
		m_fDisableTimer = -1.f;
	}
	else
	{
		m_fDisableTimer = 1.f;
	}
}

// only an enabled node holding a previous pose blends: child 0 (the live tree) to weight 0, child 1 (the captured pose) to 1, then back over BlendTime
void UArkAnimNodeBlendPose::BlendNow( FLOAT BlendTime, UBOOL bWaitForNextFrame )
{
	if( !m_bEnabled || m_PreviousPose.Num() == 0 || Children.Num() < 2 )
	{
		return;
	}
	m_bDoBlend = TRUE;
	Children(0).Weight = 0.f;
	Children(1).Weight = 1.f;
	ActiveChildIndex = 0;
	m_bMustBlendNextFrame = bWaitForNextFrame ? TRUE : FALSE;
	BlendTimeToGo = BlendTime;
}
