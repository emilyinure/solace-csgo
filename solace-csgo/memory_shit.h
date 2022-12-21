#pragma once
#include <Windows.h>
#define UTLMEMORY_TRACK_ALLOC() ((void)0)
#define UTLMEMORY_TRACK_FREE() ((void)0)
#define MEM_ALLOC_CREDIT_CLASS()
#define Assert(num)

template <class T> inline T* CopyConstruct(T* pMemory, T const& src)
{
    return reinterpret_cast<T*>(::new (pMemory) T(src));
}

template <class T> inline void Destruct(T* pMemory)
{
    pMemory->~T();

#ifdef _DEBUG
    memset(reinterpret_cast<void*>(pMemory), 0xDD, sizeof(T));
#endif
}

// The above will error when binding to a type of: foo(*)[] -- there is no provision in c++ for knowing how many objects
// to destruct without preserving the count and calling the necessary destructors.
template <class T, size_t N> inline void Destruct(T (*pMemory)[N])
{
    for (size_t i = 0; i < N; i++)
    {
        (pMemory[i])->~T();
    }

#ifdef _DEBUG
    memset(reinterpret_cast<void*>(pMemory), 0xDD, sizeof(*pMemory));
#endif
}

//-----------------------------------------------------------------------------
// The CUtlMemory class:
// A growable memory class which doubles in size by default.
//-----------------------------------------------------------------------------
template <class T, class I = int> class CUtlMemory
{
public:
    // constructor, destructor
    CUtlMemory(int nGrowSize = 0, int nInitSize = 0);
    CUtlMemory(T* pMemory, int numElements);
    CUtlMemory(const T* pMemory, int numElements);
    ~CUtlMemory();

    // Set the size by which the memory grows
    void Init(int nGrowSize = 0, int nInitSize = 0);

    class Iterator_t
    {
    public:
        Iterator_t(I i) : index(i)
        {
        }
        I index;

        bool operator==(const Iterator_t it) const
        {
            return index == it.index;
        }
        bool operator!=(const Iterator_t it) const
        {
            return index != it.index;
        }
    };
    Iterator_t First() const
    {
        return Iterator_t(IsIdxValid(0) ? 0 : InvalidIndex());
    }
    Iterator_t Next(const Iterator_t& it) const
    {
        return Iterator_t(IsIdxValid(it.index + 1) ? it.index + 1 : InvalidIndex());
    }
    I GetIndex(const Iterator_t& it) const
    {
        return it.index;
    }
    bool IsIdxAfter(I i, const Iterator_t& it) const
    {
        return i > it.index;
    }
    bool IsValidIterator(const Iterator_t& it) const
    {
        return IsIdxValid(it.index);
    }
    Iterator_t InvalidIterator() const
    {
        return Iterator_t(InvalidIndex());
    }

    // element access
    T& operator[](I i);
    const T& operator[](I i) const;
    T& Element(I i);
    const T& Element(I i) const;

    // Can we use this index?
    bool IsIdxValid(I i) const;

    // Specify the invalid ('null') index that we'll only return on failure
    static const I INVALID_INDEX = (I)-1; // For use with COMPILE_TIME_ASSERT
    static I InvalidIndex()
    {
        return INVALID_INDEX;
    }

    // Gets the base address (can change when adding elements!)
    T* Base();
    const T* Base() const;

    // Attaches the buffer to external memory....
    void SetExternalBuffer(T* pMemory, int numElements);
    void SetExternalBuffer(const T* pMemory, int numElements);
    void AssumeMemory(T* pMemory, int nSize);
    T* Detach();
    void* DetachMemory();

    // Fast swap
    void Swap(CUtlMemory<T, I>& mem);

    // Switches the buffer from an external memory buffer to a reallocatable buffer
    // Will copy the current contents of the external buffer to the reallocatable buffer
    void ConvertToGrowableMemory(int nGrowSize);

    // Size
    int NumAllocated() const;
    int Count() const;

    // Grows the memory, so that at least allocated + num elements are allocated
    void Grow(int num = 1);

    // Makes sure we've got at least this much memory
    void EnsureCapacity(int num);

    // Memory deallocation
    void Purge();

    // Purge all but the given number of elements
    void Purge(int numElements);

    // is the memory externally allocated?
    bool IsExternallyAllocated() const;

    // is the memory read only?
    bool IsReadOnly() const;

    // Set the size by which the memory grows
    void SetGrowSize(int size);

protected:
    void ValidateGrowSize()
    {
#ifdef _X360
        if (m_nGrowSize && m_nGrowSize != EXTERNAL_BUFFER_MARKER)
        {
            // Max grow size at 128 bytes on XBOX
            const int MAX_GROW = 128;
            if (m_nGrowSize * sizeof(T) > MAX_GROW)
            {
                m_nGrowSize = max(1, MAX_GROW / sizeof(T));
            }
        }
#endif
    }

    enum
    {
        EXTERNAL_BUFFER_MARKER = -1,
        EXTERNAL_CONST_BUFFER_MARKER = -2,
    };

    T* m_pMemory;
    int m_nAllocationCount;
    int m_nGrowSize;
};

//-----------------------------------------------------------------------------
// constructor, destructor
//-----------------------------------------------------------------------------

template <class T, class I>
CUtlMemory<T, I>::CUtlMemory(int nGrowSize, int nInitAllocationCount)
    : m_pMemory(0), m_nAllocationCount(nInitAllocationCount), m_nGrowSize(nGrowSize)
{
    ValidateGrowSize();
    Assert(nGrowSize >= 0);
    if (m_nAllocationCount)
    {
        UTLMEMORY_TRACK_ALLOC();
        MEM_ALLOC_CREDIT_CLASS();
        m_pMemory = (T*)malloc(m_nAllocationCount * sizeof(T));
    }
}

template <class T, class I>
CUtlMemory<T, I>::CUtlMemory(T* pMemory, int numElements) : m_pMemory(pMemory), m_nAllocationCount(numElements)
{
    // Special marker indicating externally supplied modifyable memory
    m_nGrowSize = EXTERNAL_BUFFER_MARKER;
}

template <class T, class I>
CUtlMemory<T, I>::CUtlMemory(const T* pMemory, int numElements)
    : m_pMemory((T*)pMemory), m_nAllocationCount(numElements)
{
    // Special marker indicating externally supplied modifyable memory
    m_nGrowSize = EXTERNAL_CONST_BUFFER_MARKER;
}

template <class T, class I> CUtlMemory<T, I>::~CUtlMemory()
{
    Purge();

#ifdef _DEBUG
    m_pMemory = reinterpret_cast<T*>(0xFEFEBAAD);
    m_nAllocationCount = 0x7BADF00D;
#endif
}

template <class T, class I> void CUtlMemory<T, I>::Init(int nGrowSize /*= 0*/, int nInitSize /*= 0*/)
{
    Purge();

    m_nGrowSize = nGrowSize;
    m_nAllocationCount = nInitSize;
    ValidateGrowSize();
    Assert(nGrowSize >= 0);
    if (m_nAllocationCount)
    {
        UTLMEMORY_TRACK_ALLOC();
        MEM_ALLOC_CREDIT_CLASS();
        m_pMemory = (T*)malloc(m_nAllocationCount * sizeof(T));
    }
}

//-----------------------------------------------------------------------------
// Fast swap
//-----------------------------------------------------------------------------
template <class T, class I> void CUtlMemory<T, I>::Swap(CUtlMemory<T, I>& mem)
{
    V_swap(m_nGrowSize, mem.m_nGrowSize);
    V_swap(m_pMemory, mem.m_pMemory);
    V_swap(m_nAllocationCount, mem.m_nAllocationCount);
}

//-----------------------------------------------------------------------------
// Switches the buffer from an external memory buffer to a reallocatable buffer
//-----------------------------------------------------------------------------
template <class T, class I> void CUtlMemory<T, I>::ConvertToGrowableMemory(int nGrowSize)
{
    if (!IsExternallyAllocated())
        return;

    m_nGrowSize = nGrowSize;
    if (m_nAllocationCount)
    {
        UTLMEMORY_TRACK_ALLOC();
        MEM_ALLOC_CREDIT_CLASS();

        int nNumBytes = m_nAllocationCount * sizeof(T);
        T* pMemory = (T*)malloc(nNumBytes);
        memcpy(pMemory, m_pMemory, nNumBytes);
        m_pMemory = pMemory;
    }
    else
    {
        m_pMemory = nullptr;
    }
}

//-----------------------------------------------------------------------------
// Attaches the buffer to external memory....
//-----------------------------------------------------------------------------
template <class T, class I> void CUtlMemory<T, I>::SetExternalBuffer(T* pMemory, int numElements)
{
    // Blow away any existing allocated memory
    Purge();

    m_pMemory = pMemory;
    m_nAllocationCount = numElements;

    // Indicate that we don't own the memory
    m_nGrowSize = EXTERNAL_BUFFER_MARKER;
}

template <class T, class I> void CUtlMemory<T, I>::SetExternalBuffer(const T* pMemory, int numElements)
{
    // Blow away any existing allocated memory
    Purge();

    m_pMemory = const_cast<T*>(pMemory);
    m_nAllocationCount = numElements;

    // Indicate that we don't own the memory
    m_nGrowSize = EXTERNAL_CONST_BUFFER_MARKER;
}

template <class T, class I> void CUtlMemory<T, I>::AssumeMemory(T* pMemory, int numElements)
{
    // Blow away any existing allocated memory
    Purge();

    // Simply take the pointer but don't mark us as external
    m_pMemory = pMemory;
    m_nAllocationCount = numElements;
}

template <class T, class I> void* CUtlMemory<T, I>::DetachMemory()
{
    if (IsExternallyAllocated())
        return nullptr;

    void* pMemory = m_pMemory;
    m_pMemory = 0;
    m_nAllocationCount = 0;
    return pMemory;
}

template <class T, class I> inline T* CUtlMemory<T, I>::Detach()
{
    return (T*)DetachMemory();
}

//-----------------------------------------------------------------------------
// element access
//-----------------------------------------------------------------------------
template <class T, class I> inline T& CUtlMemory<T, I>::operator[](I i)
{
    // Assert(!IsReadOnly());
    // Assert(IsIdxValid(i));
    return m_pMemory[i];
}

template <class T, class I> inline const T& CUtlMemory<T, I>::operator[](I i) const
{
    // Assert(IsIdxValid(i));
    return m_pMemory[i];
}

template <class T, class I> inline T& CUtlMemory<T, I>::Element(I i)
{
    // Assert(!IsReadOnly());
    // Assert(IsIdxValid(i));
    return m_pMemory[i];
}

template <class T, class I> inline const T& CUtlMemory<T, I>::Element(I i) const
{
    Assert(IsIdxValid(i));
    return m_pMemory[i];
}

//-----------------------------------------------------------------------------
// is the memory externally allocated?
//-----------------------------------------------------------------------------
template <class T, class I> bool CUtlMemory<T, I>::IsExternallyAllocated() const
{
    return (m_nGrowSize < 0);
}

//-----------------------------------------------------------------------------
// is the memory read only?
//-----------------------------------------------------------------------------
template <class T, class I> bool CUtlMemory<T, I>::IsReadOnly() const
{
    return (m_nGrowSize == EXTERNAL_CONST_BUFFER_MARKER);
}

template <class T, class I> void CUtlMemory<T, I>::SetGrowSize(int nSize)
{
    // Assert(!IsExternallyAllocated());
    // Assert(nSize >= 0);
    m_nGrowSize = nSize;
    ValidateGrowSize();
}

//-----------------------------------------------------------------------------
// Gets the base address (can change when adding elements!)
//-----------------------------------------------------------------------------
template <class T, class I> inline T* CUtlMemory<T, I>::Base()
{
    // Assert(!IsReadOnly());
    return m_pMemory;
}

template <class T, class I> inline const T* CUtlMemory<T, I>::Base() const
{
    return m_pMemory;
}

//-----------------------------------------------------------------------------
// Size
//-----------------------------------------------------------------------------
template <class T, class I> inline int CUtlMemory<T, I>::NumAllocated() const
{
    return m_nAllocationCount;
}

template <class T, class I> inline int CUtlMemory<T, I>::Count() const
{
    return m_nAllocationCount;
}

//-----------------------------------------------------------------------------
// Is element index valid?
//-----------------------------------------------------------------------------
template <class T, class I> inline bool CUtlMemory<T, I>::IsIdxValid(I i) const
{
    // GCC warns if I is an unsigned type and we do a ">= 0" against it (since the comparison is always 0).
    // We get the warning even if we cast inside the expression. It only goes away if we assign to another variable.
    long x = i;
    return (x >= 0) && (x < m_nAllocationCount);
}

//-----------------------------------------------------------------------------
// Grows the memory
//-----------------------------------------------------------------------------
inline int UtlMemory_CalcNewAllocationCount(int nAllocationCount, int nGrowSize, int nNewSize, int nBytesItem)
{
    if (nGrowSize)
    {
        nAllocationCount = ((1 + ((nNewSize - 1) / nGrowSize)) * nGrowSize);
    }
    else
    {
        if (!nAllocationCount)
        {
            // Compute an allocation which is at least as big as a cache line...
            nAllocationCount = (31 + nBytesItem) / nBytesItem;
        }

        while (nAllocationCount < nNewSize)
        {
#ifndef _X360
            nAllocationCount *= 2;
#else
            int nNewAllocationCount = (nAllocationCount * 9) / 8; // 12.5 %
            if (nNewAllocationCount > nAllocationCount)
                nAllocationCount = nNewAllocationCount;
            else
                nAllocationCount *= 2;
#endif
        }
    }

    return nAllocationCount;
}

template <class T, class I> void CUtlMemory<T, I>::Grow(int num)
{
    // Assert(num > 0);

    if (IsExternallyAllocated())
    {
        // Can't grow a buffer whose memory was externally allocated
        // Assert(0);
        return;
    }

    // Make sure we have at least numallocated + num allocations.
    // Use the grow rules specified for this memory (in m_nGrowSize)
    int nAllocationRequested = m_nAllocationCount + num;

    // UTLMEMORY_TRACK_FREE();

    int nNewAllocationCount =
        UtlMemory_CalcNewAllocationCount(m_nAllocationCount, m_nGrowSize, nAllocationRequested, sizeof(T));

    // if m_nAllocationRequested wraps index type I, recalculate
    if ((int)(I)nNewAllocationCount < nAllocationRequested)
    {
        if ((int)(I)nNewAllocationCount == 0 && (int)(I)(nNewAllocationCount - 1) >= nAllocationRequested)
        {
            --nNewAllocationCount; // deal w/ the common case of m_nAllocationCount == MAX_USHORT + 1
        }
        else
        {
            if ((int)(I)nAllocationRequested != nAllocationRequested)
            {
                // we've been asked to grow memory to a size s.t. the index type can't address the requested amount of
                // memory
                // Assert(0);
                return;
            }
            while ((int)(I)nNewAllocationCount < nAllocationRequested)
            {
                nNewAllocationCount = (nNewAllocationCount + nAllocationRequested) / 2;
            }
        }
    }

    m_nAllocationCount = nNewAllocationCount;

    // UTLMEMORY_TRACK_ALLOC();

    if (m_pMemory)
    {
        // MEM_ALLOC_CREDIT_CLASS();
        m_pMemory = (T*)realloc(m_pMemory, m_nAllocationCount * sizeof(T));
        // Assert(m_pMemory);
    }
    else
    {
        // MEM_ALLOC_CREDIT_CLASS();
        m_pMemory = (T*)malloc(m_nAllocationCount * sizeof(T));
        // Assert(m_pMemory);
    }
}

//-----------------------------------------------------------------------------
// Makes sure we've got at least this much memory
//-----------------------------------------------------------------------------
template <class T, class I> inline void CUtlMemory<T, I>::EnsureCapacity(int num)
{
    if (m_nAllocationCount >= num)
        return;

    if (IsExternallyAllocated())
    {
        // Can't grow a buffer whose memory was externally allocated
        Assert(0);
        return;
    }

    UTLMEMORY_TRACK_FREE();

    m_nAllocationCount = num;

    UTLMEMORY_TRACK_ALLOC();

    if (m_pMemory)
    {
        MEM_ALLOC_CREDIT_CLASS();
        m_pMemory = (T*)realloc(m_pMemory, m_nAllocationCount * sizeof(T));
    }
    else
    {
        MEM_ALLOC_CREDIT_CLASS();
        m_pMemory = (T*)malloc(m_nAllocationCount * sizeof(T));
    }
}

//-----------------------------------------------------------------------------
// Memory deallocation
//-----------------------------------------------------------------------------
template <class T, class I> void CUtlMemory<T, I>::Purge()
{
    if (!IsExternallyAllocated())
    {
        if (m_pMemory)
        {
            UTLMEMORY_TRACK_FREE();
            free((void*)m_pMemory);
            m_pMemory = 0;
        }
        m_nAllocationCount = 0;
    }
}

template <class T, class I> void CUtlMemory<T, I>::Purge(int numElements)
{
    Assert(numElements >= 0);

    if (numElements > m_nAllocationCount)
    {
        // Ensure this isn't a grow request in disguise.
        Assert(numElements <= m_nAllocationCount);
        return;
    }

    // If we have zero elements, simply do a purge:
    if (numElements == 0)
    {
        Purge();
        return;
    }

    if (IsExternallyAllocated())
    {
        // Can't shrink a buffer whose memory was externally allocated, fail silently like purge
        return;
    }

    // If the number of elements is the same as the allocation count, we are done.
    if (numElements == m_nAllocationCount)
    {
        return;
    }

    if (!m_pMemory)
    {
        // Allocation count is non zero, but memory is null.
        Assert(m_pMemory);
        return;
    }

    UTLMEMORY_TRACK_FREE();

    m_nAllocationCount = numElements;

    UTLMEMORY_TRACK_ALLOC();

    // Allocation count > 0, shrink it down.
    MEM_ALLOC_CREDIT_CLASS();
    m_pMemory = (T*)realloc(m_pMemory, m_nAllocationCount * sizeof(T));
}

template <class T, class A = CUtlMemory<T>> class CUtlVector
{
public:
    typedef A CAllocator;
    typedef T ElemType_t;

    // constructor, destructor
    CUtlVector(int growSize = 0, int initSize = 0);
    CUtlVector(T* pMemory, int allocationCount, int numElements = 0);
    ~CUtlVector();

    // Copy the array.
    CUtlVector<T, A>& operator=(const CUtlVector<T, A>& other);

    // element access
    T& operator[](int i);
    const T& operator[](int i) const;
    T& Element(int i);
    const T& Element(int i) const;
    T& Head();
    const T& Head() const;
    T& Tail();
    const T& Tail() const;

    // Gets the base address (can change when adding elements!)
    T* Base()
    {
        return m_Memory.Base();
    }
    const T* Base() const
    {
        return m_Memory.Base();
    }

    // Returns the number of elements in the vector
    int Count() const;

    // Is element index valid?
    bool IsValidIndex(int i) const;
    static int InvalidIndex();

    // Adds an element, uses default constructor
    int AddToHead();
    int AddToTail();
    int InsertBefore(int elem);
    int InsertAfter(int elem);

    // Adds an element, uses copy constructor
    int AddToHead(const T& src);
    int AddToTail(const T& src);
    int InsertBefore(int elem, const T& src);
    int InsertAfter(int elem, const T& src);

    // Adds multiple elements, uses default constructor
    int AddMultipleToHead(int num);
    int AddMultipleToTail(int num);
    int AddMultipleToTail(int num, const T* pToCopy);
    int InsertMultipleBefore(int elem, int num);
    int InsertMultipleBefore(int elem, int num, const T* pToCopy);
    int InsertMultipleAfter(int elem, int num);

    // Calls RemoveAll() then AddMultipleToTail.
    void SetSize(int size);
    void SetCount(int count);
    void SetCountNonDestructively(int count); // sets count by adding or removing elements to tail TODO: This should
                                              // probably be the default behavior for SetCount

    // Calls SetSize and copies each element.
    void CopyArray(const T* pArray, int size);

    // Fast swap
    void Swap(CUtlVector<T, A>& vec);

    // Add the specified array to the tail.
    int AddVectorToTail(CUtlVector<T, A> const& src);

    // Finds an element (element needs operator== defined)
    int Find(const T& src) const;
    void FillWithValue(const T& src);

    bool HasElement(const T& src) const;

    // Makes sure we have enough memory allocated to store a requested # of elements
    void EnsureCapacity(int num);

    // Makes sure we have at least this many elements
    void EnsureCount(int num);

    // Element removal
    void FastRemove(int elem);              // doesn't preserve order
    void Remove(int elem);                  // preserves order, shifts elements
    bool FindAndRemove(const T& src);       // removes first occurrence of src, preserves order, shifts elements
    bool FindAndFastRemove(const T& src);   // removes first occurrence of src, doesn't preserve order
    void RemoveMultiple(int elem, int num); // preserves order, shifts elements
    void RemoveMultipleFromHead(int num);   // removes num elements from tail
    void RemoveMultipleFromTail(int num);   // removes num elements from tail
    void RemoveAll();                       // doesn't deallocate memory

    // Memory deallocation
    void Purge();

    // Purges the list and calls delete on each element in it.
    void PurgeAndDeleteElements();

    // Compacts the vector to the number of elements actually in use
    void Compact();

    // Set the size by which it grows when it needs to allocate more memory.
    void SetGrowSize(int size)
    {
        m_Memory.SetGrowSize(size);
    }

    int NumAllocated() const; // Only use this if you really know what you're doing!

    void Sort(int(__cdecl* pfnCompare)(const T*, const T*));

    // Call this to quickly sort non-contiguously allocated vectors
    void InPlaceQuickSort(int(__cdecl* pfnCompare)(const T*, const T*));

#ifdef DBGFLAG_VALIDATE
    void Validate(CValidator& validator, char* pchName); // Validate our internal structures
#endif                                                   // DBGFLAG_VALIDATE

    // Can't copy this unless we explicitly do it!
    CUtlVector(CUtlVector const& vec)
    {
        Assert(0);
    }

    // Grows the vector
    void GrowVector(int num = 1);

    // Shifts elements....
    void ShiftElementsRight(int elem, int num = 1);
    void ShiftElementsLeft(int elem, int num = 1);

    CAllocator m_Memory;
    int m_Size;

#ifndef _X360
    // For easier access to the elements through the debugger
    // it's in release builds so this can be used in libraries correctly
    T* m_pElements;

    inline void ResetDbgInfo()
    {
        m_pElements = Base();
    }
#else
    inline void ResetDbgInfo()
    {
    }
#endif

    void InPlaceQuickSort_r(int(__cdecl* pfnCompare)(const T*, const T*), int nLeft, int nRight);
};

//-----------------------------------------------------------------------------
// constructor, destructor
//-----------------------------------------------------------------------------
template <typename T, class A>
inline CUtlVector<T, A>::CUtlVector(int growSize, int initSize) : m_Memory(growSize, initSize), m_Size(0)
{
    ResetDbgInfo();
}

template <typename T, class A>
inline CUtlVector<T, A>::CUtlVector(T* pMemory, int allocationCount, int numElements)
    : m_Memory(pMemory, allocationCount), m_Size(numElements)
{
    ResetDbgInfo();
}

template <typename T, class A> inline CUtlVector<T, A>::~CUtlVector()
{
    Purge();
}

template <typename T, class A> inline CUtlVector<T, A>& CUtlVector<T, A>::operator=(const CUtlVector<T, A>& other)
{
    int nCount = other.Count();
    SetSize(nCount);
    for (int i = 0; i < nCount; i++)
    {
        (*this)[i] = other[i];
    }
    return *this;
}

//-----------------------------------------------------------------------------
// element access
//-----------------------------------------------------------------------------
template <typename T, class A> inline T& CUtlVector<T, A>::operator[](int i)
{
    // Assert(i < m_Size);
    return m_Memory[i];
}

template <typename T, class A> inline const T& CUtlVector<T, A>::operator[](int i) const
{
    // Assert(i < m_Size);
    return m_Memory[i];
}

template <typename T, class A> inline T& CUtlVector<T, A>::Element(int i)
{
    // Assert(i < m_Size);
    return m_Memory[i];
}

template <typename T, class A> inline const T& CUtlVector<T, A>::Element(int i) const
{
    // Assert(i < m_Size);
    return m_Memory[i];
}

template <typename T, class A> inline T& CUtlVector<T, A>::Head()
{
    // Assert(m_Size > 0);
    return m_Memory[0];
}

template <typename T, class A> inline const T& CUtlVector<T, A>::Head() const
{
    // Assert(m_Size > 0);
    return m_Memory[0];
}

template <typename T, class A> inline T& CUtlVector<T, A>::Tail()
{
    // Assert(m_Size > 0);
    return m_Memory[m_Size - 1];
}

template <typename T, class A> inline const T& CUtlVector<T, A>::Tail() const
{
    // Assert(m_Size > 0);
    return m_Memory[m_Size - 1];
}

//-----------------------------------------------------------------------------
// Count
//-----------------------------------------------------------------------------
template <typename T, class A> inline int CUtlVector<T, A>::Count() const
{
    return m_Size;
}

//-----------------------------------------------------------------------------
// Is element index valid?
//-----------------------------------------------------------------------------
template <typename T, class A> inline bool CUtlVector<T, A>::IsValidIndex(int i) const
{
    return (i >= 0) && (i < m_Size);
}

//-----------------------------------------------------------------------------
// Returns in invalid index
//-----------------------------------------------------------------------------
template <typename T, class A> inline int CUtlVector<T, A>::InvalidIndex()
{
    return -1;
}

//-----------------------------------------------------------------------------
// Grows the vector
//-----------------------------------------------------------------------------
template <typename T, class A> void CUtlVector<T, A>::GrowVector(int num)
{
    if (m_Size + num > m_Memory.NumAllocated())
    {
        // MEM_ALLOC_CREDIT_CLASS();
        m_Memory.Grow(m_Size + num - m_Memory.NumAllocated());
    }

    m_Size += num;
    ResetDbgInfo();
}

//-----------------------------------------------------------------------------
// Sorts the vector
//-----------------------------------------------------------------------------
template <typename T, class A> void CUtlVector<T, A>::Sort(int(__cdecl* pfnCompare)(const T*, const T*))
{
    typedef int(__cdecl * QSortCompareFunc_t)(const void*, const void*);
    if (Count() <= 1)
        return;

    if (Base())
    {
        qsort(Base(), Count(), sizeof(T), (QSortCompareFunc_t)(pfnCompare));
    }
    else
    {
        // Assert(0);
        //  this path is untested
        //  if you want to sort vectors that use a non-sequential memory allocator,
        //  you'll probably want to patch in a quicksort algorithm here
        //  I just threw in this bubble sort to have something just in case...

        for (int i = m_Size - 1; i >= 0; --i)
        {
            for (int j = 1; j <= i; ++j)
            {
                if (pfnCompare(&Element(j - 1), &Element(j)) < 0)
                {
                    V_swap(Element(j - 1), Element(j));
                }
            }
        }
    }
}

//----------------------------------------------------------------------------------------------
// Private function that does the in-place quicksort for non-contiguously allocated vectors.
//----------------------------------------------------------------------------------------------
template <typename T, class A>
void CUtlVector<T, A>::InPlaceQuickSort_r(int(__cdecl* pfnCompare)(const T*, const T*), int nLeft, int nRight)
{
    int nPivot;
    int nLeftIdx = nLeft;
    int nRightIdx = nRight;

    if (nRight - nLeft > 0)
    {
        nPivot = (nLeft + nRight) / 2;

        while ((nLeftIdx <= nPivot) && (nRightIdx >= nPivot))
        {
            while ((pfnCompare(&Element(nLeftIdx), &Element(nPivot)) < 0) && (nLeftIdx <= nPivot))
            {
                nLeftIdx++;
            }

            while ((pfnCompare(&Element(nRightIdx), &Element(nPivot)) > 0) && (nRightIdx >= nPivot))
            {
                nRightIdx--;
            }

            V_swap(Element(nLeftIdx), Element(nRightIdx));

            nLeftIdx++;
            nRightIdx--;

            if ((nLeftIdx - 1) == nPivot)
            {
                nPivot = nRightIdx = nRightIdx + 1;
            }
            else if (nRightIdx + 1 == nPivot)
            {
                nPivot = nLeftIdx = nLeftIdx - 1;
            }
        }

        InPlaceQuickSort_r(pfnCompare, nLeft, nPivot - 1);
        InPlaceQuickSort_r(pfnCompare, nPivot + 1, nRight);
    }
}

//----------------------------------------------------------------------------------------------
// Call this to quickly sort non-contiguously allocated vectors. Sort uses a slower bubble sort.
//----------------------------------------------------------------------------------------------
template <typename T, class A> void CUtlVector<T, A>::InPlaceQuickSort(int(__cdecl* pfnCompare)(const T*, const T*))
{
    InPlaceQuickSort_r(pfnCompare, 0, Count() - 1);
}

//-----------------------------------------------------------------------------
// Makes sure we have enough memory allocated to store a requested # of elements
//-----------------------------------------------------------------------------
template <typename T, class A> void CUtlVector<T, A>::EnsureCapacity(int num)
{
    MEM_ALLOC_CREDIT_CLASS();
    m_Memory.EnsureCapacity(num);
    ResetDbgInfo();
}

//-----------------------------------------------------------------------------
// Makes sure we have at least this many elements
//-----------------------------------------------------------------------------
template <typename T, class A> void CUtlVector<T, A>::EnsureCount(int num)
{
    if (Count() < num)
    {
        AddMultipleToTail(num - Count());
    }
}

//-----------------------------------------------------------------------------
// Shifts elements
//-----------------------------------------------------------------------------
template <typename T, class A> void CUtlVector<T, A>::ShiftElementsRight(int elem, int num)
{
    // Assert(IsValidIndex(elem) || (m_Size == 0) || (num == 0));
    int numToMove = m_Size - elem - num;
    if ((numToMove > 0) && (num > 0))
        memmove(&Element(elem + num), &Element(elem), numToMove * sizeof(T));
}

template <typename T, class A> void CUtlVector<T, A>::ShiftElementsLeft(int elem, int num)
{
    // Assert(IsValidIndex(elem) || (m_Size == 0) || (num == 0));
    int numToMove = m_Size - elem - num;
    if ((numToMove > 0) && (num > 0))
    {
        memmove(&Element(elem), &Element(elem + num), numToMove * sizeof(T));

#ifdef _DEBUG
        memset(&Element(m_Size - num), 0xDD, num * sizeof(T));
#endif
    }
}

//-----------------------------------------------------------------------------
// Adds an element, uses default constructor
//-----------------------------------------------------------------------------
template <typename T, class A> inline int CUtlVector<T, A>::AddToHead()
{
    return InsertBefore(0);
}

template <typename T, class A> inline int CUtlVector<T, A>::AddToTail()
{
    return InsertBefore(m_Size);
}

template <typename T, class A> inline int CUtlVector<T, A>::InsertAfter(int elem)
{
    return InsertBefore(elem + 1);
}

template <typename T, class A> int CUtlVector<T, A>::InsertBefore(int elem)
{
    // Can insert at the end
    // Assert((elem == Count()) || IsValidIndex(elem));

    GrowVector();
    ShiftElementsRight(elem);
    Construct(&Element(elem));
    return elem;
}

//-----------------------------------------------------------------------------
// Adds an element, uses copy constructor
//-----------------------------------------------------------------------------
template <typename T, class A> inline int CUtlVector<T, A>::AddToHead(const T& src)
{
    // Can't insert something that's in the list... reallocation may hose us
    // Assert((Base() == NULL) || (&src < Base()) || (&src >= (Base() + Count())));
    return InsertBefore(0, src);
}

template <typename T, class A> inline int CUtlVector<T, A>::AddToTail(const T& src)
{
    // Can't insert something that's in the list... reallocation may hose us
    // Assert((Base() == NULL) || (&src < Base()) || (&src >= (Base() + Count())));
    return InsertBefore(m_Size, src);
}

template <typename T, class A> inline int CUtlVector<T, A>::InsertAfter(int elem, const T& src)
{
    // Can't insert something that's in the list... reallocation may hose us
    // Assert((Base() == NULL) || (&src < Base()) || (&src >= (Base() + Count())));
    return InsertBefore(elem + 1, src);
}

template <typename T, class A> int CUtlVector<T, A>::InsertBefore(int elem, const T& src)
{
    // Can't insert something that's in the list... reallocation may hose us
    // Assert((Base() == NULL) || (&src < Base()) || (&src >= (Base() + Count())));

    // Can insert at the end
    // Assert((elem == Count()) || IsValidIndex(elem));

    GrowVector();
    ShiftElementsRight(elem);
    CopyConstruct(&Element(elem), src);
    return elem;
}

//-----------------------------------------------------------------------------
// Adds multiple elements, uses default constructor
//-----------------------------------------------------------------------------
template <typename T, class A> inline int CUtlVector<T, A>::AddMultipleToHead(int num)
{
    return InsertMultipleBefore(0, num);
}

template <typename T, class A> inline int CUtlVector<T, A>::AddMultipleToTail(int num)
{
    return InsertMultipleBefore(m_Size, num);
}

template <typename T, class A> inline int CUtlVector<T, A>::AddMultipleToTail(int num, const T* pToCopy)
{
    // Can't insert something that's in the list... reallocation may hose us
    // Assert((Base() == NULL) || !pToCopy || (pToCopy + num <= Base()) || (pToCopy >= (Base() + Count())));

    return InsertMultipleBefore(m_Size, num, pToCopy);
}

template <typename T, class A> int CUtlVector<T, A>::InsertMultipleAfter(int elem, int num)
{
    return InsertMultipleBefore(elem + 1, num);
}

template <typename T, class A> void CUtlVector<T, A>::SetCount(int count)
{
    RemoveAll();
    AddMultipleToTail(count);
}

template <typename T, class A> inline void CUtlVector<T, A>::SetSize(int size)
{
    SetCount(size);
}

template <typename T, class A> void CUtlVector<T, A>::SetCountNonDestructively(int count)
{
    int delta = count - m_Size;
    if (delta > 0)
        AddMultipleToTail(delta);
    else if (delta < 0)
        RemoveMultipleFromTail(-delta);
}

template <typename T, class A> void CUtlVector<T, A>::CopyArray(const T* pArray, int size)
{
    // Can't insert something that's in the list... reallocation may hose us
    // Assert((Base() == NULL) || !pArray || (Base() >= (pArray + size)) || (pArray >= (Base() + Count())));

    SetSize(size);
    for (int i = 0; i < size; i++)
    {
        (*this)[i] = pArray[i];
    }
}

template <typename T, class A> void CUtlVector<T, A>::Swap(CUtlVector<T, A>& vec)
{
    m_Memory.Swap(vec.m_Memory);
    swap(m_Size, vec.m_Size);
#ifndef _X360
    swap(m_pElements, vec.m_pElements);
#endif
}

template <typename T, class A> int CUtlVector<T, A>::AddVectorToTail(CUtlVector const& src)
{
    // Assert(&src != this);

    int base = Count();

    // Make space.
    int nSrcCount = src.Count();
    EnsureCapacity(base + nSrcCount);

    // Copy the elements.
    m_Size += nSrcCount;
    for (int i = 0; i < nSrcCount; i++)
    {
        CopyConstruct(&Element(base + i), src[i]);
    }
    return base;
}

template <typename T, class A> inline int CUtlVector<T, A>::InsertMultipleBefore(int elem, int num)
{
    if (num == 0)
        return elem;

    // Can insert at the end
    // Assert((elem == Count()) || IsValidIndex(elem));

    GrowVector(num);
    ShiftElementsRight(elem, num);

    // Invoke default constructors
    for (int i = 0; i < num; ++i)
    {
        Construct(&Element(elem + i));
    }

    return elem;
}

template <typename T, class A> inline int CUtlVector<T, A>::InsertMultipleBefore(int elem, int num, const T* pToInsert)
{
    if (num == 0)
        return elem;

    // Can insert at the end
    // Assert((elem == Count()) || IsValidIndex(elem));

    GrowVector(num);
    ShiftElementsRight(elem, num);

    // Invoke default constructors
    if (!pToInsert)
    {
        for (int i = 0; i < num; ++i)
        {
            Construct(&Element(elem + i));
        }
    }
    else
    {
        for (int i = 0; i < num; i++)
        {
            CopyConstruct(&Element(elem + i), pToInsert[i]);
        }
    }

    return elem;
}

//-----------------------------------------------------------------------------
// Finds an element (element needs operator== defined)
//-----------------------------------------------------------------------------
template <typename T, class A> int CUtlVector<T, A>::Find(const T& src) const
{
    for (int i = 0; i < Count(); ++i)
    {
        if (Element(i) == src)
            return i;
    }
    return -1;
}

template <typename T, class A> void CUtlVector<T, A>::FillWithValue(const T& src)
{
    for (int i = 0; i < Count(); i++)
    {
        Element(i) = src;
    }
}

template <typename T, class A> bool CUtlVector<T, A>::HasElement(const T& src) const
{
    return (Find(src) >= 0);
}

//-----------------------------------------------------------------------------
// Element removal
//-----------------------------------------------------------------------------
template <typename T, class A> void CUtlVector<T, A>::FastRemove(int elem)
{
    // Assert(IsValidIndex(elem));

    Destruct(&Element(elem));
    if (m_Size > 0)
    {
        if (elem != m_Size - 1)
            memcpy(&Element(elem), &Element(m_Size - 1), sizeof(T));
        --m_Size;
    }
}

template <typename T, class A> void CUtlVector<T, A>::Remove(int elem)
{
    Destruct(&Element(elem));
    ShiftElementsLeft(elem);
    --m_Size;
}

template <typename T, class A> bool CUtlVector<T, A>::FindAndRemove(const T& src)
{
    int elem = Find(src);
    if (elem != -1)
    {
        Remove(elem);
        return true;
    }
    return false;
}

template <typename T, class A> bool CUtlVector<T, A>::FindAndFastRemove(const T& src)
{
    int elem = Find(src);
    if (elem != -1)
    {
        FastRemove(elem);
        return true;
    }
    return false;
}

template <typename T, class A> void CUtlVector<T, A>::RemoveMultiple(int elem, int num)
{
    // Assert(elem >= 0);
    // Assert(elem + num <= Count());

    for (int i = elem + num; --i >= elem;)
        Destruct(&Element(i));

    ShiftElementsLeft(elem, num);
    m_Size -= num;
}

template <typename T, class A> void CUtlVector<T, A>::RemoveMultipleFromHead(int num)
{
    // Assert(num <= Count());

    for (int i = num; --i >= 0;)
        Destruct(&Element(i));

    ShiftElementsLeft(0, num);
    m_Size -= num;
}

template <typename T, class A> void CUtlVector<T, A>::RemoveMultipleFromTail(int num)
{
    // Assert(num <= Count());

    for (int i = m_Size - num; i < m_Size; i++)
        Destruct(&Element(i));

    m_Size -= num;
}

template <typename T, class A> void CUtlVector<T, A>::RemoveAll()
{
    for (int i = m_Size; --i >= 0;)
    {
        Destruct(&Element(i));
    }

    m_Size = 0;
}

//-----------------------------------------------------------------------------
// Memory deallocation
//-----------------------------------------------------------------------------

template <typename T, class A> inline void CUtlVector<T, A>::Purge()
{
    RemoveAll();
    m_Memory.Purge();
    ResetDbgInfo();
}

template <typename T, class A> inline void CUtlVector<T, A>::PurgeAndDeleteElements()
{
    for (int i = 0; i < m_Size; i++)
    {
        delete Element(i);
    }
    Purge();
}

template <typename T, class A> inline void CUtlVector<T, A>::Compact()
{
    m_Memory.Purge(m_Size);
}

template <typename T, class A> inline int CUtlVector<T, A>::NumAllocated() const
{
    return m_Memory.NumAllocated();
}

template <class T, class M = CUtlMemory<T>> class CUtlStack
{
public:
    // constructor, destructor
    CUtlStack(int growSize = 0, int initSize = 0);
    ~CUtlStack();

    void CopyFrom(const CUtlStack<T, M>& from);

    // element access
    T& operator[](int i);
    T const& operator[](int i) const;
    T& Element(int i);
    T const& Element(int i) const;

    // Gets the base address (can change when adding elements!)
    T* Base();
    T const* Base() const;

    // Looks at the stack top
    T& Top();
    T const& Top() const;

    // Size
    int Count() const;

    // Is element index valid?
    bool IsIdxValid(int i) const;

    // Adds an element, uses default constructor
    int Push();

    // Adds an element, uses copy constructor
    int Push(T const& src);

    // Pops the stack
    void Pop();
    void Pop(T& oldTop);
    void PopMultiple(int num);

    // Makes sure we have enough memory allocated to store a requested # of elements
    void EnsureCapacity(int num);

    // Clears the stack, no deallocation
    void Clear();

    // Memory deallocation
    void Purge();

private:
    // Grows the stack allocation
    void GrowStack();

    // For easier access to the elements through the debugger
    void ResetDbgInfo();

    M m_Memory;
    int m_Size;

    // For easier access to the elements through the debugger
    T* m_pElements;
};

//-----------------------------------------------------------------------------
// For easier access to the elements through the debugger
//-----------------------------------------------------------------------------

template <class T, class M> inline void CUtlStack<T, M>::ResetDbgInfo()
{
    m_pElements = m_Memory.Base();
}

//-----------------------------------------------------------------------------
// constructor, destructor
//-----------------------------------------------------------------------------

template <class T, class M>
CUtlStack<T, M>::CUtlStack(int growSize, int initSize) : m_Memory(growSize, initSize), m_Size(0)
{
    ResetDbgInfo();
}

template <class T, class M> CUtlStack<T, M>::~CUtlStack()
{
    Purge();
}

//-----------------------------------------------------------------------------
// copy into
//-----------------------------------------------------------------------------

template <class T, class M> void CUtlStack<T, M>::CopyFrom(const CUtlStack<T, M>& from)
{
    Purge();
    EnsureCapacity(from.Count());
    for (int i = 0; i < from.Count(); i++)
    {
        Push(from[i]);
    }
}

//-----------------------------------------------------------------------------
// element access
//-----------------------------------------------------------------------------

template <class T, class M> inline T& CUtlStack<T, M>::operator[](int i)
{
    // assert(IsIdxValid(i));
    return m_Memory[i];
}

template <class T, class M> inline T const& CUtlStack<T, M>::operator[](int i) const
{
    // assert(IsIdxValid(i));
    return m_Memory[i];
}

template <class T, class M> inline T& CUtlStack<T, M>::Element(int i)
{
    // assert(IsIdxValid(i));
    return m_Memory[i];
}

template <class T, class M> inline T const& CUtlStack<T, M>::Element(int i) const
{
    // assert(IsIdxValid(i));
    return m_Memory[i];
}

//-----------------------------------------------------------------------------
// Gets the base address (can change when adding elements!)
//-----------------------------------------------------------------------------

template <class T, class M> inline T* CUtlStack<T, M>::Base()
{
    return m_Memory.Base();
}

template <class T, class M> inline T const* CUtlStack<T, M>::Base() const
{
    return m_Memory.Base();
}

//-----------------------------------------------------------------------------
// Returns the top of the stack
//-----------------------------------------------------------------------------

template <class T, class M> inline T& CUtlStack<T, M>::Top()
{
    // assert(m_Size > 0);
    return Element(m_Size - 1);
}

template <class T, class M> inline T const& CUtlStack<T, M>::Top() const
{
    // assert(m_Size > 0);
    return Element(m_Size - 1);
}

//-----------------------------------------------------------------------------
// Size
//-----------------------------------------------------------------------------

template <class T, class M> inline int CUtlStack<T, M>::Count() const
{
    return m_Size;
}

//-----------------------------------------------------------------------------
// Is element index valid?
//-----------------------------------------------------------------------------

template <class T, class M> inline bool CUtlStack<T, M>::IsIdxValid(int i) const
{
    return (i >= 0) && (i < m_Size);
}

//-----------------------------------------------------------------------------
// Grows the stack
//-----------------------------------------------------------------------------

template <class T, class M> void CUtlStack<T, M>::GrowStack()
{
    if (m_Size >= m_Memory.NumAllocated())
        m_Memory.Grow();

    ++m_Size;

    ResetDbgInfo();
}

//-----------------------------------------------------------------------------
// Makes sure we have enough memory allocated to store a requested # of elements
//-----------------------------------------------------------------------------

template <class T, class M> void CUtlStack<T, M>::EnsureCapacity(int num)
{
    m_Memory.EnsureCapacity(num);
    ResetDbgInfo();
}

//-----------------------------------------------------------------------------
// Adds an element, uses default constructor
//-----------------------------------------------------------------------------

template <class T, class M> int CUtlStack<T, M>::Push()
{
    GrowStack();
    Construct(&Element(m_Size - 1));
    return m_Size - 1;
}

//-----------------------------------------------------------------------------
// Adds an element, uses copy constructor
//-----------------------------------------------------------------------------

template <class T, class M> int CUtlStack<T, M>::Push(T const& src)
{
    GrowStack();
    CopyConstruct(&Element(m_Size - 1), src);
    return m_Size - 1;
}

//-----------------------------------------------------------------------------
// Pops the stack
//-----------------------------------------------------------------------------

template <class T, class M> void CUtlStack<T, M>::Pop()
{
    // assert(m_Size > 0);
    Destruct(&Element(m_Size - 1));
    --m_Size;
}

template <class T, class M> void CUtlStack<T, M>::Pop(T& oldTop)
{
    // assert(m_Size > 0);
    oldTop = Top();
    Pop();
}

template <class T, class M> void CUtlStack<T, M>::PopMultiple(int num)
{
    // assert(m_Size >= num);
    for (int i = 0; i < num; ++i)
        Destruct(&Element(m_Size - i - 1));
    m_Size -= num;
}

//-----------------------------------------------------------------------------
// Element removal
//-----------------------------------------------------------------------------

template <class T, class M> void CUtlStack<T, M>::Clear()
{
    for (int i = m_Size; --i >= 0;)
        Destruct(&Element(i));

    m_Size = 0;
}

//-----------------------------------------------------------------------------
// Memory deallocation
//-----------------------------------------------------------------------------

template <class T, class M> void CUtlStack<T, M>::Purge()
{
    Clear();
    m_Memory.Purge();
    ResetDbgInfo();
}

//-----------------------------------------------------------------------------
// forward declarations
//-----------------------------------------------------------------------------
class CUtlSymbolTable;
class CUtlSymbolTableMT;

//-----------------------------------------------------------------------------
// This is a symbol, which is a easier way of dealing with strings.
//-----------------------------------------------------------------------------
typedef unsigned short UtlSymId_t;

#define UTL_INVAL_SYMBOL ((UtlSymId_t)~0)

class CUtlSymbol
{
public:
    // constructor, destructor
    CUtlSymbol() : m_Id(UTL_INVAL_SYMBOL)
    {
    }
    CUtlSymbol(UtlSymId_t id) : m_Id(id)
    {
    }

    CUtlSymbol(CUtlSymbol const& sym) : m_Id(sym.m_Id)
    {
    }

    // operator=
    CUtlSymbol& operator=(CUtlSymbol const& src)
    {
        m_Id = src.m_Id;
        return *this;
    }

    // operator==
    bool operator==(CUtlSymbol const& src) const
    {
        return m_Id == src.m_Id;
    }

    // Is valid?
    bool IsValid() const
    {
        return m_Id != UTL_INVAL_SYMBOL;
    }

    // Gets at the symbol
    operator UtlSymId_t() const
    {
        return m_Id;
    }

protected:
    UtlSymId_t m_Id;

    /*
    // The standard global symbol table
    static CUtlSymbolTableMT* s_pSymbolTable;

    static bool s_bAllowStaticSymbolTable;

    friend class CCleanupUtlSymbolTable;*/
};
