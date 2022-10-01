#pragma once
#include <cstdint>
#include <cstring>
#include <memory>

class c_hook {
private:
	uintptr_t                      m_base;
	void **m_old_vmt;
	std::unique_ptr< uintptr_t[ ] > m_new_vmt;
	size_t                       m_size;
	bool                         m_rtti;

private:
	__forceinline size_t count_methods( ) const {
		auto i{ 0u };

		while ( m_old_vmt[ i ] != nullptr )
			++i;

		return i;
	}

public:
	// default ctor.
	__forceinline c_hook( ) : m_base{}, m_old_vmt{}, m_new_vmt{}, m_size{}, m_rtti{} {}

	// ctor.
	__forceinline c_hook( uintptr_t base, bool rtti = true ) : m_base{}, m_old_vmt{}, m_new_vmt{}, m_size{}, m_rtti{} {
		init( base, rtti );
	}
	__forceinline c_hook( void* base, bool rtti = true ) : m_base{}, m_old_vmt{}, m_new_vmt{}, m_size{}, m_rtti{} {
		init( ( uintptr_t )base, rtti );
	}

	// dtor.
	__forceinline ~c_hook( ) {
		reset( );
	}

	// reset entire class.
	__forceinline void reset( ) {
		m_new_vmt.reset( );

		if ( m_base )
			*( uintptr_t * )m_base = ( uintptr_t )m_old_vmt;

		m_base = uintptr_t{};
		m_old_vmt = nullptr;
		m_size = 0;
		m_rtti = false;
	}

	// setup and replace vmt.
	__forceinline void init( uintptr_t base, const bool rtti = true ) {
		// save base class.
		m_base = base;

		// get ptr to old VMT.
		m_old_vmt = *reinterpret_cast< void *** >( base );
		if ( !m_old_vmt )
			return;

		// count number of methods in old VMT.
		m_size = count_methods( );
		if ( !m_size )
			return;

		// allocate new VMT.
		m_new_vmt = std::make_unique< uintptr_t[ ] >( rtti ? m_size + 1 : m_size );
		if ( !m_new_vmt )
			return;

		// get raw memory ptr.
		const auto vmt = reinterpret_cast< uintptr_t >( m_new_vmt.get( ) );

		if ( rtti ) {
			// copy VMT, starting from RTTI.
			std::memcpy( reinterpret_cast< uintptr_t * >( vmt ), m_old_vmt - 1, ( m_size + 1 ) * sizeof( uintptr_t ) );

			// VMTs are ( usually ) stored in the .data section we should be able to just overwrite it, so let's do that here.
			// also, since we've copied RTTI ptr then point the new table at index 1 ( index 0 contains RTTI ptr ).
			*reinterpret_cast< uintptr_t * >( base ) = ( vmt + sizeof( uintptr_t ) );

			// we've sucesfully copied the RTTI ptr.
			m_rtti = true;
		}

		else {
			// copy vmt.
			std::memcpy( reinterpret_cast< uintptr_t * >( vmt ), m_old_vmt, m_size * sizeof( uintptr_t ) );

			// since VMTs are ( usually ) stored in the .data section we should be able to just overwrite it, so let's do that here.
			*reinterpret_cast< uintptr_t * >( base ) = vmt;
		}
	}

	template< typename t = uintptr_t >
	__forceinline t add( void *function_ptr, size_t index ) {
		const auto vmt_index{ m_rtti ? index + 1 : index };

		// sanity check some stuff first.
		if ( !m_old_vmt || !m_new_vmt || vmt_index > m_size )
			return t{};

		// redirect.
		m_new_vmt[ vmt_index ] = reinterpret_cast< uintptr_t >( function_ptr );

		return reinterpret_cast< t >(m_old_vmt[index]);
	}

	__forceinline bool remove( size_t index ) const {
		const auto vmt_index{ m_rtti ? index + 1 : index };

		// sanity check some stuff first.
		if ( !m_old_vmt || !m_new_vmt || vmt_index > m_size )
			return false;

		// redirect.
		//m_new_vmt[vmt_index] = m_old_vmt[index];

		return true;
	}

	template< typename t = uintptr_t >
	__forceinline t get_original( size_t index ) {
		return  reinterpret_cast< t >(m_old_vmt[index]);
	}
};
