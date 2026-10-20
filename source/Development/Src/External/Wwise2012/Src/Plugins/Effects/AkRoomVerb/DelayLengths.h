/*=============================================================================
	DelayLengths.h - DelayLengths (plugins\effects\akroomverb\delaylengths.h): the delay times of the RoomVerb's FDN lines,
	reverb-unit input delays and diffusion allpasses (seeded linear-congruential jitter), made prime in samples.

	DISHONORED(port): agent IH4 fork F1 (wave 26), 2012 0x9886b0..0x988d70 (AkDelayQSortCompareFloat,
	ComputeFDNDelayTimes, ComputeFDNInputDelayTimes, ComputeDiffusionFiltersDelayTimes, MakePrimeNumber,
	ComputePrimeDelayLengths). The tunings retail passes in a register are an explicit argument; the 4x unroll of the
	time loops is not reproduced (same arithmetic per line).
=============================================================================*/
#pragma once

#include "AkRoomVerbFXParams.h"

namespace DelayLengths
{
	void ComputeFDNDelayTimes( const AlgorithmTunings & in_Tunings, AkReal32 in_fDensity, AkReal32 in_fRoomShape, AkUInt32 in_uNumDelayLines, AkReal32 * out_pfFDNDelayTimes );
	void ComputeFDNInputDelayTimes( const AlgorithmTunings & in_Tunings, AkUInt32 in_uNumDelayLines, AkReal32 * out_pfFDNInputDelayTimes );
	void ComputeDiffusionFiltersDelayTimes( const AlgorithmTunings & in_Tunings, AkUInt32 in_uNumReverbUnits, AkReal32 * out_pfAPFDelayTimes );
	void MakePrimeNumber( AkUInt32 & io_uIn, AkUInt32 in_uPrevVal );
	void ComputePrimeDelayLengths( AkUInt32 in_uSampleRate, AkUInt32 * out_puDelayLengths, const AkReal32 * in_pfDelayTimes, AkUInt32 in_uNumDelays );
}
