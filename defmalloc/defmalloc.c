#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "defmalloc.h"


bool SetComPareMemMini(MemoryPool *pMemPool,int nMini)
{
    if (1 >= nMini)
    {
        return false;
    }
    pMemPool->m_nCompareMini = nMini;
    return true;
}

bool SetComPareMemMax(MemoryPool *pMemPool,int nMax)
{
    if (1 >= nMax)
    {
        return false;
    }
    pMemPool->m_nCompareMax = nMax;
    return true;
}

//获取执行中出现的最后一条错误
char * GetLastError(MemoryPool *pMemPool)
{
    return pMemPool->m_LastError;
}

//写入错误信息
bool WriteLastError(MemoryPool *pMemPool,const char *data)
{
    if (0 == data)
        return false;
    memset(pMemPool->m_LastError,0,sizeof(pMemPool->m_LastError));
    memcpy(pMemPool->m_LastError,data,sizeof(data));
    return true;
}
//初始化内存池
bool InitMemPool(MemoryPool *pMemPool,int AllocSize)
{
    if (0 == pMemPool->m_Memory)
    {
        pMemPool->m_Memory = (PMemoryStore)malloc(sizeof(struct MemoryStore));
        pMemPool->m_Memory->Init = MemoryStoreInit;
		pMemPool->m_Memory->Init(*(pMemPool->m_Memory));
    }
    if (0 == pMemPool->m_Memory)
        return false;
    //构建池子
    if (0 < AllocSize)
    {
        pMemPool->m_Memory->MemVolumeDose = AllocSize;
        char *Mem = (char *)malloc(AllocSize);
        pMemPool->m_Memory->StartAddress = (unsigned long long)Mem;
        pMemPool->m_Memory->EndAddress = (pMemPool->m_Memory->StartAddress + AllocSize);
    }
    else
    {
        pMemPool->m_Memory->MemVolumeDose = MEMPOOLSIZE;
        char *Mem = (char *)malloc(MEMPOOLSIZE);
        pMemPool->m_Memory->StartAddress = (unsigned long long)Mem;
        pMemPool->m_Memory->EndAddress = (pMemPool->m_Memory->StartAddress + MEMPOOLSIZE);
    }

	printf("<liro-debug> 0x%lx\n",MEMPOOLSIZE);

	
    pMemPool->m_Memory->Count = 1;
    pMemPool->m_Memory->CurrentUsageAmount = 0;
    pMemPool->m_Memory->SurplusVolumeDose = pMemPool->m_Memory->MemVolumeDose;

    //分配内存失败
    if (0 ==  pMemPool->m_Memory->StartAddress)
    {
        pMemPool->WriteLastError(pMemPool,"this MemoryAlloc is Not Valid");
        return false;
    }
    pMemPool->m_MemoryEnd = pMemPool->m_Memory;
    return true; 
}

//创建下一个内存池
bool CreateNextMemPool(MemoryPool *pMemPool,int AllocSize)
{
    PMemoryStore memoryPool = pMemPool->GetPoolHead(pMemPool);
    if (0 == memoryPool)
    {
        pMemPool->InitMemPool(pMemPool,((AllocSize + pMemPool->m_nCompareMini >= MEMPOOLSIZE) ? (AllocSize + pMemPool->m_nCompareMini) : MEMPOOLSIZE));
        return true;
    }
    while (memoryPool && 0 != memoryPool->Next)
        memoryPool = memoryPool->Next;
    memoryPool->Next = (PMemoryStore)malloc(sizeof(struct MemoryStore));
    memoryPool->Next->Init = MemoryStoreInit ;  //by liro
	memoryPool->Next->Init(*(memoryPool->Next)); //by liro
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
        pMemPool->WriteLastError(pMemPool,"this MemoryAlloc is Not Valid");
        return false;
    }
    pMemPool->m_MemoryEnd = memoryPool->Next;
    pMemPool->m_MemoryEnd->Priv = memoryPool;
    return true; 
}

//内存分配
void * MemAlloc(MemoryPool *pMemPool,int nSize)
{
    //增加头节点
    if (0 == pMemPool->GetMemListHead(pMemPool))
    {

        if (!pMemPool->InitMemPool(pMemPool,pMemPool->m_nInitPoolSize))
        {
            pMemPool->WriteLastError(pMemPool,"this Init is Not Valid");
            return 0;
        }
        return pMemPool->CreateListHead(pMemPool,nSize);
		
    }
    else
    {
        return pMemPool->GetPoolMem(pMemPool,nSize);
    }
}
//创建第一个节点
void *CreateListHead(MemoryPool *pMemPool,int nSize)
{
    return pMemPool->CreateFirstMem(pMemPool,nSize,1);
}
//获取节点头
PMemList GetHeadList(MemoryPool *pMemPool)
{
    return pMemPool->m_MemHead;
}
//获取首个内存池信息
PMemoryStore GetPoolHead(MemoryPool *pMemPool)
{
    return pMemPool->m_Memory;
}
//获取最后一个内存池信息
PMemoryStore GetPoolEnd(MemoryPool *pMemPool)
{
    return pMemPool->m_MemoryEnd;
}
//从所有内存池中取出未使用的内存地址
void * GetPoolMem(MemoryPool *pMemPool,int nSize)
{
    PMemoryStore pool = pMemPool->GetPoolHead(pMemPool);

    while (pool)
    {
        char *pData = (char *)pMemPool->GetAPoolofMem(pMemPool,pool,nSize);
        if (0 != pData)
            return (void *)pData;
        pool = pool->Next;
    }   
    //如果所有的池子都遍历了还是没有合适内存，那么创建一个新的池子
    if ((nSize + pMemPool->m_nCompareMini)  > MEMPOOLSIZE)
        pMemPool->CreateNextMemPool(pMemPool,nSize + pMemPool->m_nCompareMini);
    else
        pMemPool->CreateNextMemPool(pMemPool,MEMPOOLSIZE);

        char *pData = (char *)((pMemPool->GetAPoolofMem)(pMemPool,pMemPool->m_MemoryEnd,nSize));
        return (void *)pData;
}

//从一块内存池中获取数据
void *GetAPoolofMem(MemoryPool *pMemPool,PMemoryStore obj,int nSize)
{
    if (0 >= obj->SurplusVolumeDose || nSize >= obj->SurplusVolumeDose)
    {
        return 0;
    }
        //如果达到查询的条件 开始遍历对应编号的内存池 ，为了最大利用内存选择从头开始遍历，如果没有数据插入到最后
        PMemList listData = GetMemListHead(pMemPool);
        while (listData && (0 != listData->Next))
        {
            //判断是否有解除占用的节点，如果有的话记录第一个，如果没有找到合适的内存链 ，那么用这个进行分配内存
            if ((listData->nPoolCount == 0) && (0 < listData->Count))
            {
                pMemPool->SetAllocList(pMemPool,listData->Count);
            }

            //如果节点中保存的内存使用量大于或等于需要分配的内存，那么使用这块内存
            if (((nSize + pMemPool->m_nCompareMini  <= listData->CurrentUsgeAmount) && (nSize + pMemPool->m_nCompareMax >= listData->CurrentUsgeAmount)) && (0 == listData->bValid))
            {
                pMemPool->RevampMemUsgeAmount(pMemPool,listData->nPoolCount,listData->CurrentUsgeAmount);
                listData->bValid = 1;
                return (void *)listData->StartAddress;
            }
            listData = listData->Next;
        }
        int nCount = pMemPool->GetAllocList(pMemPool);
        if (0 < nCount)
            return pMemPool->AllocMemList(pMemPool,nCount,nSize,1);
    //创建新的节点保存分配内存
    return pMemPool->CreateNextMem(pMemPool,obj,nSize,1);
}

//标记第一个被解除占用的内存节点
bool SetAllocList(MemoryPool *pMemPool,int Count)
{
    if (0 >= Count || 0 < pMemPool->m_nCount)
        return false;
    pMemPool->m_nCount = Count;
    return true;
}

//获取第一个被解除占用的内存节点,当前保存的节点被使用后可以再次储存下一个被解除占用的内存节点
int GetAllocList(MemoryPool *pMemPool)
{
    int Count = pMemPool->m_nCount;
    pMemPool->m_nCount = 0;
    return Count;
}

//修改指定内存块当前使用量
bool RevampMemUsgeAmount(MemoryPool *pMemPool,int nCount,long long nSize)
{
    if (0 >= nCount)
        return false;
    PMemoryStore memPool = pMemPool->GetPoolHead(pMemPool);
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
void * CreateFirstMem(MemoryPool *pMemPool,int nSize,bool bValid)
{
    //如果头节点已经创建 调用默认内存分配
    if (0 != pMemPool->m_MemHead)
        return pMemPool->GetPoolMem(pMemPool,nSize);

    PMemoryStore pool = pMemPool->GetPoolHead(pMemPool);

    pMemPool->m_MemHead = pMemPool->GetMemList(pMemPool);
    pMemPool->m_MemHead->Count = 1;
    pMemPool->m_MemHead->StartAddress = (pool->StartAddress + pool->CurrentUsageAmount);
    //多分配一个字节用来防止内存越界
    pMemPool->m_MemHead->EndAddress = (pMemPool->m_MemHead->StartAddress + nSize + 1);
    pMemPool->m_MemHead->CurrentUsgeAmount = (nSize + 1);
    pMemPool->m_MemHead->nPoolCount = pool->Count;
    pMemPool->m_MemHead->bValid = bValid;
    pool->CurrentUsageAmount += (nSize + 1);
    pool->SurplusVolumeDose -= pool->CurrentUsageAmount;
    pMemPool->m_MemEnd = pMemPool->m_MemHead;
    //分配出一段干净的内存 上层方便使用
    memset((void *)pMemPool->m_MemHead->StartAddress,0,nSize + 1);
    return (void *)pMemPool->m_MemHead->StartAddress;
}

//创建新的节点以及分配内存
void *CreateNextMem(MemoryPool *pMemPool,PMemoryStore obj,int nSize,bool bValid)
{
    PMemList list = pMemPool->GetEndList(pMemPool);
    list->Next = pMemPool->GetMemList(pMemPool);
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
    pMemPool->m_MemEnd = list->Next;
    //分配出一段干净的内存 上层方便使用
    memset((void *)list->Next->StartAddress,0,nSize + 1);
    return (void *)list->Next->StartAddress;
}

//获取可以分出内存的池子，这个直接从空闲内存中取出需要的内存
PMemoryStore GetValidMemPool(MemoryPool *pMemPool,int nSize)
{
    PMemoryStore pool = pMemPool->GetPoolHead(pMemPool);
    while (pool)
    {
        if (pool->SurplusVolumeDose >= (nSize + pMemPool->m_nCompareMini))
        {
            return pool;
        }
        pool = pool->Next;
    }
    //如果没有 就创建一个新的内存池
    if (pMemPool->CreateNextMemPool(pMemPool,((nSize + pMemPool->m_nCompareMini) >= MEMPOOLSIZE ? (nSize + pMemPool->m_nCompareMini) : MEMPOOLSIZE)))
        return pMemPool->GetPoolEnd(pMemPool);

    return 0;
}

//使用被解除占用的节点分配内存
void *AllocMemList(MemoryPool *pMemPool,int nCount,int nSize,bool bValid)
{
    PMemList list = pMemPool->GetCountList(pMemPool,nCount);
    if (0 == list)
        return 0;
    PMemoryStore memPool = pMemPool->GetValidMemPool(pMemPool,nSize);
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
PMemList GetCountList(MemoryPool *pMemPool,int Count)
{
    if (0 < Count)
    {
        PMemList list = pMemPool->GetHeadList(pMemPool);
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
PMemList GetEndList(MemoryPool *pMemPool)
{
    return pMemPool->m_MemEnd;
}

//获取链表内存结构头节点
PMemList GetMemListHead(MemoryPool *pMemPool)
{
    return pMemPool->m_MemHead;
}

//创建链表内存结构头
bool InitMemHead(MemoryPool *pMemPool)
{
    pMemPool->m_MemHead = pMemPool->GetMemList(pMemPool);
    pMemPool->m_MemCurrent = pMemPool->m_MemEnd = pMemPool->m_MemHead;
    return (pMemPool->m_MemHead != 0 ? true : false);
}

//创建链接结构节点
PMemList GetMemList(MemoryPool *pMemPool)
{
    PMemList list = (PMemList)malloc(sizeof(struct MemList));
    list->Init = MemListInit;
	list->Init(*list);
    return list;
}

//内存释放
bool MemFree(MemoryPool *pMemPool,void * ptr)
{
    //根据分配的地址在内存池中匹配，匹配到后 修改结构数据后，等待再次使用
    PMemList list = pMemPool->GetMemListHead(pMemPool);
    while (list)
    {
        //如果链表中其中一节数据与需要释放的地址相同 ，而且这段数据属于使用中，属于这块内存
        if ((list->StartAddress == (unsigned long)ptr) && (1 == list->bValid))
        {
            pMemPool->RevampMemUsgeAmount(pMemPool,list->nPoolCount,~(long long)list->CurrentUsgeAmount + 1);
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
bool MemPoolFree(MemoryPool *pMemPool)
{
    PMemoryStore memPool = pMemPool->GetPoolHead(pMemPool);
    while (0 != memPool)
    {
        PMemoryStore next =  memPool->Next;
        pMemPool->_MemPoolFree(pMemPool,memPool->Count);
        memPool = next;
    }
    return true;
}

//释放指定编号内存池
bool _MemPoolFree(MemoryPool *pMemPool,int nCount)
{
    PMemoryStore memPool = pMemPool->GetPoolHead(pMemPool);
    while (memPool->Count != nCount)
        memPool = memPool->Next;

    if (0 == memPool)
        return false;

    PMemoryStore priv = 0,next = 0;
    if (0 != memPool->Priv)
        priv = memPool->Priv;
    if (0 != memPool->Next)
        next = memPool->Next;

    pMemPool->MemListFree(pMemPool,memPool->Count);
    free(memPool);
    memPool = 0;
    if (0 != priv)
        priv->Next = next;
    else
        pMemPool->m_Memory = next;
    if (0 != next)
        next->Priv = priv;
    else
        pMemPool->m_MemoryEnd = pMemPool->m_Memory;
    return true;
}

//解除占用标记的某块内存 ，释放类对象前统一释放
bool MemListFree(MemoryPool *pMemPool,int nPoolCount)
{
    PMemList list = pMemPool->GetHeadList(pMemPool);
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
PMemList FreeList(MemoryPool *pMemPool,int Count)
{
    PMemList list = pMemPool->GetCountList(pMemPool,Count);
    if (0 == list)
        return 0;

    PMemList priv = 0,next = 0;
    if (0 != list->Priv)
        priv = list->Priv;
    if (0 != list->Next)
        next = list->Next;

    free(list);
    if (0 != priv)
        priv->Next = next;
    else
        pMemPool->m_MemHead = next;
    if (0 != next)
        next->Priv = priv;
    else
        pMemPool->m_MemEnd = pMemPool->m_MemHead;
    return next;
}
//释放链表的所有节点
bool FreeAllList(MemoryPool *pMemPool)
{
    PMemList list = pMemPool->GetHeadList(pMemPool);
    while (list)
    {
        list = pMemPool->FreeList(pMemPool,list->Count);
    }
    return true;
}


void MemoryPoolInit(MemoryPool *pMemPool)
{
		pMemPool -> MemAlloc			= MemAlloc;
		pMemPool -> MemFree 	 		= MemFree;
		pMemPool -> MemPoolFree  		= MemPoolFree;
		pMemPool -> GetLastError 		= GetLastError;
		pMemPool -> SetComPareMemMini 	= SetComPareMemMini;
		pMemPool -> SetComPareMemMax    = SetComPareMemMax;
		pMemPool -> WriteLastError      = WriteLastError;
		pMemPool -> InitMemPool         = InitMemPool;
		pMemPool -> CreateNextMemPool   = CreateNextMemPool;
		pMemPool -> GetMemList			= GetMemList;
		pMemPool -> InitMemHead			= InitMemHead;
		pMemPool -> GetMemListHead		= GetMemListHead;
		pMemPool -> GetPoolMem			= GetPoolMem;
		pMemPool -> GetPoolHead			= GetPoolHead;
		pMemPool -> GetPoolEnd			= GetPoolEnd;
		pMemPool -> GetAPoolofMem		= GetAPoolofMem;
		pMemPool -> GetEndList			= GetEndList;
		pMemPool -> CreateFirstMem		= CreateFirstMem;
		pMemPool -> CreateNextMem		= CreateNextMem;
		pMemPool -> CreateListHead		= CreateListHead;
		pMemPool ->	GetHeadList			= GetHeadList;
		pMemPool -> _MemPoolFree		= _MemPoolFree;
		pMemPool -> RevampMemUsgeAmount	= RevampMemUsgeAmount;
		pMemPool -> MemListFree			= MemListFree;
		pMemPool -> SetAllocList		= SetAllocList;
		pMemPool -> GetAllocList		= GetAllocList;
		pMemPool -> AllocMemList		= AllocMemList;
		pMemPool -> GetCountList		= GetCountList;
		pMemPool -> GetValidMemPool		= GetValidMemPool;
		pMemPool -> FreeList			= FreeList;
		pMemPool -> FreeAllList			= FreeAllList;	
		
}
void MemoryStoreInit(struct MemoryStore Instance)
{
	Instance.Count 				= 0;
	Instance.MemVolumeDose 		= 0;
	Instance.StartAddress  		= 0;
	Instance.EndAddress	   		= 0;
	Instance.CurrentUsageAmount	= 0;
	Instance.SurplusVolumeDose	= 0;
	Instance.Priv				= 0;
	Instance.Next				= 0;
	
}

void MemListInit(struct MemList Instance)
{
	Instance.Count 				= 0;
	Instance.nPoolCount 		= 0;
	Instance.StartAddress  		= 0;
	Instance.EndAddress	   		= 0;
	Instance.CurrentUsgeAmount	= 0;
	Instance.bValid	= 0;
	Instance.Priv				= 0;
	Instance.Next				= 0;
	
}





