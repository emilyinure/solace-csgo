#pragma once

template< class T, class I = int >
class CUtlMemory {
public:
	__forceinline T &operator[ ]( I i ) {
		return m_pMemory[ i ];
	}

	static int CalcNewAllocationCount( int count, int size, int requested, int bytes ) {
		if ( size )
			count = ( ( 1 + ( ( requested - 1 ) / size ) ) * size );

		else {
			if ( !count )
				count = ( 31 + bytes ) / bytes;

			while ( count < requested )
				count *= 2;
		}

		return count;
	}

	__forceinline bool IsExternallyAllocated( ) const {
		return m_nGrowSize < 0;
	}

	int m_nAllocationCount;
protected:
	T *m_pMemory;
	int m_nGrowSize;
};

template < class T >
__forceinline T *Construct( T *pMemory ) {
	return ::new( pMemory ) T;
}

template <class T>
__forceinline void Destruct( T *pMemory ) {
	pMemory->~T( );
}

template< class T, class M = CUtlMemory< T > >
class CUtlStack {
public:
	M m_Memory;
	int m_Size;

	// For easier access to the elements through the debugger
	T *m_pElements;
};
template< class T, class A = CUtlMemory< T > >
class CUtlVector {
	typedef A CAllocator;
public:
	__forceinline T &operator[]( int i ) {
		return m_Memory[ i ];
	}

	__forceinline T &Element( int i ) {
		return m_Memory[ i ];
	}

	__forceinline T *Base( ) {
		return m_Memory.Base( );
	}

	__forceinline int Count( ) const {
		return m_Size;
	}

	__forceinline void RemoveAll( ) {
		for ( int i = m_Size; --i >= 0; )
			Destruct( &Element( i ) );

		m_Size = 0;
	}

	//__forceinline int AddToTail( ) {
	//	return InsertBefore( m_Size );
	//}

	//__forceinline int InsertBefore( int elem ) {
	//	GrowVector( );
	//	ShiftElementsRight( elem );
	//	Construct( &Element( elem ) );
	//
	//	return elem;
	//}

	//__forceinline void GrowVector( int num = 1 ) {
	//	if ( m_Size + num > m_Memory.m_nAllocationCount )
	//		m_Memory.Grow( m_Size + num - m_Memory.m_nAllocationCount );
	//
	//	m_Size += num;
	//	ResetDbgInfo( );
	//}

protected:
	__forceinline void ShiftElementsRight( int elem, int num = 1 ) {
		const int numToMove = m_Size - elem - num;
		if ( ( numToMove > 0 ) && ( num > 0 ) )
			memmove( &Element( elem + num ), &Element( elem ), numToMove * sizeof( T ) );
	}

	CAllocator m_Memory;
	int        m_Size;
	T *m_pElements;

	__forceinline void ResetDbgInfo( ) {
		m_pElements = Base( );
	}
};