#pragma once
#include<iostream>

using namespace std;

#define __DEBUG__
static string GetFileName(const string& path)
{
	char ch = '/';
#ifdef _WIN32
	ch = '\\';
#endif // _WIN32
	size_t pos = path.rfind(ch);
	if (pos == string::npos)
		return path;
	else
		return path.substr(pos + 1);
}
//用于调试追溯的trace log
inline static void __trace_debug(const char* function, const char* filename, int line, char*format, ...)
{
#ifdef __DEBUG__
	//输出调用函数的信息
	fprintf(stdout, "[%s:%d]%s", GetFileName(filename).c_str());

	//输出用户的trace信息
	va_list args;
	va_start(args, format);
	vfprintf(stdout, format, args);
	va_end(args);
#endif // __DEBUG__
}
#define __TRACE_DEBUG(...)\
		__trace_debug(__FUNCTION__,__FILE__,__LINE__,__VA_ARGS__);
//一级空间配置器
typedef void(*ALLOC_OOM_FUN)();
template <int inst>//参数保留，以备后边使用
class _Malloc_Alloc_Template
{
private:
	//申请不到空间的处理函数
	static void *Oom_Malloc(size_t);
	static void *Oom_Realloc(void*, size_t);
	static void(*__Malloc_Alloc_Oom_Handler)();
public:
	//申请空间
	static void* Allocate(size_t n)
	{
		void * result = malloc(n);
		if (0 == result)
			//malloc 申请不到空间处理方法
			restult = Oom_Malloc(n);
		return result;
	}
	//释放申请的空间
	static void DeAllocate(void *p, size_t)
	{
		free(p);
	}

	//修改申请的空间
	static void* ReAllocate(void* p, size_t, size_t new_sz)
	{
		void * result = realloc(p, new_sz);
		if (0 == result)
			result = Oom_Realloc(p, new_sz);
		return result;
	}
	//设置空间处理函数
	//static void(*Set_Malloc_Hander(void(*f)())()
	static ALLOC_OOM_FUN Set_Malloc_Hander(ALLOC_OOM_FUN f)
	{
		ALLOC_OOM_FUN old = __Malloc_Alloc_Oom_Handler;
		__Malloc_Alloc_Oom_Handler = f;
		return old;
	}
};

//默认设置空间处理函数为NULL
template<int inst>
ALLOC_OOM_FUN _Malloc_Alloc_Template<inst>::__Malloc_Alloc_Oom_Handler = 0;

template<int inst>
void* _Malloc_Alloc_Template<inst>::Oom_Malloc(size_t n)
{
	ALLOC_OOM_FUN My_Malloc_Hanlder;
	void * result;
	//一直循环的产看有没有空间可以释放
	for (;;)
	{
		My_Malloc_Hanlder = _malloc_alloc_oom_handler;
		if (0 == My_Malloc_Hanlder)
		{
			cerr << "out of memory";
			exit(1);
		}

		My_Malloc_Hanlder();
		result = malloc(n);
		if (result)
			return result;
	}
}

template<int inst>
void* _Malloc_Alloc_Template<inst>::Oom_Realloc(void *p, size_t n)
{
	ALLOC_OOM_FUN My_Malloc_Hanlder;
	void * result;
	//一直循环的产看有没有空间可以释放
	for (;;)
	{
		My_Malloc_Hanlder = _malloc_alloc_oom_handler;
		if (0 == My_Malloc_Hanlder)
		{
			cerr << "out of memory";
			exit(1);
		}
		My_Malloc_Hanlder();
		result = realloc(p,n);
		if (result)
			return result;
	}
}
typedef _Malloc_Alloc_Template<0> MallocAlloc;
//二级空间配置器
template <bool threads, int inst>
class _Default_Alloc_Template
{
private:
	enum {_ALIGN =8};//排列基准值
	enum {_MAX_BYTES = 128};//最大值
	enum {_NFREELISTS = _MAX_BYTES/_ALIGN};//排列链的大小
	static size_t ROUND_UP(size_t bytes)//字节向上对齐
	{
		return (bytes + _ALIGN - 1)&~(_ALIGN - 1);
	}

	union Obj
	{
		union Obj* Free_List_Link;//指向自由链表的下一个
		char Client_Data[1];//留给客户端使用，暂时不用
	};
	//计算在自由链表中的下标
	static size_t FREELIST_INDEX(size_t bytes)
	{
		return (bytes + _ALIGN - 1) / _ALIGN - 1;
	}
	//获取大块的内存插入到自由链表中
	static void* Refill(size_t n);
	//从内存池分配大块的内存
	static char* Chunk_Alloc(size_t size, int &nobs);

	static Obj* volatile _FreeList[_NFREELISTS];//自由链表
	static char* Start_Free;//内存池的开始指针
	static char* End_Free;//内存池的结束指针
	static size_t Heap_Size;//从系统分配的内存大小
public:
	static void *Allocate(size_t n);
	static void DeAllocate(void *p, size_t n);
	static void *ReAllocate(void* p,size_t old_sz, size_t new_sz);
};
//初始化全局的静态对象
template <bool threads, int inst>
typename _Default_Alloc_Template<threads, inst>::Obj*volatile _Default_Alloc_Template<threads, inst>::_FreeList[_Default_Alloc_Template<threads, inst>::_NFREELISTS];

template <bool threads, int inst>
char* _Default_Alloc_Template<threads, inst>::Start_Free = 0;
template <bool threads, int inst>
char* _Default_Alloc_Template<threads, inst>::End_Free = 0;
template <bool threads, int inst>
size_t _Default_Alloc_Template<threads, inst>::Heap_Size = 0;
template <bool threads, int inst>


void* _Default_Alloc_Template<threads, inst>::Refill(size_t n)
{
	int nobjs = 20;
	char* chunk = Chunk_Alloc(n, nobjs);
	//只分配到一块，则直接返回
	if (1 == nobjs)
		return chunk;

	Obj* result, *cur;
	size_t index = FREELIST_INDEX(n);
	result = (Obj*)chunk;

	//将剩余的块挂到自由链表上面
	cur = (Obj*)(chunk + n);
	_FreeList[index] = cur;

	for (int i = 2; i < nobjs; i++)
	{
		cur->Free_List_Link = (Obj*)(chunk + n*i);
		cur = cur->Free_List_Link;
	}

	cur->Free_List_Link = NULL;
	return result;
}

template <bool threads, int inst>
char* _Default_Alloc_Template<threads, inst>::Chunk_Alloc(size_t size, int &nobjs)
{
	char* result;
	size_t totalBytes = size*nobjs;
	size_t bytesLeft = End_Free - Start_Free;

	//剩余的空间充足，直接分配
	if (bytesLeft >= totalBytes)
	{
		result = Start_Free;
		Start_Free += totalBytes;
		return result;
	}
	//剩余的空间不够，但是至少可以分配一个
	else if (bytesLeft >= size)
	{
		njobs = bytesLeft / size;
		totalBytes = size*nobjs;
		result = Start_Free;
		Start_Free += totalBytes;
		return result;
	}//剩余的空间一个都不能分配
	else
	{
		size_t BytesToGet = 2 * totalBytes + ROUND_UP(Heap_Size);
		//将剩余的空间头插对应的自由链表中
		if (bytesLeft > 0)
		{
			obj * volatile MyFreeList = _FreeList + FREELIST_INDEX[bytesLeft];
			((Obj*)Start_Free)->Free_List_Link = *MyFreeList;
			*MyFreeList = (Obj*)Start_Free;
		}
		Start_Free = (char*)malloc(BytesToGet);
		//系统堆中没有内存，则看看自由链表中有没有空间可以分配
		if (0 == Start_Free)
		{
			int i;
			Obj* volatile * MyFreeList, *p;
			//??为什么要从当前的size自由链表开始查找
			for (i = size; i <= _MAX_BYTES; i += _ALIGN;)
			{
				MyFreeList = _FreeList + FREELIST_INDEX(i);
				p = *MyFreeList;
				if (0 != p)
				{
					*MyFreeList = p->Free_List_Link;
					Start_Free = (char*p);
					End_Free = Start_Free + i;
					return(Chunk_Alloc(size, nobjs));
				}
			}
			End_Free = 0;
			//向一级空间配置器请求空间
			Start_Free = (char*)MallocAlloc::Allocate(size);
		}

		Heap_Size += BytesToGet;
		End_Free = Start_Free + BytesToGet;
		return (Chunk_Alloc(size, nobjs));
	}
}

template <bool threads, int inst>
void* _Default_Alloc_Template<threads, inst>::Allocate(size_t n)
{
	Obj* volatile* MyFreeList;
	Obj* result;
	//申请的空间大于最大值
	if (n > (size_t)_MAX_BYTES)
	{
		return MallocAlloc::Allocate(n);
	}
	MyFreeList = MyFreeList + FREELIST_INDEX(n);
	result = *MyFreeList;
	if (result ==0)
	{
		void*r = Refill(ROUND_UP(n));
		return r;
	}
	*MyFreeList = *MyFreeList->Free_List_Link;
	return result;
}

template <bool threads, int inst>
void _Default_Alloc_Template<threads, inst>::DeAllocate(void* p, size_t n)
{
	Obj *q = (Obj*)p;
	Obj *volatile*MyFreeList;

	if (n > (size_t)(_MAX_BYTES))
	{
		MallocAlloc::DeAllocate(p, n);
	}
	MyFreeList = _FreeList + FREELIST_INDEX(n);
	q->Free_List_Link = *MyFreeList;
	*MyFreeList = q;
}


template <bool threads, int inst>
void* _Default_Alloc_Template<threads, inst>::ReAllocate(void* p, size_t old_sz, size_t new_sz)
{
	void* result;
	size_t CopySz;
	if (old_sz > (size_t)_MAX_BYTES && new_sz > (size_t)_MAX_BYTES)
	{
		return MallocAlloc::ReAllocate(p, new_sz);
	}
	if (ROUND_UP(old_sz) == ROUND_UP(new_sz))
		return p;
	result = Allocate(new_sz);
	CopySz = new_sz > old_sz ? old_sz : new_sz;
	memcpy(result, p, CopySz);
	DeAllocate(p, old_sz);
	return result;
}

void Test1()
{
	cout << "测试调用一级空间配置器分配内存" << endl;
	char* p1= 
}