/*=============================================================================
	FxStringHash.h - FaceFX 1.7.3.1 string-keyed hash (fxstringhash.h, fxstringhashimpl.h).

	DISHONORED(port): agent FX1 (wave 26). 2^HashBits bins of Container<FxHashBinEntry> plus four counters.
	The bin of a key is djb2 over its signed chars, masked to HashBits (FxStringHash<...,12,FxList>::_hash
	0xb06600, <...,7,FxArray>::_hash 0xb1b1b0: binIndex = key[i] + 33 * binIndex from 5381). Retail
	instantiations: <FxRefString**,12,FxList> (the FxName table, FxName.obj), <unsigned short,7,FxArray> (archive
	class versions, FxArchive.obj), <FxFaceGraphNode*,11,FxArray> and <unsigned int,11,FxArray> (face graph).
	Insert refuses a key already present and counts a collision when the bin is not empty; a found key counts
	a probe; Clear empties every bin and zeroes the counters. Serialize writes the used bins as
	{binIndex, length, {key, object}*} and is defined at the end of FxArchive.h.
	Not in retail (named): GetBinIndex / GetBin, read-only accessors CoreSmoke uses to compare bins with the python
	reader; the GetNum* counter accessors stand for the field reads FxName::GetNameStats (0xb056a0) inlines.
=============================================================================*/
#ifndef FxStringHash_H__
#define FxStringHash_H__

#include "FxString.h"
#include "FxList.h"

namespace OC3Ent
{
namespace Face
{

template<typename T, FxSize HashBits, template<typename> class Container> class FxStringHash
{
public:
	class FxHashBinEntry
	{
	public:
		FxHashBinEntry()
			: key(static_cast<FxSize>(0))
		{
		}

		FxHashBinEntry(const FxString& iKey, const T& iObject)
			: key(iKey)
			, object(iObject)
		{
		}

		FxHashBinEntry(const FxHashBinEntry& other)
			: key(other.key)
			, object(other.object)
		{
		}

		FxHashBinEntry& operator=(const FxHashBinEntry& other)
		{
			if( this != &other )
			{
				key = other.key;
				object = other.object;
			}
			return *this;
		}

		FxString key;
		T object;
	};

	typedef Container<FxHashBinEntry> FxHashBin;

	enum { NumBins = 1 << HashBits };

	FxStringHash()
		: numInsertions(0)
		, numRemovals(0)
		, numCollisions(0)
		, numProbes(0)
	{
	}

	~FxStringHash()
	{
		Clear();
	}

	void Clear()
	{
		FxSize numBins = GetNumBins();
		for( FxSize binIndex = 0; binIndex < numBins; ++binIndex )
		{
			hashBins[binIndex].Clear();
		}
		numInsertions = 0;
		numRemovals = 0;
		numCollisions = 0;
		numProbes = 0;
	}

	FxBool Insert(const FxChar* key, const T& object)
	{
		FxSize binIndex = _hash(key);
		FxHashBin& theBin = hashBins[binIndex];
		if( _findKeyInBin(key, theBin) )
		{
			return FxFalse;
		}
		if( !theBin.IsEmpty() )
		{
			++numCollisions;
		}
		++numInsertions;
		FxString iKey(key);
		theBin.PushBack(FxHashBinEntry(iKey, object));
		return FxTrue;
	}

	FxBool Remove(const FxChar* key)
	{
		FxSize binIndex = _hash(key);
		return _removeFromBin(key, hashBins[binIndex]);
	}

	FxBool Remove(const FxChar* key, T& object)
	{
		FxSize binIndex = _hash(key);
		return _removeFromBin(key, hashBins[binIndex], object);
	}

	FxBool Find(const FxChar* key) const
	{
		FxSize binIndex = _hash(key);
		return _findKeyInBin(key, hashBins[binIndex]);
	}

	FxBool Find(const FxChar* key, T& object) const
	{
		FxSize binIndex = _hash(key);
		return _findKeyInBin(key, hashBins[binIndex], object);
	}

	FxSize GetNumBins() const
	{
		return NumBins;
	}

	FxSize GetNumBinsInUse() const
	{
		FxSize numBinsInUse = 0;
		FxSize numBins = GetNumBins();
		for( FxSize binIndex = 0; binIndex < numBins; ++binIndex )
		{
			if( !hashBins[binIndex].IsEmpty() )
			{
				++numBinsInUse;
			}
		}
		return numBinsInUse;
	}

	FxSize GetNumInsertions() const
	{
		return numInsertions;
	}

	FxSize GetNumRemovals() const
	{
		return numRemovals;
	}

	FxSize GetNumCollisions() const
	{
		return numCollisions;
	}

	FxSize GetNumProbes() const
	{
		return numProbes;
	}

	FxSize GetMemUsedEstimate() const
	{
		FxSize numBins = GetNumBins();
		FxSize memUsedEstimate = sizeof(FxHashBin) * numBins;
		for( FxSize binIndex = 0; binIndex < numBins; ++binIndex )
		{
			const FxHashBin& theBin = hashBins[binIndex];
			for( typename FxHashBin::ConstIterator curr = theBin.Begin(); curr != theBin.End(); ++curr )
			{
				memUsedEstimate += (*curr).key.Length() + 1 + sizeof(FxHashBinEntry);
			}
		}
		return memUsedEstimate;
	}

	FxSize GetBinIndex(const FxChar* key) const
	{
		return _hash(key);
	}

	const FxHashBin& GetBin(FxSize binIndex) const
	{
		return hashBins[binIndex];
	}

	FxArchive& Serialize(FxArchive& arc);

	FxSize numInsertions;
	FxSize numRemovals;
	FxSize numCollisions;
	FxSize numProbes;
	FxHashBin hashBins[NumBins];

private:
	FxSize _hash(const FxChar* key) const
	{
		FxSize binIndex = 5381;
		FxSize keyLength = FxCharTraits<FxChar>::Length(key);
		for( FxSize i = 0; i < keyLength; ++i )
		{
			binIndex = key[i] + 33 * binIndex;
		}
		return binIndex & (NumBins - 1);
	}

	FxBool _removeFromBin(const FxChar* key, FxHashBin& theBin)
	{
		for( typename FxHashBin::Iterator curr = theBin.Begin(); curr != theBin.End(); ++curr )
		{
			if( (*curr).key == key )
			{
				++numRemovals;
				theBin.RemoveIterator(curr);
				return FxTrue;
			}
		}
		return FxFalse;
	}

	FxBool _removeFromBin(const FxChar* key, FxHashBin& theBin, T& object)
	{
		for( typename FxHashBin::Iterator curr = theBin.Begin(); curr != theBin.End(); ++curr )
		{
			if( (*curr).key == key )
			{
				++numRemovals;
				object = (*curr).object;
				theBin.RemoveIterator(curr);
				return FxTrue;
			}
		}
		return FxFalse;
	}

	FxBool _findKeyInBin(const FxChar* key, const FxHashBin& theBin) const
	{
		for( typename FxHashBin::ConstIterator curr = theBin.Begin(); curr != theBin.End(); ++curr )
		{
			if( (*curr).key == key )
			{
				++const_cast<FxStringHash*>(this)->numProbes;
				return FxTrue;
			}
		}
		return FxFalse;
	}

	FxBool _findKeyInBin(const FxChar* key, const FxHashBin& theBin, T& object) const
	{
		for( typename FxHashBin::ConstIterator curr = theBin.Begin(); curr != theBin.End(); ++curr )
		{
			if( (*curr).key == key )
			{
				++const_cast<FxStringHash*>(this)->numProbes;
				object = (*curr).object;
				return FxTrue;
			}
		}
		return FxFalse;
	}
};

} // namespace Face
} // namespace OC3Ent

#endif
