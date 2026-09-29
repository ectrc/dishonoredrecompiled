#pragma once
// DishonoredGame/inc/dishonoredutilities_saveload_ai.h
// DISHONORED(port): agent EJ (PHASE12 EJ). Retail's own header for the two AI sub-tweak reference serialisers;
// the PDB attributes them here, at lines 17 and 42, and they are the only two functions it attributes to this
// file.
//
// A sub-tweak (a UDisTweaks_AISubState, _AISubProcess or _AIBrainProcess) is a cooked package object that the
// save cannot name by dictionary index, because the dictionary only carries the level's own objects. Retail
// therefore writes it as the *owning* tweak object plus the index of the sub-tweak inside one named array of
// that owner - which is why the third argument is a pointer to member.
//
// PDB functions attributed to this file (2):
//   2012 0x771980 / 2013 0x7312a0  DisSaveAISubTweakReference<UDisTweaks_AISubState, UDisTweaks_AIBehavior>
//   2012 0x7719f0 / 2013 0x731310  DisLoadAISubTweakReference<UDisTweaks_AIBrainProcess, UDisTweaks_AIBrain>
//
// Only the loading half is defined, for agent ED's reason: the writing half of the DisSaveLoad object layer
// does not exist in this tree, so a DisSaveAISubTweakReference would have no caller. The retail body is the
// mirror of this one - the owner, then the index the owner's array holds the tweak at, or INDEX_NONE.
//
// DISHONORED(bringup): the linker folds every instantiation of each template onto one address (all three take
// a TArray<T*> by member pointer and do the same two reads), which is why the 2012 PDB lists one name per
// template although three classes use them. The demangled name IDA and match_2012_2013.csv show for
// 0x731310 - the <UDisTweaks_AIBrainProcess, UDisTweaks_AIBrain> instantiation - is therefore the fold's
// representative and not the only caller: UDisAISubState and UDisAISubProcess reach the same body with
// <UDisTweaks_AISubState, UDisTweaks_AIBehavior> and <UDisTweaks_AISubProcess, UDisTweaks_AIBehavior>.

#ifndef _INC_DISHONOREDUTILITIES_SAVELOAD_AI
#define _INC_DISHONOREDUTILITIES_SAVELOAD_AI

/** DISHONORED(port): 2013 rva 0x731310 (2012 0x7719f0). The owning tweak object goes through the archive's own
    object reference slot (retail's FArchive vtable offset 24, which is operator<<(UObject*&) and not
    operator<<(FName&) - see the note in dissavegame.cpp), then an INT index into the owner's array. An owner
    the save could not resolve, a negative index or an index past the array leaves the reference as it was:
    retail assigns only on the way out of the bounds test. */
template< class T, class TOwner >
void DisLoadAISubTweakReference( FArchive& _rArchive, T*& _rpOutTweak, TArrayNoInit<T*> TOwner::* _pOwnerArray )
{
	TOwner* pOwner = NULL;
	INT TweakIdx = INDEX_NONE;
	_rArchive << *(UObject**)&pOwner;
	_rArchive << TweakIdx;

	if( pOwner != NULL && TweakIdx >= 0 && TweakIdx < ( pOwner->*_pOwnerArray ).Num() )
	{
		_rpOutTweak = ( pOwner->*_pOwnerArray )( TweakIdx );
	}
}

#endif
