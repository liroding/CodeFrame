#include <stdio.h>
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
    MemoryStore *Priv, *Next;

    void Init()
    {
        Count = MemVolumeDose = StartAddress = EndAddress = CurrentUsageAmount = SurplusVolumeDose = 0;
        Priv = Next = 0;
    }
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
    MemList *Priv, *Next;

    void Init()
    {
        Count = nPoolCount = StartAddress = EndAddress = CurrentUsgeAmount = 0;
        bValid = 0;
        Priv = Next = 0;
    }
}* PMemList;



//代码实现结构
struct MemoryPool_
{
    MemoryPool_(unsigned long long AllocSize = MEMPOOLSIZE);
    virtual ~MemoryPool_();

    //内存分配
    void *MemAlloc(int nSize);
    //内存释放
    bool MemFree(void * ptr);
    //释放内存池 类对象销毁前进行自动释放
    bool MemPoolFree();
    //获取最后一次出现的错误
    char *GetLastError();
    //匹配的内存比需要分配的内存多的字节数(最小值 比如默认匹配到的内存比需要分配的内存多一个字节)
    bool SetComPareMemMini(int nMini);
    //匹配的内存比需要分配的内存多的字节数(最大值 比如默认匹配到的内存比需要分配的内存多五个字节)
    bool SetComPareMemMax(int nMax);

private://内部使用
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
private:
    //执行中最后出现的错误进行保存
    bool WriteLastError(const char *data);
    //初始化内存池
    bool InitMemPool(int AllocSize);
    //创建下一个内存池
    bool CreateNextMemPool(int AllocSize);
    //分配一节内存管理链表信息
    PMemList GetMemList();
    //初始化内存管理链表头信息
    bool InitMemHead();
    //获取内存管理管理头信息
    PMemList GetMemListHead();
    //从内存池中分配一块内存
    void *GetPoolMem(int nSize);
    //获取首个内存池信息
    PMemoryStore GetPoolHead();
    //获取最后一个内存池信息
    PMemoryStore GetPoolEnd();
    //从一块内存池中获取数据
    void *GetAPoolofMem(PMemoryStore obj,int nSize);
    //获取最后一个节点
    PMemList GetEndList();
    //创建第一个节点以及分配内存
    void *CreateFirstMem(int nSize,bool bValid);
    //创建新的节点以及分配内存 bValid为true标记为已使用内存 ，false标记为未使用
    void *CreateNextMem(PMemoryStore obj,int nSize,bool bValid);
    //创建第一个节点
    void *CreateListHead(int nSize);
    //获取节点头
    PMemList GetHeadList();
    //释放指定编号内存池
    bool MemPoolFree_(int nCount);
    //修改指定内存块当前使用量
    bool RevampMemUsgeAmount(int nCount,long long nSize);
    //解除占用标记的某块内存 ，释放类对象前统一释放
    bool MemListFree_(int nPoolCount);
    //标记第一个解除占用的内存
    bool SetAllocList(int Count);
    //获取第一个被解除占用的内存节点
    int GetAllocList();
    //使用被解除占用的节点分配内存
    void *AllocMemList(int nCount,int nSize,bool bValid);
    //根据计数获取list节点
    PMemList GetCountList(int Count);
    //获取可以分出内存的池子，这个直接从空闲内存中取出需要的内存
    PMemoryStore GetValidMemPool(int nSize);
    //释放指定节点，返回下一个节点
    PMemList FreeList_(int Count);
    //释放链表的所有节点
    bool FreeList();
};


#include "stdafx.h"
#include "TestMemoryPool.h"

MemoryPool_::MemoryPool_(unsigned long long AllocSize):m_Memory(0),m_MemHead(0)
    ,m_MemCurrent(0),m_MemEnd(0),m_nInitPoolSize(AllocSize)
{
    m_nCompareMini = 1;
    m_nCompareMax = 5;
    memset(m_LastError,0,sizeof(m_LastError));
}

MemoryPool_::~MemoryPool_()
{
    MemPoolFree();
    FreeList();
}

bool MemoryPool_::SetComPareMemMini(int nMini)
{
    if (1 >= nMini)
    {
        return false;
    }
    m_nCompareMini = nMini;
    return true;
}

bool MemoryPool_::SetComPareMemMax(int nMax)
{
    if (1 >= nMax)
    {
        return false;
    }
    m_nCompareMax = nMax;
    return true;
}

//获取执行中出现的最后一条错误
char *MemoryPool_::GetLastError()
{
    return m_LastError;
}

//写入错误信息
bool MemoryPool_::WriteLastError(const char *data)
{
    if (0 == data)
        return false;
    memset(m_LastError,0,sizeof(m_LastError));
    memcpy(m_LastError,data,sizeof(data));
    return true;
}
//初始化内存池
bool MemoryPool_::InitMemPool(int AllocSize)
{
    if (0 == m_Memory)
    {
        m_Memory = (PMemoryStore)malloc(sizeof(MemoryStore));
        m_Memory->Init();
    }
    if (0 == m_Memory)
        return false;
    //构建池子
    if (0 < AllocSize)
    {
        m_Memory->MemVolumeDose = AllocSize;
        char *Mem = (char *)malloc(AllocSize);
        m_Memory->StartAddress = (unsigned long long)Mem;
        m_Memory->EndAddress = (m_Memory->StartAddress + AllocSize);
    }
    else
    {
        m_Memory->MemVolumeDose = MEMPOOLSIZE;
        char *Mem = (char *)malloc(MEMPOOLSIZE);
        m_Memory->StartAddress = (unsigned long long)Mem;
        m_Memory->EndAddress = (m_Memory->StartAddress + MEMPOOLSIZE);
    }
    m_Memory->Count = 1;
    m_Memory->CurrentUsageAmount = 0;
    m_Memory->SurplusVolumeDose = m_Memory->MemVolumeDose;

    //分配内存失败
    if (0 ==  m_Memory->StartAddress)
    {
        WriteLastError("this MemoryAlloc is Not Valid");
        return false;
    }
    m_MemoryEnd = m_Memory;
    return true; 
}

//创建下一个内存池
bool MemoryPool_::CreateNextMemPool(int AllocSize)
{
    PMemoryStore memoryPool = GetPoolHead();
    if (0 == memoryPool)
    {
        InitMemPool(((AllocSize + m_nCompareMini >= MEMPOOLSIZE) ? (AllocSize + m_nCompareMini) : MEMPOOLSIZE));
        return true;
    }
    while (memoryPool && 0 != memoryPool->Next)
        memoryPool = memoryPool->Next;
    memoryPool->Next = (PMemoryStore)malloc(sizeof(MemoryStore));
    memoryPool->Next->Init();
    //构建池子
    if (0 < AllocSize)
    {
        memoryPool->Next->MemVolumeDose = AllocSize;
        char *Mem = (char *)malloc(AllocSize);
        memoryPool->Next->StartAddress = (unsigned long long)Mem;
        memoryPool->Next->EndAddress = (memoryPool->Next->StartAddress + AllocSize);
    }
    else
    {
        memoryPool->Next->MemVolumeDose = MEMPOOLSIZE;
        char *Mem = (char *)malloc(MEMPOOLSIZE);
        memoryPool->Next->StartAddress = (unsigned long long)Mem;
        memoryPool->Next->EndAddress = (memoryPool->Next->StartAddress + MEMPOOLSIZE);
    }
    memoryPool->Next->Count = (memoryPool->Count + 1);
    memoryPool->Next->CurrentUsageAmount = 0;
    memoryPool->Next->SurplusVolumeDose = memoryPool->Next->MemVolumeDose;

    //分配内存失败
    if (0 ==  memoryPool->Next->StartAddress)
    {
        WriteLastError("this MemoryAlloc is Not Valid");
        return false;
    }
    m_MemoryEnd = memoryPool->Next;
    m_MemoryEnd->Priv = memoryPool;
    return true; 
}

//内存分配
void *MemoryPool_::MemAlloc(int nSize)
{
    //增加头节点
    if (0 == GetMemListHead())
    {

        if (!InitMemPool(m_nInitPoolSize))
        {
            WriteLastError("this Init is Not Valid");
            return 0;
        }
        return CreateListHead(nSize);
    }
    else
    {
        return GetPoolMem(nSize);
    }
}
//创建第一个节点
void *MemoryPool_::CreateListHead(int nSize)
{
    return CreateFirstMem(nSize,1);
}
//获取节点头
PMemList MemoryPool_::GetHeadList()
{
    return m_MemHead;
}
//获取首个内存池信息
PMemoryStore MemoryPool_::GetPoolHead()
{
    return m_Memory;
}
//获取最后一个内存池信息
PMemoryStore MemoryPool_::GetPoolEnd()
{
    return m_MemoryEnd;
}
//从所有内存池中取出未使用的内存地址
void *MemoryPool_::GetPoolMem(int nSize)
{
    PMemoryStore pool = GetPoolHead();

    while (pool)
    {
        char *pData = (char *)GetAPoolofMem(pool,nSize);
        if (0 != pData)
            return (void *)pData;
        pool = pool->Next;
    }   
    //如果所有的池子都遍历了还是没有合适内存，那么创建一个新的池子
    if ((nSize + m_nCompareMini)  > MEMPOOLSIZE)
        CreateNextMemPool(nSize + m_nCompareMini);
    else
        CreateNextMemPool(MEMPOOLSIZE);

        char *pData = (char *)GetAPoolofMem(m_MemoryEnd,nSize);
        return (void *)pData;
}

//从一块内存池中获取数据
void *MemoryPool_::GetAPoolofMem(PMemoryStore obj,int nSize)
{
    if (0 >= obj->SurplusVolumeDose || nSize >= obj->SurplusVolumeDose)
    {
        return 0;
    }
        //如果达到查询的条件 开始遍历对应编号的内存池 ，为了最大利用内存选择从头开始遍历，如果没有数据插入到最后
        PMemList listData = GetMemListHead();
        while (listData && (0 != listData->Next))
        {
            //判断是否有解除占用的节点，如果有的话记录第一个，如果没有找到合适的内存链 ，那么用这个进行分配内存
            if ((listData->nPoolCount == 0) && (0 < listData->Count))
            {
                SetAllocList(listData->Count);
            }

            //如果节点中保存的内存使用量大于或等于需要分配的内存，那么使用这块内存
            if (((nSize + m_nCompareMini  <= listData->CurrentUsgeAmount) && (nSize + m_nCompareMax >= listData->CurrentUsgeAmount)) && (0 == listData->bValid))
            {
                RevampMemUsgeAmount(listData->nPoolCount,listData->CurrentUsgeAmount);
                listData->bValid = 1;
                return (void *)listData->StartAddress;
            }
            listData = listData->Next;
        }
        int nCount = GetAllocList();
        if (0 < nCount)
            return AllocMemList(nCount,nSize,1);
    //创建新的节点保存分配内存
    return CreateNextMem(obj,nSize,1);
}

//标记第一个被解除占用的内存节点
bool MemoryPool_::SetAllocList(int Count)
{
    if (0 >= Count || 0 < m_nCount)
        return false;
    m_nCount = Count;
    return true;
}

//获取第一个被解除占用的内存节点,当前保存的节点被使用后可以再次储存下一个被解除占用的内存节点
int MemoryPool_::GetAllocList()
{
    int Count = m_nCount;
    m_nCount = 0;
    return Count;
}

//修改指定内存块当前使用量
bool MemoryPool_::RevampMemUsgeAmount(int nCount,long long nSize)
{
    if (0 >= nCount)
        return false;
    PMemoryStore memPool = GetPoolHead();
    while (memPool->Count != nCount)
        memPool = memPool->Next;

    if (0 != memPool)
    {
        memPool->CurrentUsageAmount += nSize;
        memPool->SurplusVolumeDose = (memPool->MemVolumeDose - memPool->CurrentUsageAmount);
    }
    else
        return false;

    return true;
}

//创建第一个节点以及分配内存 ,如果不是第一个节点 走默认函数
void *MemoryPool_::CreateFirstMem(int nSize,bool bValid)
{
    //如果头节点已经创建 调用默认内存分配
    if (0 != m_MemHead)
        return GetPoolMem(nSize);

    PMemoryStore pool = GetPoolHead();

    m_MemHead = GetMemList();
    m_MemHead->Count = 1;
    m_MemHead->StartAddress = (pool->StartAddress + pool->CurrentUsageAmount);
    //多分配一个字节用来防止内存越界
    m_MemHead->EndAddress = (m_MemHead->StartAddress + nSize + 1);
    m_MemHead->CurrentUsgeAmount = (nSize + 1);
    m_MemHead->nPoolCount = pool->Count;
    m_MemHead->bValid = bValid;
    pool->CurrentUsageAmount += (nSize + 1);
    pool->SurplusVolumeDose -= pool->CurrentUsageAmount;
    m_MemEnd = m_MemHead;
    //分配出一段干净的内存 上层方便使用
    memset((void *)m_MemHead->StartAddress,0,nSize + 1);
    return (void *)m_MemHead->StartAddress;
}

//创建新的节点以及分配内存
void *MemoryPool_::CreateNextMem(PMemoryStore obj,int nSize,bool bValid)
{
    PMemList list = GetEndList();
    list->Next = GetMemList();
    list->Next->Count = (list->Count + 1);
    list->Next->StartAddress = (obj->StartAddress + obj->CurrentUsageAmount);
    //多分配一个字节用来防止内存越界
    list->Next->EndAddress = (list->Next->StartAddress + nSize + 1);
    list->Next->CurrentUsgeAmount = (nSize + 1);
    list->Next->nPoolCount = obj->Count;
    list->Next->Priv = list;
    list->Next->bValid = bValid;
    obj->CurrentUsageAmount += (nSize + 1);
    obj->SurplusVolumeDose -= obj->CurrentUsageAmount;
    m_MemEnd = list->Next;
    //分配出一段干净的内存 上层方便使用
    memset((void *)list->Next->StartAddress,0,nSize + 1);
    return (void *)list->Next->StartAddress;
}

//获取可以分出内存的池子，这个直接从空闲内存中取出需要的内存
PMemoryStore MemoryPool_::GetValidMemPool(int nSize)
{
    PMemoryStore pool = GetPoolHead();
    while (pool)
    {
        if (pool->SurplusVolumeDose >= (nSize + m_nCompareMini))
        {
            return pool;
        }
        pool = pool->Next;
    }
    //如果没有 就创建一个新的内存池
    if (CreateNextMemPool(((nSize + m_nCompareMini) >= MEMPOOLSIZE ? (nSize + m_nCompareMini) : MEMPOOLSIZE)))
        return GetPoolEnd();

    return 0;
}

//使用被解除占用的节点分配内存
void *MemoryPool_::AllocMemList(int nCount,int nSize,bool bValid)
{
    PMemList list = GetCountList(nCount);
    if (0 == list)
        return 0;
    PMemoryStore memPool = GetValidMemPool(nSize);
    if (0 == memPool)
        return 0;

    list->StartAddress = (memPool->StartAddress + memPool->CurrentUsageAmount);
    //多分配一个字节用来防止内存越界
    list->EndAddress = (list->StartAddress + nSize + 1);
    list->CurrentUsgeAmount = (nSize + 1);
    list->nPoolCount = memPool->Count;
    list->bValid = bValid;
    memPool->CurrentUsageAmount += (nSize + 1);
    memPool->SurplusVolumeDose = (memPool->MemVolumeDose - memPool->CurrentUsageAmount);
    //分配出一段干净的内存 方便使用
    memset((void *)list->StartAddress,0,nSize + 1);
    return (void *)list->StartAddress;
}

//根据计数获取list节点
PMemList MemoryPool_::GetCountList(int Count)
{
    if (0 < Count)
    {
        PMemList list = GetHeadList();
        while (list)
        {
            if (list->Count == Count)
            {
                return list;
            }
            list = list->Next;
        }
    }
    return 0;
}

//获取最后一个节点
PMemList MemoryPool_::GetEndList()
{
    return m_MemEnd;
}

//获取链表内存结构头节点
PMemList MemoryPool_::GetMemListHead()
{
    return m_MemHead;
}

//创建链表内存结构头
bool MemoryPool_::InitMemHead()
{
    m_MemHead = GetMemList();
    m_MemCurrent = m_MemEnd = m_MemHead;
    return (m_MemHead != 0 ? true : false);
}

//创建链接结构节点
PMemList MemoryPool_::GetMemList()
{
    PMemList list = (PMemList)malloc(sizeof(MemList));
    list->Init();
    return list;
}

//内存释放
bool MemoryPool_::MemFree(void * ptr)
{
    //根据分配的地址在内存池中匹配，匹配到后 修改结构数据后，等待再次使用
    PMemList list = GetMemListHead();
    while (list)
    {
        //如果链表中其中一节数据与需要释放的地址相同 ，而且这段数据属于使用中，属于这块内存
        if ((list->StartAddress == (unsigned long)ptr) && (1 == list->bValid))
        {
            RevampMemUsgeAmount(list->nPoolCount,~(long long)list->CurrentUsgeAmount + 1);
            //回收的时候不需要初始化内存，因为再次使用的时候会进行初始化
            list->bValid = 0;
            ptr = 0;
            return true;
        }
        list = list->Next;
    }
    return false;
}

//释放内存池 ，这个不需要手动释放 ，类对象销毁前会进行释放
bool MemoryPool_::MemPoolFree()
{
    PMemoryStore memPool = GetPoolHead();
    while (0 != memPool)
    {
        PMemoryStore next =  memPool->Next;
        MemPoolFree_(memPool->Count);
        memPool = next;
    }
    return true;
}

//释放指定编号内存池
bool MemoryPool_::MemPoolFree_(int nCount)
{
    PMemoryStore memPool = GetPoolHead();
    while (memPool->Count != nCount)
        memPool = memPool->Next;

    if (0 == memPool)
        return false;

    PMemoryStore priv = 0,next = 0;
    if (0 != memPool->Priv)
        priv = memPool->Priv;
    if (0 != memPool->Next)
        next = memPool->Next;

    MemListFree_(memPool->Count);
    delete memPool;
    memPool = 0;
    if (0 != priv)
        priv->Next = next;
    else
        m_Memory = next;
    if (0 != next)
        next->Priv = priv;
    else
        m_MemoryEnd = m_Memory;
    return true;
}

//解除占用标记的某块内存 ，释放类对象前统一释放
bool MemoryPool_::MemListFree_(int nPoolCount)
{
    PMemList list = GetHeadList();
    while (list)
    {
        if (list->nPoolCount == nPoolCount)
        {
            list->nPoolCount = 0;
            list->StartAddress = list->EndAddress = list->CurrentUsgeAmount = 0;
            list->bValid = 0;
        }
        list = list->Next;
    }
    return true;
}
//释放指定节点，返回下一个节点
PMemList MemoryPool_::FreeList_(int Count)
{
    PMemList list = GetCountList(Count);
    if (0 == list)
        return 0;

    PMemList priv = 0,next = 0;
    if (0 != list->Priv)
        priv = list->Priv;
    if (0 != list->Next)
        next = list->Next;

    delete list;
    if (0 != priv)
        priv->Next = next;
    else
        m_MemHead = next;
    if (0 != next)
        next->Priv = priv;
    else
        m_MemEnd = m_MemHead;
    return next;
}
//释放链表的所有节点
bool MemoryPool_::FreeList()
{
    PMemList list = GetHeadList();
    while (list)
    {
        list = FreeList_(list->Count);
    }
    return true;
}


    MemoryPool_ pool;
    char *Test1 = (char *)pool.MemAlloc(100);
    memcpy(Test1,"您好12312321312",sizeof("您好12312321312"));
    memset(Test1,0,sizeof(Test1));
    char *Test2 = (char *)pool.MemAlloc(100);
    memcpy(Test2,"您好12312321312",sizeof("您好12312321312"));
    memset(Test2,0,sizeof(Test2));
    char *Test3 = (char *)pool.MemAlloc(100 * 1024 * 1024);
    memcpy(Test3,"您好12312321312",sizeof("您好12312321312"));
    memset(Test3,0,sizeof(Test3));
    char *Test4 = (char *)pool.MemAlloc(100 * 1024);
    memcpy(Test4,"您好12312321312",sizeof("您好12312321312"));

    pool.MemFree(Test1);
    Test1 = 0;
    pool.MemFree(Test2);
    Test2 = 0;
    pool.MemFree(Test3);
    Test3 = 0;
    pool.MemFree(Test4);
    Test4 = 0;

    Test1 = (char *)pool.MemAlloc(100);
    memcpy(Test1,"您好12312321312",sizeof("您好12312321312"));
    memset(Test1,0,sizeof(Test1));
    Test2 = (char *)pool.MemAlloc(100);
    memcpy(Test2,"您好12312321312",sizeof("您好12312321312"));
    memset(Test2,0,sizeof(Test2));
    Test3 = (char *)pool.MemAlloc(100 * 1024 * 1024);
    memcpy(Test3,"您好12312321312",sizeof("您好12312321312"));
    memset(Test3,0,sizeof(Test3));
    Test4 = (char *)pool.MemAlloc(100 * 1024);
    memcpy(Test4,"您好12312321312",sizeof("您好12312321312"));

    pool.MemPoolFree();

    Test1 = (char *)pool.MemAlloc(100);
    memcpy(Test1,"您好12312321312",sizeof("您好12312321312"));
    memset(Test1,0,sizeof(Test1));
    Test2 = (char *)pool.MemAlloc(100);
    memcpy(Test2,"您好12312321312",sizeof("您好12312321312"));
    memset(Test2,0,sizeof(Test2));
    Test3 = (char *)pool.MemAlloc(100 * 1024 * 1024);
    memcpy(Test3,"您好12312321312",sizeof("您好12312321312"));
    memset(Test3,0,sizeof(Test3));
    Test4 = (char *)pool.MemAlloc(100 * 1024);
    memcpy(Test4,"您好12312321312",sizeof("您好12312321312"));

