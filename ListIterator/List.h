#pragma once
#include <iostream>
#include <assert.h>
#include "Iterator.h"


using namespace std;

template<class T>
struct __ListNode
{
	__ListNode()
		:_next(NULL)
		, _prev(NULL)
	{}

	__ListNode(const T& x)
		:_next(NULL)
		, _prev(NULL)
		, _data(x)
	{}

	T _data;
	__ListNode<T>* _prev;
	__ListNode<T>* _next;
};
//迭代器
template<class T, class Ref, class Ptr>
struct __ListIterator
{
	typedef __ListIterator<T, T&, T*>             Iterator;
	typedef __ListIterator<T, const T&, const T*> ConstIterator;
	typedef __ListIterator<T, Ref, Ptr>           Self;
	typedef __ListNode<T> Node;

	typedef Ref Reference;
	typedef Ptr Pointer;
	typedef BidirectionalIteratorTag IteratorCategory;
	typedef ptrdiff_t DifferenceType;
	typedef T ValueType; // 迭代器所指对象类型


	__ListIterator(Node* node = NULL)
		:_node(node)
	{}

	bool operator==(const Self& x)const
	{
		return _node == x._node;
	}

	bool operator!=(const Self& x)const
	{
		return _node != x._node;
	}

	Self& operator++()
	{
		_node = _node->_next;
		return *this;
	}
	Self operator++(int)
	{
		Self tmp(_node);
		_node = _node->_next;
		return tmp;
	}

	Self& operator--()
	{
		_node = _node->_prev;
		return *this;
	}
	Self operator--(int)
	{
		Self tmp(_node);
		_node = _node->_prev;
		return tmp;
	}

	Pointer* operator ->()
	{
		return &(_node->_data);
	}

	Reference operator *()
	{
		return _node->_data;
	}


	Node* _node;
};

template<class T>
class List
{
public:
	typedef __ListNode<T> Node;
	typedef __ListIterator<T, T&, T*>             Iterator;
	typedef __ListIterator<T, const T&, const T*> ConstIterator;
public:
	List()
		:_head(new Node)
	{
		_head->_next = _head;
		_head->_prev = _head;
	}

	Iterator Begin()
	{
		return Iterator(_head->_next);
	}
	Iterator End()
	{
		return Iterator(_head);
	}

	ConstIterator Begin()const
	{
		return ConstIterator(_head->_next);
	}

	ConstIterator End()const
	{
		return ConstIterator(_head);
	}

	bool Empty()
	{
		return _head->_next == _head;
	}

	Iterator Insert(Iterator pos, const T& x)
	{
		Node* tmp = new Node(x);
		Node* cur = pos._node;
		Node* prev = cur->_prev;

		tmp->_next = cur;
		cur->_prev = tmp;

		prev->_next = tmp;
		tmp->_prev = prev;
		return Iterator(prev);
	}

	Iterator Erase(Iterator pos)
	{
		assert(pos != End());
		Node* cur = pos._node;
		Node* prev = cur->_prev;
		Node* next = cur->_next;

		prev->_next = next;
		next->_prev = prev;
		delete cur;
		return Iterator(next);
	}

	Iterator PushBack(const T& x)
	{
		return Insert(End(), x);
	}
	Iterator PopBack()
	{
		return Erase(--End());
	}
	Iterator PushFront(const T& x)
	{
		return Insert(Begin(),x);
	}

	Iterator PopFront()
	{
		return Erase(Begin());
	}

private:
	Node* _head;
};
template<class T>
void PrintList(const List<T>& l)
{
	List<T>::ConstIterator it = l.Begin();
	while (it != l.End())
	{
		cout << *it << " ";
		it++;
	}
	cout << endl;
}

void Test()
{
	List<int> l;
	l.PushBack(1);
	l.PushBack(2);
	l.PushBack(3);
	l.PushBack(4);
	l.PushFront(5);
	l.PushFront(6);
	l.PushFront(7);
	cout <<"距离为"<< Distance(l.Begin(), l.End())<<endl;
	PrintList(l);
	l.PopBack();
	l.PopBack();
	PrintList(l);

	l.PopFront();
	l.PopFront();
	PrintList(l);
	l.PopBack();
	l.PopBack();
	PrintList(l);

}