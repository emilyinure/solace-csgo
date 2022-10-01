#pragma once
#include <algorithm>
#include <locale>
#include "vector"
#include "sstream"


#include "address.h"

#define CONCAT_TOKEN( x, y ) x##y
#define MACRO_CONCAT( x, y ) CONCAT_TOKEN( x, y )
#define PAD( size ) char MACRO_CONCAT( _pad, __COUNTER__ )[ size ];

#define BASE_TEMPLATE template < typename t = void* >

class util {
	class c_interface_reg {
	private:
		using create_t = void* (__cdecl*) ( );

	public:
		create_t m_create_fn;
		const char *m_name;
		c_interface_reg *m_next;
	};

public:
	BASE_TEMPLATE
	static t iter_interfaces ( c_interface_reg *register_list, const char *name ) {
		for ( auto *cur = register_list; cur; cur = cur->m_next ) {
			if ( strcmp( cur->m_name, name ) == 0 )
				return static_cast< t >(cur->m_create_fn( ));
		}
		return {};
	}

	BASE_TEMPLATE
	static t capture_interface ( const char *module, const char *name ) {
		static address interface_fn(
			reinterpret_cast< uintptr_t >(GetProcAddress( GetModuleHandleA( module ), "CreateInterface" )) );

		if ( !interface_fn.valid( ) ) { return nullptr; }
		if ( interface_fn.at< uint8_t >( 4 ) == 233 ) {
			auto jump_target = interface_fn.rel32( 5 );

			if ( jump_target.at< uint8_t >( 5 ) == 53 ) {
				return iter_interfaces< t >( **jump_target.as< c_interface_reg *** >( 6 ), name );
			}
		}
		else if ( interface_fn.at< uint8_t >( 2 ) == 53 ) {
			return iter_interfaces< t >( **interface_fn.as< c_interface_reg *** >( 3 ), name );
		}

		return nullptr;
	}
	BASE_TEMPLATE
	static t get_virtual_function( void *base, const std::uint16_t index ) {
		return ( *static_cast< t ** >( base ) )[ index ];
	}
	// container
	using patterns_t = std::vector< address >;
	using pattern_byte_t = std::pair< uint8_t, bool >;
	
	static std::uint8_t *find( const char *module_name, const char *signature, size_t len = -1 ) noexcept {
		auto *const module_handle = GetModuleHandleA( module_name );

		if ( !module_handle )
			return nullptr;

		auto pattern_to_byte = [ ]( const char *pattern ) {
			auto bytes = std::vector<int>{};
			const auto start = const_cast< char * >( pattern );
			const auto end = const_cast< char * >( pattern ) + std::strlen( pattern );

			for ( auto *current = start; current < end; ++current ) {
				if ( *current == '?' ) {
					++current;

					if ( *current == '?' )
						++current;

					bytes.push_back( -1 );
				} else {
					bytes.push_back( std::strtoul( current, &current, 16 ) );
				}
			}
			return bytes;
		};

		const auto dos_header = reinterpret_cast< PIMAGE_DOS_HEADER >( module_handle );
		const auto nt_headers =
			reinterpret_cast< PIMAGE_NT_HEADERS >( reinterpret_cast< std::uint8_t * >( module_handle ) + dos_header->e_lfanew );

		auto size_of_image = nt_headers->OptionalHeader.SizeOfImage;
		if ( len != -1 )
			size_of_image = len;
		auto pattern_bytes = pattern_to_byte( signature );
		const auto scan_bytes = reinterpret_cast< std::uint8_t * >( module_handle );

		const auto s = pattern_bytes.size( );
		const auto d = pattern_bytes.data( );

		for ( auto i = 0ul; i < size_of_image - s; ++i ) {
			auto found = true;

			for ( auto j = 0ul; j < s; ++j ) {
				if ( scan_bytes[ i + j ] != d[ j ] && d[ j ] != -1 ) {
					found = false;
					break;
				}
			}
			if ( found )
				return &scan_bytes[ i ];
		}

		throw std::runtime_error( std::string( "Wrong signature: " ) + signature );
	}
	
	static std::uint8_t *find( address start, size_t len, const char *pat ) noexcept {
		if ( !start )
			return nullptr;

		static auto pattern_to_byte = [ ]( const char *pattern ) {
			auto bytes = std::vector<int>{};
			const auto start = const_cast< char * >( pattern );
			const auto end = const_cast< char * >( pattern ) + std::strlen( pattern );

			for ( auto *current = start; current < end; ++current ) {
				if ( *current == '?' ) {
					++current;

					if ( *current == '?' )
						++current;

					bytes.push_back( -1 );
				} else {
					bytes.push_back( std::strtoul( current, &current, 16 ) );
				}
			}
			return bytes;
		};

		auto pattern_bytes = pattern_to_byte( pat );
		const auto scan_bytes = start.as< std::uint8_t * >();

		const auto s = pattern_bytes.size( );
		const auto d = pattern_bytes.data( );

		for ( auto i = 0ul; i < len - s; ++i ) {
			auto found = true;

			for ( auto j = 0ul; j < s; ++j ) {
				if ( scan_bytes[ i + j ] != d[ j ] && d[ j ] != -1 ) {
					found = false;
					break;
				}
			}
			if ( found )
				return &scan_bytes[ i ];
		}
	}
	static address find( address start, size_t len, const std::string &pat ) {
		uint8_t *scan_start, *scan_end;
		std::vector< pattern_byte_t > pattern{};
		std::stringstream			  stream{ pat };
		std::string				      w;

		if ( !start || !len || pat.empty( ) )
			return{};

		// split spaces and convert to hex.
		while ( stream >> w ) {
			// wildcard.
			if ( w[ 0 ] == '?' )
				pattern.push_back( { 0, true } );

			// valid hex digits.
			else if ( std::isxdigit( w[ 0 ] ) && std::isxdigit( w[ 1 ] ) )
				pattern.push_back( { static_cast< uint8_t >(std::strtoul( w.data( ), 0, 16 )), false } );
		}

		scan_start = start.as< uint8_t * >( );
		scan_end = scan_start + len;

		// find match.
		auto result = std::search( scan_start, scan_end, pattern.begin( ), pattern.end( ),
								   [ ]( const uint8_t b, const pattern_byte_t &p ) {
									   // byte matches or it's a wildcard.
									   return b == p.first || p.second;
								   } );

		// nothing found.
		if ( result == scan_end )
			return{};

		return ( uintptr_t )result;
	}
	static patterns_t FindAll( const char *module, const std::string &pat ) {
		patterns_t out{};
		address	   result;

		const auto module_handle = GetModuleHandleA( module );

		if ( !module_handle )
			return {};
		
		const auto dos_header = reinterpret_cast< PIMAGE_DOS_HEADER >( module_handle );
		const auto nt_headers =
			reinterpret_cast< PIMAGE_NT_HEADERS >( reinterpret_cast< std::uint8_t * >( module_handle ) + dos_header->e_lfanew );

		size_t size_of_image = nt_headers->OptionalHeader.SizeOfImage;
		address start{ module_handle };
		for ( ;; ) {
			// find result.
			result = find( start, size_of_image, pat );
			if ( !result )
				break;

			// if we arrived here we found something.
			out.push_back( result );

			// set new len.
			size_of_image = ( start + size_of_image ) - ( result + 1 );

			// new start point.
			start = result;
			start.add( 1 );
		}

		return out;
	}


	static address find( std::string &module, const std::string &pat ) {
		return find(module.c_str(), pat.c_str(  ) );
	}

	//static patterns_t FindAll( address start, const std::string &pat ) {
	//	patterns_t out{};
	//	address	   result;
	//	size_t len = std::strlen( pat.c_str( ) );
	//
	//	for ( ;; ) {
	//		// find result.
	//		result = find( start, pat.c_str(  ), len );
	//		if ( !result )
	//			break;
	//
	//		// if we arrived here we found something.
	//		out.push_back( result );
	//
	//		// set new len.
	//		len = ( start + len ) - ( result + 1 );
	//
	//		// new start m_point.
	//		start = result + 1;
	//	}
	//
	//	return out;
	//}
	//
	//static patterns_t FindAll( std::string &module, const std::string &pat ) {
	//	return FindAll( module.c_str( ), module.GetImageSize( ), pat );
	//}
};

// abusing macros, ik will revisit it later - l
#define VFUNC( function_name, index, type, ... ) \
	auto function_name { \
		return util::get_virtual_function< type >( this, index )( this, __VA_ARGS__ ); \
	};

#undef BASE_TEMPLATE
