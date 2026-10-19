// agent IC (PHASE23 IC): -icdiag - every stim a brain takes (id, soiree start/stop and priority) with the behaviour stack after it,
// every change of the current behaviour, the matinee controller list, the soiree behaviour's start/stop, and (build 2) the FaceTo
// requests. Defined in Src/dishonoredaibrain.cpp.
#ifndef DISHONORED_ICDIAG_H
#define DISHONORED_ICDIAG_H

UBOOL DisICDiagEnabled();
void DisICDiagStim( class UDishonoredAIBrain* _pBrain, const struct FAIStimStruct& _rStim );
void DisICDiagCurrentBehavior( class UDishonoredAIBrain* _pBrain, class UDishonoredAIBehavior* _pPrevious );

#endif
