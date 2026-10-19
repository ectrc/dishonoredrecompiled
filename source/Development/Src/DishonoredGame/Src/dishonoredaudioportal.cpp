// DishonoredGame/src/dishonoredaudioportal.cpp
// PDB functions attributed to this file (7):
//   0x7f3dc0  public: static void __cdecl ADishonoredAudioPortal::InitializePrivateStaticClassADishonoredAudioPortal(void)
//   0x7f3de0  public: virtual void __thiscall ADishonoredAudioPortal::GetVolumes(class AInOutVolume * *, class AInOutVolume * *)
//   0x7f5080  public: void __thiscall ADishonoredAudioPortal::GetNearestPoint(class FVector const &, class FVector &)const
//   0x7f5200  public: unsigned int __thiscall ADishonoredAudioPortal::CanConsiderPointInOtherCell(class FVector, class ADishonoredAudioVolume *)
//   0x7f8280  public: void __thiscall ADishonoredAudioPortal::GetNearestPoint(class FVector const &, class FVector const &, class FVector &)const
//   0x814a10  public: static class UClass * __cdecl ADishonoredAudioPortal::GetPrivateStaticClassADishonoredAudioPortal(wchar_t const *)
//   0x816ce0  public: static class UClass * __cdecl ADishonoredAudioPortal::StaticClassNoInline(void)
//
// DISHONORED(port): agent IH3 (fork G). 2013 rvas (2012 in parentheses): GetVolumes (0x7f3de0), GetNearestPoint 0x78be30 (0x7f5080),
// CanConsiderPointInOtherCell 0x786e20 (0x7f5200), GetNearestPoint through a segment (0x7f8280, 1,988 B). The portal is the
// rectangle Location +- m_fHalfWidth * m_WidthDir +- m_fHalfHeight * m_HeightDir in the plane of m_PlaneNormal, whose
// corners are m_Corners[0..3]; m_pCellA is on the side the normal points to.

#include "DishonoredGame.h"

void ADishonoredAudioPortal::GetVolumes( AInOutVolume** _pVolA, AInOutVolume** _pVolB )
{
	*_pVolA = m_pCellA;
	*_pVolB = m_pCellB;
}

/** The point of the portal rectangle nearest to a point: its offset along the width and the height clamped to the half sizes. */
void ADishonoredAudioPortal::GetNearestPoint( const FVector& _rPoint, FVector& _rNearestPoint ) const
{
	const FVector RelativePos = _rPoint - Location;
	const FLOAT fWidth = Clamp( m_WidthDir | RelativePos, -m_fHalfWidth, m_fHalfWidth );
	const FLOAT fHeight = Clamp( m_HeightDir | RelativePos, -m_fHalfHeight, m_fHalfHeight );
	_rNearestPoint = Location + m_WidthDir * fWidth + m_HeightDir * fHeight;
}

/**
 * The point of the portal nearest to the segment from a point in cell A to a point in cell B. A segment parallel to the plane
 * takes the portal point nearest to the segment's point nearest to the portal centre. When both points are on the good side
 * (cell A in front of the plane, cell B behind it) the segment crosses the plane: inside the rectangle that crossing is the
 * answer, outside it the nearest point of the one or two rectangle edges it passed (the nearer of the two in a corner). A point
 * on the wrong side snaps to its nearest point on the rectangle's outline; both wrong takes the middle of the two.
 */
void ADishonoredAudioPortal::GetNearestPoint( const FVector& _rPointInCellA, const FVector& _rPointInCellB, FVector& _rNearestPoint ) const
{
	const FVector Segment = _rPointInCellB - _rPointInCellA;
	const FLOAT fSegmentDot = m_PlaneNormal | Segment;
	if( Abs( fSegmentDot ) < 1e-8f )
	{
		FVector SegmentPoint;
		PointDistToSegment( Location, _rPointInCellA, _rPointInCellB, SegmentPoint );
		GetNearestPoint( SegmentPoint, _rNearestPoint );
		return;
	}
	const FLOAT fDistA = m_PlaneNormal | ( Location - _rPointInCellA );
	const FLOAT fDistB = m_PlaneNormal | ( _rPointInCellB - Location );
	const UBOOL bPointAOnGoodSide = fDistA > 0.f;
	const UBOOL bPointBOnGoodSide = fDistB > 0.f;
	FVector NearestPoint;
	FVector NearestPointB;
	if( !bPointAOnGoodSide || !bPointBOnGoodSide )
	{
		if( !bPointAOnGoodSide )
		{
			FVector EdgePoint;
			FLOAT fSegmentDist = PointDistToSegment( _rPointInCellA, m_Corners[0], m_Corners[1], NearestPoint );
			for( INT Corner = 1; Corner < 4; Corner++ )
			{
				const FLOAT fSegmentDistTmp = PointDistToSegment( _rPointInCellA, m_Corners[Corner], m_Corners[( Corner + 1 ) & 3], EdgePoint );
				if( fSegmentDist > fSegmentDistTmp )
				{
					fSegmentDist = fSegmentDistTmp;
					NearestPoint = EdgePoint;
				}
			}
			if( bPointBOnGoodSide )
			{
				_rNearestPoint = NearestPoint;
				return;
			}
		}
		FVector EdgePoint;
		FLOAT fSegmentDist = PointDistToSegment( _rPointInCellB, m_Corners[0], m_Corners[1], NearestPointB );
		for( INT Corner = 1; Corner < 4; Corner++ )
		{
			const FLOAT fSegmentDistTmp = PointDistToSegment( _rPointInCellB, m_Corners[Corner], m_Corners[( Corner + 1 ) & 3], EdgePoint );
			if( fSegmentDist > fSegmentDistTmp )
			{
				fSegmentDist = fSegmentDistTmp;
				NearestPointB = EdgePoint;
			}
		}
		_rNearestPoint = bPointAOnGoodSide ? NearestPointB : ( NearestPoint + NearestPointB ) * 0.5f;
		return;
	}
	NearestPoint = _rPointInCellA + Segment * ( fDistA / fSegmentDot );
	const FVector RelativePos = NearestPoint - Location;
	const FLOAT fWidth = m_WidthDir | RelativePos;
	const FLOAT fHeight = m_HeightDir | RelativePos;
	FVector SegmentPointW, RectanglePointW, SegmentPointH, RectanglePointH;
	INT Outside = 0;
	if( fWidth > m_fHalfWidth )
	{
		SegmentDistToSegmentSafe( _rPointInCellA, _rPointInCellB, m_Corners[1], m_Corners[2], SegmentPointW, RectanglePointW );
		Outside = 1;
	}
	else if( -m_fHalfWidth > fWidth )
	{
		SegmentDistToSegmentSafe( _rPointInCellA, _rPointInCellB, m_Corners[0], m_Corners[3], SegmentPointW, RectanglePointW );
		Outside = 1;
	}
	if( fHeight > m_fHalfHeight )
	{
		SegmentDistToSegmentSafe( _rPointInCellA, _rPointInCellB, m_Corners[2], m_Corners[3], SegmentPointH, RectanglePointH );
		Outside += 2;
	}
	else if( -m_fHalfHeight > fHeight )
	{
		SegmentDistToSegmentSafe( _rPointInCellA, _rPointInCellB, m_Corners[0], m_Corners[1], SegmentPointH, RectanglePointH );
		Outside += 2;
	}
	switch( Outside )
	{
	case 0:
		_rNearestPoint = NearestPoint;
		break;
	case 1:
		_rNearestPoint = RectanglePointW;
		break;
	case 2:
		_rNearestPoint = RectanglePointH;
		break;
	case 3:
		_rNearestPoint = ( RectanglePointH - SegmentPointH ).SizeSquared() <= ( RectanglePointW - SegmentPointW ).SizeSquared() ? RectanglePointH : RectanglePointW;
		break;
	}
}

/** A point within the portal's rectangle on the other side of it than its cell, and inside the other cell's volume. */
UBOOL ADishonoredAudioPortal::CanConsiderPointInOtherCell( FVector _Point, ADishonoredAudioVolume* _pCell )
{
	const FVector RelativePos = _Point - Location;
	const FLOAT fPlaneDist = m_PlaneNormal | RelativePos;
	if( ( ( _pCell == m_pCellA && fPlaneDist > 0.f ) || ( _pCell == m_pCellB && fPlaneDist < 0.f ) )
		&& m_fHalfWidth >= Abs( m_WidthDir | RelativePos ) && m_fHalfHeight >= Abs( m_HeightDir | RelativePos ) )
	{
		ADishonoredAudioVolume* OtherCell = _pCell == m_pCellA ? m_pCellB : m_pCellA;
		if( OtherCell->Encompasses( _Point ) )
		{
			return TRUE;
		}
	}
	return FALSE;
}
