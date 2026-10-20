/*=============================================================================
	FxList.h - FaceFX 1.7.3.1 doubly linked list (fxlist.h, fxlistimpl.h).

	DISHONORED(port): agent FX1 (wave 26). 8 bytes {_head, _size}. The head is a sentinel node allocated on the
	first Begin()/End() (its element default-constructed), never in the constructor - FxList<...>::End 0xb05ff0,
	Begin 0x236c60. Insert copy-constructs the node from a stack temporary as retail (0xb06c20); RemoveIterator
	ignores the sentinel (0xb06cf0); Clear frees every node and the sentinel (0xb06810).
=============================================================================*/
#ifndef FxList_H__
#define FxList_H__

#include "FxMemory.h"

namespace OC3Ent
{
namespace Face
{

template<typename T> class FxList
{
public:
	class FxListNode
	{
	public:
		FxListNode()
			: element()
			, next(NULL)
			, prev(NULL)
		{
		}

		FxListNode(const T& iElement, FxListNode* iNext, FxListNode* iPrev)
			: element(iElement)
			, next(iNext)
			, prev(iPrev)
		{
		}

		T element;
		FxListNode* next;
		FxListNode* prev;
	};

	class Iterator
	{
	public:
		Iterator(FxListNode* pNode = NULL)
			: _pNode(pNode)
		{
		}

		Iterator& operator++()
		{
			_pNode = _pNode->next;
			return *this;
		}

		Iterator& operator--()
		{
			_pNode = _pNode->prev;
			return *this;
		}

		Iterator& Advance(FxSize num)
		{
			while( num-- )
			{
				_pNode = _pNode->next;
			}
			return *this;
		}

		T& operator*() const
		{
			return _pNode->element;
		}

		T* operator->() const
		{
			return &_pNode->element;
		}

		FxBool operator==(const Iterator& other) const
		{
			return _pNode == other._pNode;
		}

		FxBool operator!=(const Iterator& other) const
		{
			return _pNode != other._pNode;
		}

		FxListNode* GetNode() const
		{
			return _pNode;
		}

		FxListNode* _pNode;
	};

	class ConstIterator
	{
	public:
		ConstIterator(const FxListNode* pNode = NULL)
			: _pNode(pNode)
		{
		}

		ConstIterator(const Iterator& other)
			: _pNode(other._pNode)
		{
		}

		ConstIterator& operator++()
		{
			_pNode = _pNode->next;
			return *this;
		}

		ConstIterator& operator--()
		{
			_pNode = _pNode->prev;
			return *this;
		}

		ConstIterator& Advance(FxSize num)
		{
			while( num-- )
			{
				_pNode = _pNode->next;
			}
			return *this;
		}

		const T& operator*() const
		{
			return _pNode->element;
		}

		const T* operator->() const
		{
			return &_pNode->element;
		}

		FxBool operator==(const ConstIterator& other) const
		{
			return _pNode == other._pNode;
		}

		FxBool operator!=(const ConstIterator& other) const
		{
			return _pNode != other._pNode;
		}

		const FxListNode* _pNode;
	};

	FxList()
		: _head(NULL)
		, _size(0)
	{
	}

	FxList(const FxList& other)
		: _head(NULL)
		, _size(0)
	{
		for( ConstIterator curr = other.Begin(); curr != other.End(); ++curr )
		{
			PushBack(*curr);
		}
	}

	FxList& operator=(const FxList& other)
	{
		if( this != &other )
		{
			Clear();
			for( ConstIterator curr = other.Begin(); curr != other.End(); ++curr )
			{
				PushBack(*curr);
			}
		}
		return *this;
	}

	~FxList()
	{
		Clear();
		if( _head )
		{
			FxDelete(_head);
		}
	}

	void Clear()
	{
		if( _head )
		{
			FxListNode* pNode = _head->next;
			while( pNode != _head )
			{
				FxListNode* temp = pNode;
				pNode = pNode->next;
				FxDelete(temp);
			}
			FxDelete(_head);
			_size = 0;
		}
	}

	FxBool IsEmpty() const
	{
		return _size == 0;
	}

	FxSize Length() const
	{
		return _size;
	}

	Iterator Begin()
	{
		_EnsureHead();
		return Iterator(_head->next);
	}

	ConstIterator Begin() const
	{
		const_cast<FxList*>(this)->_EnsureHead();
		return ConstIterator(_head->next);
	}

	Iterator End()
	{
		_EnsureHead();
		return Iterator(_head);
	}

	ConstIterator End() const
	{
		const_cast<FxList*>(this)->_EnsureHead();
		return ConstIterator(_head);
	}

	T& Front()
	{
		return _head->next->element;
	}

	T& Back()
	{
		return _head->prev->element;
	}

	Iterator Insert(const T& element, Iterator iter)
	{
		FxListNode* insertNode = static_cast<FxListNode*>(FxAllocate(sizeof(FxListNode)));
		FxConstruct(insertNode, FxListNode(element, iter._pNode, iter._pNode->prev));
		insertNode->prev->next = insertNode;
		insertNode->next->prev = insertNode;
		++_size;
		return Iterator(insertNode);
	}

	void PushBack(const T& element)
	{
		Insert(element, End());
	}

	void PushFront(const T& element)
	{
		Insert(element, Begin());
	}

	void RemoveIterator(Iterator iter)
	{
		FxListNode* toRemove = iter._pNode;
		if( toRemove != _head )
		{
			toRemove->prev->next = toRemove->next;
			toRemove->next->prev = toRemove->prev;
			FxDelete(toRemove);
			--_size;
		}
	}

	void Remove(Iterator iter)
	{
		RemoveIterator(iter);
	}

	void RemoveIfEqual(const T& element)
	{
		if( _head )
		{
			FxListNode* curr = _head->next;
			while( curr != _head )
			{
				if( curr->element == element )
				{
					FxListNode* temp = curr;
					curr = curr->next;
					temp->prev->next = temp->next;
					temp->next->prev = temp->prev;
					FxDelete(temp);
					--_size;
				}
				else
				{
					curr = curr->next;
				}
			}
		}
	}

	Iterator Find(const T& toFind)
	{
		Iterator finder = Begin();
		Iterator end = End();
		while( finder != end && *finder != toFind )
		{
			++finder;
		}
		return finder;
	}

	FxListNode* _head;
	FxSize _size;

private:
	void _EnsureHead()
	{
		if( !_head )
		{
			_head = static_cast<FxListNode*>(FxAllocate(sizeof(FxListNode)));
			FxDefaultConstruct(_head);
			_head->next = _head;
			_head->prev = _head;
		}
	}
};

} // namespace Face
} // namespace OC3Ent

#endif
