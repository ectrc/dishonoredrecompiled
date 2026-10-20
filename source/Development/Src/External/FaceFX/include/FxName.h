/*=============================================================================
	FxName.h - FaceFX 1.7.3.1 shared names (fxname.h).

	DISHONORED(port): agent FX1 (wave 26). An FxName is one pointer to a slot of the global name list (the
	FxList<FxRefString*> NameTable); the slot holds the reference-counted FxRefString. Lookup goes through the
	4096-bin FxStringHash<FxRefString**,12,FxList> _hashTable keyed by the proofed string (ProofName: '|' and ' '
	become '_', '"' becomes '\''). Two names are equal when they share the FxRefString (operator== 0x229590).
	A name whose slot pointer is NULL reads as NullName.
=============================================================================*/
#ifndef FxName_H__
#define FxName_H__

#include "FxStringHash.h"

namespace OC3Ent
{
namespace Face
{

class FxRefString;
class FxArchive;

class FxName
{
public:
	FxName();
	FxName(const FxChar* name);
	FxName(const FxName& other);
	FxName& operator=(const FxChar* other);
	FxName& operator=(const FxName& other);
	~FxName();

	static void Startup();
	static void Shutdown();

	FxBool operator==(const FxName& other) const
	{
		if( !_ppRefString )
		{
			const_cast<FxName*>(this)->_ppRefString = NullName._ppRefString;
		}
		return *_ppRefString == *other._ppRefString;
	}

	FxBool operator!=(const FxName& other) const
	{
		return !(*this == other);
	}

	FxBool operator==(const FxChar* other) const;
	FxBool operator!=(const FxChar* other) const;

	void SetName(const FxChar* name);
	const FxString& GetAsString() const;
	const FxChar* GetAsCstr() const;
	FxBool Rename(const FxChar* name);

	static FxSize GetNumNames();
	static FxSize Find(const FxChar* nameToFind);
	static FxSize GetNameTableSize();
	static const FxString* GetNameAsString(FxSize index);
	static FxSize GetNumReferences(FxSize index);
	static void GetNameStats(FxSize& numCollisions, FxSize& numNamesAdded, FxSize& numProbes,
		FxSize& numHashBins, FxSize& numUsedHashBins, FxSize& memUsed);

	static FxName NullName;

	friend FxArchive& operator<<(FxArchive& arc, FxName& name);
	friend class FxArchive;

private:
	void _addNameToTable(const FxChar* name);
	void _safeRemoveRef();

	FxRefString** _ppRefString;

	static FxStringHash<FxRefString**, 12, FxList> _hashTable;
};

FxArchive& operator<<(FxArchive& arc, FxName& name);

} // namespace Face
} // namespace OC3Ent

#endif
