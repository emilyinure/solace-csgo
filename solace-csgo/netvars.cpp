#include "netvars.h"
#include <unordered_map>
#include <map>
#include "sdk.h"

namespace netvar_manager {
	struct netvar_data_t {
		bool        m_datamap_var; // we can't do proxies on stuff from datamaps :).
		recv_prop_t *m_prop_ptr;
		size_t      m_offset;
		int	        m_prop;

		__forceinline netvar_data_t() : m_datamap_var{}, m_prop_ptr{}, m_offset{}, m_prop{}{ }
	};
	using netvar_key_value_map = std::unordered_map< uint32_t, netvar_data_t >;
	using netvar_table_map = std::unordered_map< uint32_t, netvar_key_value_map >;
	void initialize_props( netvar_table_map &table_map );
	void add_props_for_table( netvar_table_map &table_map, const uint32_t table_name_hash, const std::string &table_name, recv_table_t *table, const bool dump_vars, const size_t child_offset = 0 ) {
		
		for ( auto i = 0; i < table->m_prop_count; ++i ) {
			auto &prop = table->m_props[ i ];
			if ( prop.m_data_table && prop.m_elements > 0 ) {
				if ( std::string( prop.m_var_name ).substr( 0, 1 ) == std::string( "0" ) )
					continue;

				add_props_for_table( table_map, table_name_hash, table_name, prop.m_data_table, dump_vars, prop.m_offset + child_offset );
			}

			auto name = std::string( prop.m_var_name );

			if ( name.substr( 0, 1 ) != "m" /*&& name.substr( 0, 1 ) != "b"*/ )
				continue;

			const auto name_hash = fnv::hash( prop.m_var_name );
			const auto offset = uintptr_t( prop.m_offset ) + child_offset;

			table_map[ table_name_hash ][ name_hash ].m_datamap_var = false;
			table_map[ table_name_hash ][ name_hash ].m_offset = offset;
			table_map[ table_name_hash ][ name_hash ].m_prop_ptr = &prop;
			table_map[ table_name_hash ][name_hash].m_prop = i;
		}
	}

	netvar_table_map map = {};
	uintptr_t get_net_var( const uint32_t table,
						   const uint32_t prop ) {
		if ( map.empty( ) )
			initialize_props( map );

		auto &table_map = map.at( table );
		if ( table_map.find( prop ) == table_map.end( ) )
			return 0;

		if ( map.find( table ) == map.end( ) )
			return 0;

		return table_map.at( prop ).m_offset;
	}

	void set_proxy( uint32_t table, uint32_t prop, void *proxy, recv_var_proxy_fn &original ) {
		auto netvar_entry = map[table][prop];

		// we can't set a proxy on a datamap.
		if ( netvar_entry.m_datamap_var )
			return;

		// save original.
		original = netvar_entry.m_prop_ptr->m_proxy_fn;

		// redirect.
		netvar_entry.m_prop_ptr->m_proxy_fn = static_cast< recv_var_proxy_fn >(proxy);
	}

	void StoreDataMap( address ptr, netvar_table_map &var_map );
	// iterate client module and find all datamaps.
	void FindAndStoreDataMaps( netvar_table_map &map ) {
		auto matches = util::FindAll( "client.dll", "C7 05 ? ? ? ? ? ? ? ? C7 05 ? ? ? ? ? ? ? ? C3 CC" );
		if ( matches.empty( ) )
			return;
	
		for ( auto &m : matches )
			StoreDataMap( m, map );
	}
	void StoreDataMap( address ptr, netvar_table_map &var_map ) {
		// get datamap and verify.
		auto *const map = ptr.at( 2 ).sub( 4 ).as< datamap_t * >( );
	
		if ( !map || !map->m_num_fields || map->m_num_fields > 200 || !map->m_desc || !map->m_name )
			return;
	
		// hash table name.
		const auto base = fnv::hash( map->m_name );
	
		for ( int i{}; i < map->m_num_fields; ++i ) {
			auto entry = &map->m_desc[i];
			if ( !entry->m_name )
				continue;
	
			// hash var name.
			auto var = fnv::hash( entry->m_name );
	
			// if we dont have this var stored yet.
			if ( !var_map[ base ][ var ].m_offset ) {
				var_map[ base ][ var ].m_datamap_var = true;
				var_map[ base ][ var ].m_offset = static_cast< size_t >(entry->fieldOffset);
				var_map[ base ][ var ].m_prop_ptr = nullptr;
			}
		}
	}
	
	void initialize_props( netvar_table_map &table_map ) {
		const auto dump_vars = true;  //true if netvar dump

		netvar_table_map var_dump;
		for ( auto *client_class = g.m_interfaces->client( )->get_all_classes( );
			  client_class;
			  client_class = client_class->m_pNext ) {
			auto *const table = client_class->m_pRecvTable;
			auto *const table_name = table->m_net_table_name;
			const auto table_name_hash = fnv::hash( table_name );

			if ( table == nullptr )
				continue;

			add_props_for_table( table_map, table_name_hash, table_name, table, dump_vars );
		}
		FindAndStoreDataMaps( table_map );
	}
}
