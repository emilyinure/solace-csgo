#pragma once
#define BASE_TEMPLATE template<typename t = address>
#include "utils.h"

class address {
public:
	uintptr_t m_base{};
	__forceinline ~address ( ) {};
	__forceinline address( const void* a ) : m_base{ reinterpret_cast< uintptr_t >(a) } { }
	__forceinline operator void* ( ) const { return ( void* )m_base; }
	__forceinline operator const void *( ) const { return ( const void * )m_base; }
	__forceinline operator const bool( ) const { return valid( ); }
	
	__forceinline bool operator!=( address other ) const { return m_base != other.m_base; }

	address( ) {
		m_base = 0;
	}

	address( const uintptr_t addr ) {
		m_base = addr;
	}

	__forceinline operator uintptr_t( ) { return m_base; }

	BASE_TEMPLATE
	t as ( const uintptr_t offset = 0 ) {
		//cast
		return valid( ) ? reinterpret_cast< t >(m_base + offset) : 0;
	}

	template< typename t = address >
	__forceinline t get( size_t n = 1 ) {

		if ( !m_base )
			return t{};

		uintptr_t out = m_base;

		for ( size_t i{ n }; i > 0; --i ) {
			// can't dereference, return null.
			if ( !valid( ) )
				return t{};

			out = *reinterpret_cast< uintptr_t * >(out);
		}

		return (t)( out );
	}

	BASE_TEMPLATE
		t sub( const uintptr_t offset = 0 ) {
		//cast
		return valid( ) ? static_cast< t >( m_base -= offset ) : 0;
	}
	
	BASE_TEMPLATE
		t add( const uintptr_t offset = 0 ) {
		//cast
		return valid( ) ? static_cast< t >( m_base += offset ) : 0;
	}

	BASE_TEMPLATE
	t to ( const uintptr_t offset = 0 ) {
		return valid( ) ? *reinterpret_cast< t * >(m_base + offset) : 0;
	}
	BASE_TEMPLATE
	t deref( int n = 1 ) {
		auto new_addr = m_base;
		for ( auto i = 1; i < n; i++ )
			new_addr = *reinterpret_cast< uintptr_t * >(new_addr);
		return *reinterpret_cast< t * >(new_addr);
	}

	BASE_TEMPLATE
	t at ( const uintptr_t offset ) {
		return valid( ) ? *reinterpret_cast< t * >(m_base + offset) : 0;
	}

	[[nodiscard]] bool valid ( ) const {
		//nulltptr?
		return m_base != 0;
	}

	BASE_TEMPLATE
	t rel32 ( const size_t offset ) {
		if ( !m_base )
			return t{};

		auto out = m_base + offset;

		// get rel32 offset.
		const auto r = *reinterpret_cast< int32_t* >(out);
		if ( !r )
			return t{};

		// relative to address of next instruction.
		out = (out + 4) + r;

		return static_cast< t >(out);
	}
};

#undef BASE_TEMPLATE
