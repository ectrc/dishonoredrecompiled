/*=============================================================================
	FxString.h - FaceFX 1.7.3.1 strings (fxchartraits.h, fxstring.h, fxstringimpl.h).

	DISHONORED(port): agent FX1 (wave 26). FxStringBase<T> is one FxArrayBase<T> (12 bytes in the PDB);
	_usedCount is the character count and the buffer keeps a terminator one past it once allocated.
	Instantiations retail emitted: FxStringBase<char>(FxSize) 0xb05c40, (const char*, FxSize) 0x569c40,
	copy 0x237ee0, operator=(const&) 0xb05c80, operator=(const char*) 0xb179c0, GetData 0x3449f0,
	operator<<(int) 0xb14f20, operator<<(const char*) 0xb15090, _DoAppendString 0xb150c0,
	Serialize 0xb062a0 (defined in FxArchive.h), operator==(.., const char*) 0xb061c0, operator!= 0xb06250,
	operator==(.., const FxStringBase&) 0xb19db0; FxCharTraits<char> Length 0x567f00, Copy 0x229580,
	Compare 0xb06210, Convert 0xb06440; FxTempCharBuffer<char> 0xb1a130 / 0xb1a1a0 / Release 0xb1a1e0.
	GetData allocates a 16-character buffer for a never-allocated string, as retail, so it is logically const
	only (the member is mutated through const_cast).
=============================================================================*/
#ifndef FxString_H__
#define FxString_H__

#include "FxArray.h"

namespace OC3Ent
{
namespace Face
{

template<typename T> struct FxCharTraits;

template<> struct FxCharTraits<FxChar>
{
	static FxSize Length(const FxChar* str)
	{
		return static_cast<FxSize>(strlen(str));
	}

	static FxChar* Copy(FxChar* dest, const FxChar* src, FxSize num)
	{
		return static_cast<FxChar*>(memcpy(dest, src, num));
	}

	static FxInt32 Compare(const FxChar* first, const FxChar* second, FxSize num)
	{
		return FxStrncmp(first, second, num);
	}

	static FxChar* Convert(FxChar* dest, const FxWChar* src, FxSize num)
	{
		for( FxSize i = 0; i < num; ++i )
		{
			dest[i] = static_cast<FxChar>(wctob(src[i]));
		}
		return dest;
	}
};

template<> struct FxCharTraits<FxWChar>
{
	static FxSize Length(const FxWChar* str)
	{
		return static_cast<FxSize>(wcslen(str));
	}

	static FxWChar* Copy(FxWChar* dest, const FxWChar* src, FxSize num)
	{
		return static_cast<FxWChar*>(memcpy(dest, src, num * sizeof(FxWChar)));
	}

	static FxInt32 Compare(const FxWChar* first, const FxWChar* second, FxSize num)
	{
		return wcsncmp(first, second, num);
	}

	static FxWChar* Convert(FxWChar* dest, const FxChar* src, FxSize num)
	{
		for( FxSize i = 0; i < num; ++i )
		{
			dest[i] = static_cast<FxWChar>(btowc(static_cast<unsigned char>(src[i])));
		}
		return dest;
	}
};

template<typename T> class FxStringBase
{
public:
	FxStringBase(FxSize numReserved = 0)
		: _container(numReserved ? numReserved + 1 : 0)
	{
	}

	FxStringBase(const T* str, FxSize numChars = FxInvalidIndex)
		: _container(0)
	{
		if( str )
		{
			FxSize length = numChars == FxInvalidIndex ? FxCharTraits<T>::Length(str) : numChars;
			if( length )
			{
				FxArrayBase<T> temp(length + 1);
				FxCharTraits<T>::Copy(temp._v, str, length);
				_container.Swap(temp);
				_container._usedCount = length;
				_container._v[length] = 0;
			}
		}
	}

	FxStringBase(const FxStringBase& other)
		: _container(other._container._usedCount ? other._container._usedCount + 1 : 0)
	{
		FxCharTraits<T>::Copy(_container._v, other._container._v, other._container._usedCount);
		_container._usedCount = other._container._usedCount;
		if( _container._usedCount )
		{
			_container._v[_container._usedCount] = 0;
		}
	}

	FxStringBase& operator=(const FxStringBase& other)
	{
		FxStringBase temp(other);
		_container.Swap(temp._container);
		return *this;
	}

	FxStringBase& operator=(const T* str)
	{
		FxStringBase temp(str);
		_container.Swap(temp._container);
		return *this;
	}

	FxSize Length() const
	{
		return _container._usedCount;
	}

	FxBool IsEmpty() const
	{
		return _container._usedCount == 0;
	}

	void Clear()
	{
		FxStringBase temp;
		_container.Swap(temp._container);
	}

	const T* GetData() const
	{
		FxArrayBase<T>& container = const_cast<FxArrayBase<T>&>(_container);
		if( !container._v )
		{
			FxArrayBase<T> temp(16);
			container.Swap(temp);
		}
		container._v[container._usedCount] = 0;
		return container._v;
	}

	const T* GetCstr() const
	{
		return GetData();
	}

	T& operator[](FxSize index)
	{
		return _container._v[index];
	}

	const T& operator[](FxSize index) const
	{
		return _container._v[index];
	}

	FxStringBase& operator<<(const T* str)
	{
		if( str )
		{
			_DoAppendString(str, FxCharTraits<T>::Length(str));
		}
		return *this;
	}

	FxStringBase& operator<<(const FxStringBase& str)
	{
		_DoAppendString(str.GetData(), str.Length());
		return *this;
	}

	FxStringBase& operator<<(FxInt32 num)
	{
		FxChar buffer[68];
		memset(buffer, 0, 64);
		FxItoa(num, buffer, 64);
		FxSize length = FxCharTraits<FxChar>::Length(buffer);
		T converted[68];
		for( FxSize i = 0; i <= length; ++i )
		{
			converted[i] = static_cast<T>(buffer[i]);
		}
		_DoAppendString(converted, length);
		return *this;
	}

	FxStringBase& operator+=(const T* str)
	{
		return *this << str;
	}

	FxStringBase& operator+=(const FxStringBase& str)
	{
		return *this << str;
	}

	FxArchive& Serialize(FxArchive& arc);

	FxArrayBase<T> _container;

private:
	void _DoAppendString(const T* str, FxSize length)
	{
		if( _container._allocatedCount >= _container._usedCount + length + 1 )
		{
			FxCharTraits<T>::Copy(_container._v + _container._usedCount, str, length);
			_container._usedCount += length;
		}
		else
		{
			FxArrayBase<T> temp(_container._usedCount + length + 1);
			FxCharTraits<T>::Copy(temp._v, _container._v, _container._usedCount);
			temp._usedCount += _container._usedCount;
			FxCharTraits<T>::Copy(temp._v + temp._usedCount, str, length);
			temp._usedCount += length;
			_container.Swap(temp);
		}
		_container._v[_container._usedCount] = 0;
	}
};

typedef FxStringBase<FxChar> FxString;
typedef FxStringBase<FxWChar> FxWString;

template<typename T> inline FxBool operator==(const FxStringBase<T>& lhs, const T* rhs)
{
	if( !rhs )
	{
		return FxFalse;
	}
	if( lhs.Length() != FxCharTraits<T>::Length(rhs) )
	{
		return FxFalse;
	}
	return FxCharTraits<T>::Compare(lhs.GetData(), rhs, lhs.Length()) == 0;
}

template<typename T> inline FxBool operator!=(const FxStringBase<T>& lhs, const T* rhs)
{
	return !(lhs == rhs);
}

template<typename T> inline FxBool operator==(const FxStringBase<T>& lhs, const FxStringBase<T>& rhs)
{
	if( lhs.Length() != rhs.Length() )
	{
		return FxFalse;
	}
	return FxCharTraits<T>::Compare(lhs.GetData(), rhs.GetData(), lhs.Length()) == 0;
}

template<typename T> inline FxBool operator!=(const FxStringBase<T>& lhs, const FxStringBase<T>& rhs)
{
	return !(lhs == rhs);
}

template<typename T> inline FxBool operator<(const FxStringBase<T>& lhs, const FxStringBase<T>& rhs)
{
	FxSize length = FxMin(lhs.Length(), rhs.Length()) + 1;
	return FxCharTraits<T>::Compare(lhs.GetData(), rhs.GetData(), length) < 0;
}

template<typename T> class FxTempCharBuffer
{
public:
	FxTempCharBuffer(const T* pNativeStr)
	{
		FxSize length = FxCharTraits<T>::Length(pNativeStr);
		_bufferSize = length + 1;
		_buffer = static_cast<T*>(FxAllocate(_bufferSize * sizeof(T)));
		FxCharTraits<T>::Copy(_buffer, pNativeStr, length);
		_buffer[length] = 0;
	}

	~FxTempCharBuffer()
	{
		if( _buffer )
		{
			FxFree(_buffer, _bufferSize * sizeof(T));
			_buffer = NULL;
			_bufferSize = 0;
		}
	}

	T* Release()
	{
		T* retval = _buffer;
		_buffer = NULL;
		return retval;
	}

	FxSize GetBufferSize() const
	{
		return _bufferSize;
	}

	T* _buffer;
	FxSize _bufferSize;
};

} // namespace Face
} // namespace OC3Ent

#endif
