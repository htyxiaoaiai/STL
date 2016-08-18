#pragma once
#include <iostream>
#include "Iterator.h"

using namespace std;

template <class T >
class Vector
{
public:
	typedef T* Iterator;
	typedef const T* ConstIterator;
	typedef T* Pointer;
	typedef T& Reference;

	Vector()
		:_start(NULL)
		, _finish(NULL)
		, _endOfStorage(NULL)
	{}

	Iterator Begin()
	{
		return _start;
	}

	Iterator End()
	{
		return _finish;
	}

	ConstIterator Begin()const
	{
		return _start;
	}

	ConstIterator End()const
	{
		return _finish;
	}

	Iterator Insert(Iterator pos, const T& x)
	{
		_CheckExpend();

	}

	Iterator Erase(Iterator pos)
	{
		Iterator begin = pos + 1;
		while (begin != _finish)
		{
			*(begin - 1) = *begin;
			begin++;
		}
		--_finish;
		return pos;
	}

	size_t Size()
	{
		return _finish - _start;
	}
	size_t Capacity()
	{
		return _endOfStorage - _start;
	}

	void PushBack(const T&x)
	{
		_CheckExpend();
		
		*_finish = x;
		_finish++;
	}
	void PopBack()
	{
		assert(_finish > _start);
		_finish--;
	}

	Reference operator[](size_t index)
	{
		assert(index < Size());
		return _start[index];
	}

protected:
	void _CheckExpend()
	{
		if (_finish == _endOfStorage)
		{
			size_t size = Size();
			size_t capacity = 2 * Size() + 3;
			T* tmp = new T[capacity];
			for (size_t i = 0; i < size; i++)
			{
				tmp[i] = _start[i];
			}
			_start = tmp;
			_finish = _start + size;
			_endOfStorage = _start + capacity;
		}
	}


private:
	Iterator _start;
	Iterator _finish;
	Iterator _endOfStorage;
};

void TestVector()
{
	Vector<int> v;
	v.PushBack(1);
	v.PushBack(2);
	v.PushBack(3);
	v.PushBack(4);
	Vector<int>::Iterator start = v.Begin();
	Vector<int>::Iterator end= v.End();

	cout << "¾àÀëÎª£º" << Distance(start, end)<<endl;

	/*Vector<int>::Iterator it = v.Begin();
	while (it != v.End())
	{
		cout << *it << " ";
		it++;
	}*/

	for (size_t i = 0; i < v.Size(); i++)
	{
		v[i]++;
		cout << v[i]<<" ";	
	}
}
