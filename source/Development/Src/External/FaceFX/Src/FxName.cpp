/*=============================================================================
	FxName.cpp - FaceFX 1.7.3.1 shared names (fxsdk\src\fxname.cpp, compiland FxName.obj).

	DISHONORED(port): agent FX1 (wave 26). The name table is an FxList<FxRefString*> (allocated by Startup);
	an FxName points at a list slot, so a rename is seen by every name sharing it. Names are proofed before
	lookup (ProofName 0xb05a20, 512 characters at most: a longer name is left unterminated by the loop and comes
	out empty, as retail).
	Startup also names FxAnimGroup::Default ("Default") and FxActor::NewActor ("NewActor") and Shutdown clears
	them; those two statics belong to FX3's classes, so those lines compile once FxAnim.h / FxActor.h are in
	the tree (__has_include) - deviation until then: the two names are not pre-registered.
=============================================================================*/
#include "FxName.h"
#include "FxRefObject.h"
#if __has_include("FxAnim.h")
#include "FxAnim.h"
#define FX1_HAS_FXANIMGROUP 1
#else
#define FX1_HAS_FXANIMGROUP 0
#endif
#if __has_include("FxActor.h")
#include "FxActor.h"
#define FX1_HAS_FXACTOR 1
#else
#define FX1_HAS_FXACTOR 0
#endif

namespace OC3Ent
{
namespace Face
{

static const FxSize FX_PROOF_NAME_SCRATCH_LENGTH = 512;

/** 2012 0xb05a20. ProofNameScratchLength is a value parameter retail never reads. */
static void ProofName(const FxChar* name, FxChar* ProofNameScratch, FxSize ProofNameScratchLength)
{
	if( name )
	{
		for( FxSize i = 0; i < FX_PROOF_NAME_SCRATCH_LENGTH; ++i )
		{
			switch( name[i] )
			{
			case '\0':
				ProofNameScratchLength = i;
				ProofNameScratch[ProofNameScratchLength] = 0;
				return;
			case '|':
				ProofNameScratch[i] = '_';
				break;
			case '"':
				ProofNameScratch[i] = '\'';
				break;
			case ' ':
				ProofNameScratch[i] = '_';
				break;
			default:
				ProofNameScratch[i] = name[i];
				break;
			}
		}
	}
	else
	{
		ProofNameScratchLength = 0;
	}
	ProofNameScratch[ProofNameScratchLength] = 0;
}

/** fxname.cpp:86-87 (dynamic initialisers 0xbab3a0 / 0xbab3c0). */
FxName FxName::NullName;
FxStringHash<FxRefString**, 12, FxList> FxName::_hashTable;

static FxList<FxRefString*>* NameTable = NULL;

/** 2012 0xb04fe0. */
FxName::FxName()
	: _ppRefString(NULL)
{
	_ppRefString = NullName._ppRefString;
	if( _ppRefString )
	{
		(*_ppRefString)->AddReference();
	}
}

/** 2012 0xb05020. */
FxName::FxName(const FxChar* name)
	: _ppRefString(NULL)
{
	_addNameToTable(name);
}

/** 2012 0xb05050. */
FxName::FxName(const FxName& other)
	: _ppRefString(other._ppRefString)
{
	if( _ppRefString )
	{
		(*_ppRefString)->AddReference();
	}
}

/** 2012 0xb05080. */
FxName& FxName::operator=(const FxChar* other)
{
	_safeRemoveRef();
	_addNameToTable(other);
	return *this;
}

/** 2012 0xb050b0. */
FxName& FxName::operator=(const FxName& other)
{
	if( this == &other )
	{
		return *this;
	}
	_safeRemoveRef();
	_ppRefString = other._ppRefString;
	if( _ppRefString )
	{
		(*_ppRefString)->AddReference();
	}
	return *this;
}

/** 2012 0xb05100. */
FxName::~FxName()
{
	if( NameTable )
	{
		_safeRemoveRef();
	}
}

/** 2012 0xb05120. */
void FxName::Startup()
{
	if( !NameTable )
	{
		NameTable = static_cast<FxList<FxRefString*>*>(FxAllocate(sizeof(FxList<FxRefString*>)));
		FxDefaultConstruct(NameTable);
		NullName = "";
#if FX1_HAS_FXANIMGROUP
		FxAnimGroup::Default = "Default";
#endif
#if FX1_HAS_FXACTOR
		FxActor::NewActor = "NewActor";
#endif
	}
}

/** 2012 0xb05180: deletes every string once (a slot can repeat a pointer), then the table itself. */
void FxName::Shutdown()
{
	_hashTable.Clear();
	FxList<FxRefString*>::Iterator curr = NameTable->Begin();
	FxList<FxRefString*>::Iterator end = NameTable->End();
	while( curr != end )
	{
		if( *curr )
		{
			FxRefString* badPointer = *curr;
			FxDelete(*curr);
			FxList<FxRefString*>::Iterator pos(NULL);
			while( (pos = NameTable->Find(badPointer)) != end )
			{
				*pos = NULL;
			}
		}
		++curr;
	}
	FxDelete(NameTable);
	NullName._ppRefString = NULL;
#if FX1_HAS_FXANIMGROUP
	FxAnimGroup::Default._ppRefString = NULL;
#endif
#if FX1_HAS_FXACTOR
	FxActor::NewActor._ppRefString = NULL;
#endif
}

/** 2012 0xb05270. */
FxBool FxName::operator==(const FxChar* other) const
{
	if( !_ppRefString )
	{
		const_cast<FxName*>(this)->_ppRefString = NullName._ppRefString;
	}
	return (*_ppRefString)->GetString() == other;
}

/** 2012 0xb052d0. */
FxBool FxName::operator!=(const FxChar* other) const
{
	if( !_ppRefString )
	{
		const_cast<FxName*>(this)->_ppRefString = NullName._ppRefString;
	}
	return (*_ppRefString)->GetString() != other;
}

/** 2012 0xb05330. */
void FxName::SetName(const FxChar* name)
{
	_safeRemoveRef();
	_addNameToTable(name);
}

/** 2012 0xb05360. */
const FxString& FxName::GetAsString() const
{
	if( !_ppRefString )
	{
		const_cast<FxName*>(this)->_ppRefString = NullName._ppRefString;
	}
	return (*_ppRefString)->GetString();
}

/** 2012 0xb05390. */
const FxChar* FxName::GetAsCstr() const
{
	if( !_ppRefString )
	{
		const_cast<FxName*>(this)->_ppRefString = NullName._ppRefString;
	}
	return (*_ppRefString)->GetString().GetData();
}

/** 2012 0xb053d0: renames the shared string in place unless another name already has the new spelling. */
FxBool FxName::Rename(const FxChar* name)
{
	FxChar ProofNameScratch[FX_PROOF_NAME_SCRATCH_LENGTH + 4];
	FxSize ProofNameScratchLength = 0;
	ProofName(name, ProofNameScratch, ProofNameScratchLength);
	const FxString& theString = (*_ppRefString)->GetString();
	if( !(theString != ProofNameScratch) )
	{
		return FxFalse;
	}
	for( FxList<FxRefString*>::Iterator curr = NameTable->Begin(); curr != NameTable->End(); ++curr )
	{
		if( (*curr)->GetString() == ProofNameScratch )
		{
			return FxFalse;
		}
	}
	_hashTable.Remove(theString.GetData());
	_hashTable.Insert(ProofNameScratch, _ppRefString);
	(*_ppRefString)->SetString(ProofNameScratch);
	return FxTrue;
}

/** 2012 0xb05530. */
FxSize FxName::GetNumNames()
{
	return NameTable->Length();
}

/** 2012 0xb05540. */
FxSize FxName::Find(const FxChar* nameToFind)
{
	FxSize numNames = NameTable->Length();
	for( FxSize i = 0; i < numNames; ++i )
	{
		const FxString* pString = GetNameAsString(i);
		if( pString && *pString == nameToFind )
		{
			return i;
		}
	}
	return FxInvalidIndex;
}

/** 2012 0xb055b0. */
FxSize FxName::GetNameTableSize()
{
	FxSize result = 0;
	FxSize numNames = NameTable->Length();
	for( FxSize i = 0; i < numNames; ++i )
	{
		const FxString* pString = GetNameAsString(i);
		if( pString )
		{
			result += pString->Length();
		}
	}
	return result;
}

/** 2012 0xb05620. */
const FxString* FxName::GetNameAsString(FxSize index)
{
	FxList<FxRefString*>::Iterator pos = NameTable->Begin();
	pos.Advance(index);
	return &(*pos)->GetString();
}

/** 2012 0xb05660. */
FxSize FxName::GetNumReferences(FxSize index)
{
	FxList<FxRefString*>::Iterator pos = NameTable->Begin();
	pos.Advance(index);
	return (*pos)->GetReferenceCount();
}

/** 2012 0xb056a0: the hash counters in the order retail reads them (collisions, insertions, probes). */
void FxName::GetNameStats(FxSize& numCollisions, FxSize& numNamesAdded, FxSize& numProbes,
	FxSize& numHashBins, FxSize& numUsedHashBins, FxSize& memUsed)
{
	numCollisions = _hashTable.GetNumCollisions();
	numNamesAdded = _hashTable.GetNumInsertions();
	numProbes = _hashTable.GetNumProbes();
	numHashBins = _hashTable.GetNumBins();
	numUsedHashBins = _hashTable.GetNumBinsInUse();
	memUsed = _hashTable.GetMemUsedEstimate();
}

/** 2012 0xb05700. */
void FxName::_addNameToTable(const FxChar* name)
{
	FxChar ProofNameScratch[FX_PROOF_NAME_SCRATCH_LENGTH + 4];
	FxSize ProofNameScratchLength = 0;
	ProofName(name, ProofNameScratch, ProofNameScratchLength);
	FxRefString** ppRefString = NULL;
	FxBool findResult = _hashTable.Find(ProofNameScratch, ppRefString);
	if( findResult && ppRefString )
	{
		_ppRefString = ppRefString;
	}
	else
	{
		FxRefString* element = new FxRefString(ProofNameScratch);
		FxList<FxRefString*>::Iterator inserted = NameTable->Insert(element, NameTable->End());
		_ppRefString = &(*inserted);
		_hashTable.Insert(ProofNameScratch, _ppRefString);
	}
	(*_ppRefString)->AddReference();
}

/** 2012 0xb05ae0: the last reference also takes the string out of the hash and the table. */
void FxName::_safeRemoveRef()
{
	if( _ppRefString )
	{
		FxRefString* pRefString = *_ppRefString;
		if( pRefString && pRefString->GetReferenceCount() == 1 )
		{
			_hashTable.Remove(pRefString->GetString().GetData());
			NameTable->RemoveIfEqual(pRefString);
			_ppRefString = NULL;
		}
		pRefString->RemoveReference();
	}
}

/** 2012 0xb05890: since sdk 0x6A4 (class version 1) a name is an index into the archive name table. */
FxArchive& operator<<(FxArchive& arc, FxName& name)
{
	FxUInt16 version = arc.SerializeClassVersion("FxName", FxTrue, 1);
	if( arc.IsSaving() )
	{
		FxString tempString;
		if( name._ppRefString )
		{
			tempString = (*name._ppRefString)->GetString();
		}
		FxSize nameIndex = arc.AddName(&tempString);
		arc << nameIndex;
	}
	else
	{
		if( name._ppRefString )
		{
			name._safeRemoveRef();
		}
		if( version )
		{
			FxSize nameIndex = FxInvalidIndex;
			arc << nameIndex;
			name._ppRefString = arc.GetName(nameIndex);
			if( name._ppRefString )
			{
				(*name._ppRefString)->AddReference();
			}
		}
		else
		{
			FxString str;
			arc << str;
			name._addNameToTable(str.GetData());
		}
	}
	return arc;
}

} // namespace Face
} // namespace OC3Ent
