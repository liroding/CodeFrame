#ifndef DEFMALLOC_H
#define DEFMALLOC_H

#include <stdio.h>
#include <stdbool.h>
//默认单个内存池大小
#define MEMPOOLSIZE 100 * 1024 * 1024

//内存管理表
//允许使用者追加分配内存，追加的内存将会保存在下一个结构中
typedef struct MemoryStore
{
    int Count;
    //总容量
    unsigned long long MemVolumeDose;
    //起始地址
    unsigned long long StartAddress;
    //末尾地址
    unsigned long long EndAddress;
    //当前使用量
    unsigned long long CurrentUsageAmount;
    //剩余容量
    unsigned long SurplusVolumeDose;
    struct MemoryStore *Priv, *Next;

    void (*Init)(struct MemoryStore Instance);   //add by liro

}* PMemoryStore;


//链表内存管理结构
typedef struct MemList
{
    //记录是第多少个节点
    int Count;
    //记录内存属于哪个池子
    int nPoolCount;
    //起始地址
    unsigned long StartAddress;
    //末尾地址
    unsigned long EndAddress;
    //当前使用量
    unsigned long CurrentUsgeAmount;
    //标记是否已经保存了数据
    bool bValid;
    struct MemList *Priv, *Next;


	void (*Init)(struct MemList Instance);  //add by liro

}* PMemList;



//代码实现结构
typedef struct _MemoryPool
{
    void *(*MemAlloc)(struct _MemoryPool *pMemPool,int nSize);
    //内存释放
    bool (*MemFree)(struct _MemoryPool *pMemPool,void * ptr);
    //释放内存池 类对象销毁前进行自动释放
    bool (*MemPoolFree)(struct _MemoryPool *pMemPool);
    //获取最后一次出现的错误
    char *(*GetLastError)(struct _MemoryPool *pMemPool);
    //匹配的内存比需要分配的内存多的字节数(最小值 比如默认匹配到的内存比需要分配的内存多一个字节)
    bool (*SetComPareMemMini)(struct _MemoryPool *pMemPool,int nMini);
    //匹配的内存比需要分配的内存多的字节数(最大值 比如默认匹配到的内存比需要分配的内存多五个字节)
    bool (*SetComPareMemMax)(struct _MemoryPool *pMemPool,int nMax);

    //内存池管理
    PMemoryStore m_Memory,m_MemoryEnd;
    //数据管理
    PMemList m_MemHead,m_MemCurrent,m_MemEnd;
    char m_LastError[256];
    //匹配最小值差距
    int m_nCompareMini;
    //匹配最大值差距
    int m_nCompareMax;
    //初始化内存池大小
    unsigned long long m_nInitPoolSize;
    //标记被解除占用的一节
    int m_nCount;


    //执行中最后出现的错误进行保存
    bool (*WriteLastError)(struct _MemoryPool *pMemPool,const char *data);
    //初始化内存池
    bool (*InitMemPool)(struct _MemoryPool *pMemPool,int AllocSize);
    //创建下一个内存池
    bool (*CreateNextMemPool)(struct _MemoryPool *pMemPool,int AllocSize);
    //分配一节内存管理链表信息
    PMemList (*GetMemList)(struct _MemoryPool *pMemPool); 
    //初始化内存管理链表头信息
    bool (*InitMemHead)(struct _MemoryPool *pMemPool);
    //获取内存管理管理头信息
    PMemList (*GetMemListHead)(struct _MemoryPool *pMemPool);
    //从内存池中分配一块内存
    void *(*GetPoolMem)(struct _MemoryPool *pMemPool,int nSize);
    //获取首个内存池信息
    PMemoryStore (*GetPoolHead)(struct _MemoryPool *pMemPool);
    //获取最后一个内存池信息
    PMemoryStore (*GetPoolEnd)(struct _MemoryPool *pMemPool);
    //从一块内存池中获取数据
    void* (*GetAPoolofMem)(struct _MemoryPool *pMemPool,PMemoryStore obj,int nSize);
    //获取最后一个节点
    PMemList (*GetEndList)(struct _MemoryPool *pMemPool);
    //创建第一个节点以及分配内存
    void* (*CreateFirstMem)(struct _MemoryPool *pMemPool,int nSize,bool bValid);
    //创建新的节点以及分配内存 bValid为true标记为已使用内存 ，false标记为未使用
    void *(*CreateNextMem)(struct _MemoryPool *pMemPool,PMemoryStore obj,int nSize,bool bValid);
    //创建第一个节点
    void *(*CreateListHead)(struct _MemoryPool *pMemPool,int nSize);
    //获取节点头
    PMemList (*GetHeadList)(struct _MemoryPool *pMemPool);
    //释放指定编号内存池
    bool (*_MemPoolFree)(struct _MemoryPool *pMemPool,int nCount);
    //修改指定内存块当前使用量
    bool (*RevampMemUsgeAmount)(struct _MemoryPool *pMemPool,int nCount,long long nSize);
    //解除占用标记的某块内存 ，释放类对象前统一释放
    bool (*MemListFree)(struct _MemoryPool *pMemPool,int nPoolCount);
    //标记第一个解除占用的内存
    bool (*SetAllocList)(struct _MemoryPool *pMemPool,int Count);
    //获取第一个被解除占用的内存节点
    int (*GetAllocList)(struct _MemoryPool *pMemPool);
    //使用被解除占用的节点分配内存
    void *(*AllocMemList)(struct _MemoryPool *pMemPool,int nCount,int nSize,bool bValid);
    //根据计数获取list节点
    PMemList (*GetCountList)(struct _MemoryPool *pMemPool,int Count);
    //获取可以分出内存的池子，这个直接从空闲内存中取出需要的内存
    PMemoryStore (*GetValidMemPool)(struct _MemoryPool *pMemPool,int nSize);
    //释放指定节点，返回下一个节点
    PMemList (*FreeList)(struct _MemoryPool *pMemPool,int Count);
    //释放链表的所有节点
    bool (*FreeAllList)(struct _MemoryPool *pMemPool);


    //初始化struct
    void (*Init)(struct _MemoryPool *pMemPool);
}MemoryPool;


void *MemAlloc(MemoryPool *pMemPool,int nSize);
//内存释放
bool MemFree(MemoryPool *pMemPool,void * ptr);
//释放内存池 类对象销毁前进行自动释放
bool MemPoolFree(MemoryPool *pMemPool);
//获取最后一次出现的错误
char *GetLastError(MemoryPool *pMemPool);
//匹配的内存比需要分配的内存多的字节数(最小值 比如默认匹配到的内存比需要分配的内存多一个字节)
bool SetComPareMemMini(MemoryPool *pMemPool,int nMini);
//匹配的内存比需要分配的内存多的字节数(最大值 比如默认匹配到的内存比需要分配的内存多五个字节)
bool SetComPareMemMax(MemoryPool *pMemPool,int nMax);

//内存池管理
PMemoryStore m_Memory,m_MemoryEnd;
//数据管理
PMemList m_MemHead,m_MemCurrent,m_MemEnd;
char m_LastError[256];
//匹配最小值差距
int m_nCompareMini;
//匹配最大值差距
int m_nCompareMax;
//初始化内存池大小
unsigned long long m_nInitPoolSize;
//标记被解除占用的一节
int m_nCount;


//执行中最后出现的错误进行保存
bool WriteLastError(MemoryPool *pMemPool,const char *data);
//初始化内存池
bool InitMemPool(MemoryPool *pMemPool,int AllocSize);
//创建下一个内存池
bool CreateNextMemPool(MemoryPool *pMemPool,int AllocSize);
//分配一节内存管理链表信息
PMemList GetMemList(MemoryPool *pMemPool); 
//初始化内存管理链表头信息
bool InitMemHead(MemoryPool *pMemPool);
//获取内存管理管理头信息
PMemList GetMemListHead(MemoryPool *pMemPool);
//从内存池中分配一块内存
void *GetPoolMem(MemoryPool *pMemPool,int nSize);
//获取首个内存池信息
PMemoryStore GetPoolHead(MemoryPool *pMemPool);
//获取最后一个内存池信息
PMemoryStore GetPoolEnd(MemoryPool *pMemPool);
//从一块内存池中获取数据
void *GetAPoolofMem(MemoryPool *pMemPool,PMemoryStore obj,int nSize);
//获取最后一个节点
PMemList GetEndList(MemoryPool *pMemPool);
//创建第一个节点以及分配内存
void *CreateFirstMem(MemoryPool *pMemPool,int nSize,bool bValid);
//创建新的节点以及分配内存 bValid为true标记为已使用内存 ，false标记为未使用
void *CreateNextMem(MemoryPool *pMemPool,PMemoryStore obj,int nSize,bool bValid);
//创建第一个节点
void *CreateListHead(MemoryPool *pMemPool,int nSize);
//获取节点头
PMemList GetHeadList(MemoryPool *pMemPool);
//释放指定编号内存池
bool _MemPoolFree(MemoryPool *pMemPool,int nCount);
//修改指定内存块当前使用量
bool RevampMemUsgeAmount(MemoryPool *pMemPool,int nCount,long long nSize);
//解除占用标记的某块内存 ，释放类对象前统一释放
bool MemListFree(MemoryPool *pMemPool,int nPoolCount);
//标记第一个解除占用的内存
bool SetAllocList(MemoryPool *pMemPool,int Count);
//获取第一个被解除占用的内存节点
int GetAllocList(MemoryPool *pMemPool);
//使用被解除占用的节点分配内存
void *AllocMemList(MemoryPool *pMemPool,int nCount,int nSize,bool bValid);
//根据计数获取list节点
PMemList GetCountList(MemoryPool *pMemPool,int Count);
//获取可以分出内存的池子，这个直接从空闲内存中取出需要的内存
PMemoryStore GetValidMemPool(MemoryPool *pMemPool,int nSize);
//释放指定节点，返回下一个节点
PMemList FreeList(MemoryPool *pMemPool,int Count);
//释放链表的所有节点
bool FreeAllList(MemoryPool *pMemPool);


void MemoryPoolInit(MemoryPool *pMemPool);
void MemoryStoreInit(struct MemoryStore Instance);
void MemListInit(struct MemList Instance);




#endif

