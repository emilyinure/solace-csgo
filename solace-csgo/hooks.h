#pragma once
#include "vmt.h"
#include "includes.h"

class hooks_t {
	c_hook m_players[ 64 ];

	class IEntityListener {
	public:
		virtual void OnEntityCreated( entity_t *ent ) = 0;
		// virtual void OnEntitySpawned( Entity *ent ) = 0; // note - dex; doesn't seem used on the client?
		virtual void OnEntityDeleted( entity_t *ent ) = 0;
	};

	class CustomEntityListener : public IEntityListener {
	public:
		void OnEntityCreated( entity_t *ent ) override;
		void OnEntityDeleted( entity_t *ent ) override;

		using AddListenerEntity_t = void( __stdcall * )( IEntityListener * );
		__forceinline void init( ) {
			static auto AddListenerEntity = ( AddListenerEntity_t )( util::find( "client.dll", "55 8B EC 8B 0D ? ? ? ? 33 C0 56 85 C9 7E 32 8B 55 08 8B 35" ) );
			AddListenerEntity( this );
		}
	}  m_custom_entity_listener{};
	
public:
	hooks_t( );

	~hooks_t( ) {
		for ( auto &p : m_players )
			p.reset( );
	}

	auto *players_hook( const int index ) {
		return &m_players[ index ];
	}
};
