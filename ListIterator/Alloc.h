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
//���ڵ���׷�ݵ�trace log
inline static void __trace_debug(const char* function, const char* filename, int line, char*format, ...)
{
#ifdef __DEBUG__
	//������ú�������Ϣ
	fprintf(stdout, "[%s:%d]%s", GetFileName(filename).c_str());

	//����û���trace��Ϣ
	va_list args;
	va_start(args, format);
	vfprintf(stdout, format, args);
	va_end(args);
#endif // __DEBUG__
}
#define __TRACE_DEBUG(...)\
		__trace_debug(__FUNCTION__,__FILE__,__LINE__,__VA_ARGS__);
//һ���ռ�������
typedef void(*ALLOC_OOM_FUN)();
template <int inst>//�����������Ա����ʹ��
class _Malloc_Alloc_Template
{
private:
	//���벻���ռ�Ĵ�����
	static void *Oom_Malloc(size_t);
	static void *Oom_Realloc(void*, size_t);
	static void(*__Malloc_Alloc_Oom_Handler)();
public:
	//����ռ�
	static void* Allocate(size_t n)
	{
		void * result = malloc(n);
		if (0 == result)
			//malloc ���벻���ռ䴦����
			restult = Oom_Malloc(n);
		return result;
	}
	//�ͷ�����Ŀռ�
	static void DeAllocate(void *p, size_t)
	{
		free(p);
	}

	//�޸�����Ŀռ�
	static void* ReAllocate(void* p, size_t, size_t new_sz)
	{
		void * result = realloc(p, new_sz);
		if (0 == result)
			result = Oom_Realloc(p, new_sz);
		return result;
	}
	//���ÿռ䴦����
	//static void(*Set_Malloc_Hander(void(*f)())()
	static ALLOC_OOM_FUN Set_Malloc_Hander(ALLOC_OOM_FUN f)
	{
		ALLOC_OOM_FUN old = __Malloc_Alloc_Oom_Handler;
		__Malloc_Alloc_Oom_Handler = f;
		return old;
	}
};

//Ĭ�����ÿռ䴦����ΪNULL
template<int inst>
ALLOC_OOM_FUN _Malloc_Alloc_Template<inst>::__Malloc_Alloc_Oom_Handler = 0;

template<int inst>
void* _Malloc_Alloc_Template<inst>::Oom_Malloc(size_t n)
{
	ALLOC_OOM_FUN My_Malloc_Hanlder;
	void * result;
	//һֱѭ���Ĳ�����û�пռ�����ͷ�
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
	//һֱѭ���Ĳ�����û�пռ�����ͷ�
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
//�����ռ�������
template <bool threads, int inst>
class _Default_Alloc_Template
{
private:
	enum {_ALIGN =8};//���л�׼ֵ
	enum {_MAX_BYTES = 128};//���ֵ
	enum {_NFREELISTS = _MAX_BYTES/_ALIGN};//�������Ĵ�С
	static size_t ROUND_UP(size_t bytes)//�ֽ����϶���
	{
		return (bytes + _ALIGN - 1)&~(_ALIGN - 1);
	}

	union Obj
	{
		union Obj* Free_List_Link;//ָ�������������һ��
		char Client_Data[1];//�����ͻ���ʹ�ã���ʱ����
	};
	//���������������е��±�
	static size_t FREELIST_INDEX(size_t bytes)
	{
		return (bytes + _ALIGN - 1) / _ALIGN - 1;
	}
	//��ȡ�����ڴ���뵽����������
	static void* Refill(size_t n);
	//���ڴ�ط�������ڴ�
	static char* Chunk_Alloc(size_t size, int &nobs);

	static Obj* volatile _FreeList[_NFREELISTS];//��������
	static char* Start_Free;//�ڴ�صĿ�ʼָ��
	static char* End_Free;//�ڴ�صĽ���ָ��
	static size_t Heap_Size;//��ϵͳ������ڴ��С
public:
	static void *Allocate(size_t n);
	static void DeAllocate(void *p, size_t n);
	static void *ReAllocate(void* p,size_t old_sz, size_t new_sz);
};
//��ʼ��ȫ�ֵľ�̬����
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
	//ֻ���䵽һ�飬��ֱ�ӷ���
	if (1 == nobjs)
		return chunk;

	Obj* result, *cur;
	size_t index = FREELIST_INDEX(n);
	result = (Obj*)chunk;

	//��ʣ��Ŀ�ҵ�������������
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

	//ʣ��Ŀռ���㣬ֱ�ӷ���
	if (bytesLeft >= totalBytes)
	{
		result = Start_Free;
		Start_Free += totalBytes;
		return result;
	}
	//ʣ��Ŀռ䲻�����������ٿ��Է���һ��
	else if (bytesLeft >= size)
	{
		njobs = bytesLeft / size;
		totalBytes = size*nobjs;
		result = Start_Free;
		Start_Free += totalBytes;
		return result;
	}//ʣ��Ŀռ�һ�������ܷ���
	else
	{
		size_t BytesToGet = 2 * totalBytes + ROUND_UP(Heap_Size);
		//��ʣ��Ŀռ�ͷ���Ӧ������������
		if (bytesLeft > 0)
		{
			obj * volatile MyFreeList = _FreeList + FREELIST_INDEX[bytesLeft];
			((Obj*)Start_Free)->Free_List_Link = *MyFreeList;
			*MyFreeList = (Obj*)Start_Free;
		}
		Start_Free = (char*)malloc(BytesToGet);
		//ϵͳ����û���ڴ棬�򿴿�������������û�пռ���Է���
		if (0 == Start_Free)
		{
			int i;
			Obj* volatile * MyFreeList, *p;
			//??ΪʲôҪ�ӵ�ǰ��size��������ʼ����
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
			//��һ���ռ�����������ռ�
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
	//����Ŀռ�������ֵ
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
	cout << "���Ե���һ���ռ������������ڴ�" << endl;
	char* p1= 
}