#pragma once
// 迭代器的型别
//
struct InputIteratorTag {};
struct OutputIteratorTag {};
struct ForwardIteratorTag : public InputIteratorTag {};
struct BidirectionalIteratorTag : public ForwardIteratorTag {};
struct RandomAccessIteratorTag : public BidirectionalIteratorTag {};
//
// 迭代器内嵌包含的种相应的型别
// Iterator Category 、Value Type、 Difference Type、Pointer、 Reference
// 这种内嵌的型别定义，确保了能够更方便的跟 STL融合。
// 且方便Iterator Traits的类型萃取
//
template <class Category, class T, class Distance = ptrdiff_t,
class Pointer = T*, class Reference = T&>
struct Iterator
{
	typedef Category IteratorCategory; // 迭代器类型
	typedef T ValueType; // 迭代器所指对象类型
	typedef Distance DifferenceType; // 两个迭代器之间的距离
	typedef Pointer Pointer; // 迭代器所指对象类型的指针
	typedef Reference Reference; // 迭代器所指对象类型的引用
};
//
// Traits 就像一台“特性萃取机”，榨取各个迭代器的特性（对应的型别）
//
template <class Iterator>
struct IteratorTraits
{
	typedef typename Iterator::IteratorCategory IteratorCategory;
	typedef typename Iterator::ValueType ValueType;
	typedef typename Iterator::DifferenceType DifferenceType;
	typedef typename Iterator::Pointer Pointer;
	typedef typename Iterator::Reference Reference;
};
//
// 偏特化原生指针类型
//
template <class T>
struct IteratorTraits< T*>
{
	typedef RandomAccessIteratorTag IteratorCategory;
	typedef T ValueType;
	typedef ptrdiff_t DifferenceType;
	typedef T * Pointer;
	typedef T & Reference;
};
//
// 偏特化const原生指针类型
//
template <class T>
struct IteratorTraits< const T *>
{
	typedef RandomAccessIteratorTag IteratorCategory;
	typedef T ValueType;
	typedef ptrdiff_t DifferenceType;
	typedef const T* Pointer;
	typedef const T& Reference;
};
//非原生指针
template<class InputIterator>
inline size_t  _Distance(InputIterator first, InputIterator last,InputIteratorTag)
{
	IteratorTraits<InputIterator>::DifferenceType n = 0;
	while (first!= last)
	{
		n++;
		first++;
	}
	return n;
}

//原生指针
//tyname显示的告诉编译器IteratorTraits<InputIterator>::DifferenceType是一个类型名
template<class InputIterator> 
inline size_t _Distance(InputIterator first, InputIterator last, RandomAccessIteratorTag)
{
	return last - first;
}
template<class InputIterator>
inline size_t Distance(InputIterator first, InputIterator last)
{
	return _Distance(first, last, IteratorTraits<InputIterator>::IteratorCategory());
}


///////////////////////////////////////////////////////////
// Advance 的实现
template <class InputIterator, class Distance>
inline void __Advance(InputIterator & i, Distance n, InputIteratorTag)
{
	while (n--) ++i;
}
template <class InputIterator, class Distance>
inline void __Advance(InputIterator & i, Distance n,
	BidirectionalIteratorTag)
{
	if (n >= 0)
		while (n--) ++i;
	else
		while (n++) --i;
}
template <class InputIterator, class Distance>
inline void __Advance(InputIterator & i, Distance n,
	RandomAccessIteratorTag)
{
	i += n;
}

template <class InputIterator, class Distance>
inline void Advance(InputIterator & i, Distance n)
{
	__Advance(i, n, IteratorTraits <InputIterator>::IteratorCategory());
}