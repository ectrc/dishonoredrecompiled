/*=============================================================================
	FxArray.h - FaceFX 1.7.3.1 dynamic array (fxarraybase.h, fxarray.h, fxarrayimpl.h).

	DISHONORED(port): agent FX1 (wave 26). FxArrayBase<T> is the 12-byte storage {_v, _allocatedCount, _usedCount}
	the PDB shows for every instantiation; FxArray<T> wraps it. Behaviour taken from the instantiations retail
	emitted (FxArchive.obj / FxName.obj / FxAnim.obj ...):
	  - PushBack grows to twice the allocated count (1 from empty) and copy-constructs into the new block
	    (FxArray<FxArchiveNameEntry>::PushBack 0xb19a90);
	  - Insert grows by exactly one element (FxArray<FxAnimCurve>::Insert);
	  - Remove copy-assigns the tail down, destroys the last slot and zeroes it (0xb1ba60); arithmetic
	    element types take the memmove path (FxArray<FxFaceGraphNode*>::Remove 0xb19e40);
	  - Clear swaps with an empty array, so it frees the block (0xb1b850);
	  - Reserve only grows (FxArray<FxRegister>::Reserve).
	FxArray<T>::Serialize and operator<< are defined at the end of FxArchive.h (they need the archive).
=============================================================================*/
#ifndef FxArray_H__
#define FxArray_H__

#include "FxMemory.h"
#include "FxUtil.h"

namespace OC3Ent
{
namespace Face
{

class FxArchive;

template<typename T> class FxArrayBase
{
public:
	FxArrayBase(FxSize size = 0)
		: _v(size ? static_cast<T*>(FxAllocate(sizeof(T) * size)) : NULL)
		, _allocatedCount(size)
		, _usedCount(0)
	{
	}

	~FxArrayBase()
	{
		FxDestroy(_v, _v + _usedCount);
		FxFree(_v, sizeof(T) * _allocatedCount);
		_v = NULL;
		_usedCount = 0;
		_allocatedCount = 0;
	}

	void Swap(FxArrayBase& other)
	{
		FxSwap(_v, other._v);
		FxSwap(_allocatedCount, other._allocatedCount);
		FxSwap(_usedCount, other._usedCount);
	}

	void Prefetch() const
	{
	}

	T* _v;
	FxSize _allocatedCount;
	FxSize _usedCount;

private:
	FxArrayBase(const FxArrayBase&);
	FxArrayBase& operator=(const FxArrayBase&);
};

template<typename T> class FxArray
{
public:
	typedef T* Iterator;
	typedef const T* ConstIterator;

	FxArray(FxSize numReserved = 0)
		: _container(numReserved)
	{
	}

	FxArray(const FxArray& other)
		: _container(other._container._usedCount)
	{
		while( _container._usedCount < other._container._usedCount )
		{
			FxConstruct(_container._v + _container._usedCount, other._container._v[_container._usedCount]);
			++_container._usedCount;
		}
	}

	FxArray& operator=(const FxArray& other)
	{
		FxArray temp(other);
		_container.Swap(temp._container);
		return *this;
	}

	~FxArray()
	{
	}

	void Clear()
	{
		FxArray temp(0);
		Swap(temp);
	}

	FxSize Length() const
	{
		return _container._usedCount;
	}

	FxSize Allocated() const
	{
		return _container._allocatedCount;
	}

	FxBool IsEmpty() const
	{
		return _container._usedCount == 0;
	}

	void Reserve(FxSize n)
	{
		if( n > _container._allocatedCount )
		{
			FxArrayBase<T> temp(n);
			while( temp._usedCount < _container._usedCount )
			{
				FxConstruct(temp._v + temp._usedCount, _container._v[temp._usedCount]);
				++temp._usedCount;
			}
			_container.Swap(temp);
		}
	}

	T& operator[](FxSize index)
	{
		return _container._v[index];
	}

	const T& operator[](FxSize index) const
	{
		return _container._v[index];
	}

	Iterator Begin()
	{
		return _container._v;
	}

	ConstIterator Begin() const
	{
		return _container._v;
	}

	Iterator End()
	{
		return _container._v + _container._usedCount;
	}

	ConstIterator End() const
	{
		return _container._v + _container._usedCount;
	}

	T& Back()
	{
		return _container._v[_container._usedCount - 1];
	}

	const T& Back() const
	{
		return _container._v[_container._usedCount - 1];
	}

	void PushBack(const T& element)
	{
		if( _container._usedCount < _container._allocatedCount )
		{
			FxConstruct(_container._v + _container._usedCount, element);
			++_container._usedCount;
		}
		else
		{
			FxArrayBase<T> temp(_container._allocatedCount ? 2 * _container._allocatedCount : 1);
			while( temp._usedCount < _container._usedCount )
			{
				FxConstruct(temp._v + temp._usedCount, _container._v[temp._usedCount]);
				++temp._usedCount;
			}
			FxConstruct(temp._v + temp._usedCount++, element);
			_container.Swap(temp);
		}
	}

	void PopBack()
	{
		Remove(_container._usedCount - 1);
	}

	void Insert(const T& element, FxSize index)
	{
		if( _container._usedCount < _container._allocatedCount )
		{
			if( index == _container._usedCount )
			{
				FxConstruct(_container._v + index, element);
				++_container._usedCount;
			}
			else
			{
				T* curr = _container._v + _container._usedCount;
				FxConstruct(curr, *(curr - 1));
				for( --curr; curr != _container._v + index; --curr )
				{
					*curr = *(curr - 1);
				}
				_container._v[index] = element;
				++_container._usedCount;
			}
		}
		else
		{
			FxArrayBase<T> temp(_container._allocatedCount + 1);
			while( temp._usedCount < index )
			{
				FxConstruct(temp._v + temp._usedCount, _container._v[temp._usedCount]);
				++temp._usedCount;
			}
			FxConstruct(temp._v + temp._usedCount++, element);
			while( temp._usedCount <= _container._usedCount )
			{
				FxConstruct(temp._v + temp._usedCount, _container._v[temp._usedCount - 1]);
				++temp._usedCount;
			}
			_container.Swap(temp);
		}
	}

	void Remove(FxSize index)
	{
		if( FxTypeTraits::is_arithmetic<T>::value )
		{
			FxMemmove(_container._v + index, _container._v + index + 1, sizeof(T) * (_container._usedCount - index) - sizeof(T));
			--_container._usedCount;
			FxDestroy(_container._v + _container._usedCount);
			FxMemset(_container._v + _container._usedCount, 0, sizeof(T));
		}
		else
		{
			for( T* curr = _container._v + index; curr != _container._v + _container._usedCount - 1; ++curr )
			{
				*curr = *(curr + 1);
			}
			FxDestroy(_container._v + --_container._usedCount);
			FxMemset(_container._v + _container._usedCount, 0, sizeof(T));
		}
	}

	void RemoveIterator(Iterator iter)
	{
		Remove(static_cast<FxSize>(iter - _container._v));
	}

	FxSize Find(const T& element, FxSize startIndex = 0) const
	{
		while( startIndex < _container._usedCount )
		{
			if( element == _container._v[startIndex] )
			{
				return startIndex;
			}
			++startIndex;
		}
		return FxInvalidIndex;
	}

	void Swap(FxArray& other)
	{
		_container.Swap(other._container);
	}

	void Prefetch() const
	{
		_container.Prefetch();
	}

	FxArchive& Serialize(FxArchive& arc);

	FxArrayBase<T> _container;
};

} // namespace Face
} // namespace OC3Ent

#endif
