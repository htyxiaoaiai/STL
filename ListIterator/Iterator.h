#pragma once
// ���������ͱ�
//
struct InputIteratorTag {};
struct OutputIteratorTag {};
struct ForwardIteratorTag : public InputIteratorTag {};
struct BidirectionalIteratorTag : public ForwardIteratorTag {};
struct RandomAccessIteratorTag : public BidirectionalIteratorTag {};
//
// ��������Ƕ����������Ӧ���ͱ�
// Iterator Category ��Value Type�� Difference Type��Pointer�� Reference
// ������Ƕ���ͱ��壬ȷ�����ܹ�������ĸ� STL�ںϡ�
// �ҷ���Iterator Traits��������ȡ
//
template <class Category, class T, class Distance = ptrdiff_t,
class Pointer = T*, class Reference = T&>
struct Iterator
{
	typedef Category IteratorCategory; // ����������
	typedef T ValueType; // ��������ָ��������
	typedef Distance DifferenceType; // ����������֮��ľ���
	typedef Pointer Pointer; // ��������ָ�������͵�ָ��
	typedef Reference Reference; // ��������ָ�������͵�����
};
//
// Traits ����һ̨��������ȡ������եȡ���������������ԣ���Ӧ���ͱ�
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
// ƫ�ػ�ԭ��ָ������
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
// ƫ�ػ�constԭ��ָ������
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
//��ԭ��ָ��
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

//ԭ��ָ��
//tyname��ʾ�ĸ��߱�����IteratorTraits<InputIterator>::DifferenceType��һ��������
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
// Advance ��ʵ��
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