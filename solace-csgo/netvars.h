
#include <cstdint>

class c_recv_proxy_data;
typedef void( *recv_var_proxy_fn )( const c_recv_proxy_data *pData, void *pStruct, void *pOut );

namespace netvar_manager { // ripped this from designer, same method everyone uses
	uintptr_t get_net_var( const uint32_t table,
						   const uint32_t prop );
	void set_proxy( uint32_t table, uint32_t prop, void *proxy, recv_var_proxy_fn &original );
};

// more macro abuse.. if only resharper could convert macro calls to code
#define NETVAR( table, prop, name ) uintptr_t name = netvar_manager::get_net_var( fnv::hash( table ), fnv::hash( prop ) );
#define OFFSET( type, var, offset ) type& var() { \
    return *reinterpret_cast< type* >( uintptr_t( this ) + (offset) ); \
}
#define OFFSETPTR( type, var, offset ) type* var() { \
    return reinterpret_cast< type* >( uintptr_t( this ) + (offset) ); \
}