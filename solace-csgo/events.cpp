#include "events.h"

#include "resolver.h"

#include "includes.h"

class hurt_listener final : IGameEventListener2 {
	void FireGameEvent( IGameEvent *evt ) override {
		g_resolver.OnHurt( evt );
	}
public:
	explicit hurt_listener( IGameEventManager2 *game_events ) : game_events_( game_events ) {
		game_events_->AddListener( this, "player_hurt", false );
	}
private:
	IGameEventManager2 *game_events_;
} *hurt_listener;

class impact_listener final : IGameEventListener2 {
	void FireGameEvent( IGameEvent *evt ) override {
		g_resolver.on_impact( evt );
	}
public:
	explicit impact_listener( IGameEventManager2 *game_events ) : game_events_( game_events ) {
		game_events_->AddListener( this, "bullet_impact", false );
	}
private:
	IGameEventManager2 *game_events_;
} *impact_listener;

class start_listener final : public IGameEventListener2 {
	void FireGameEvent( IGameEvent *evt ) override {
		g_resolver.clear( );
		g.m_valid_round = true;
	}
public:
	explicit start_listener( IGameEventManager2 *game_events ) : game_events_( game_events ) {
		game_events_->AddListener( this, "round_start", false );
	}
private:
	IGameEventManager2 *game_events_;
} *start_listener;
class end_listener final : public IGameEventListener2 {
	void FireGameEvent( IGameEvent *evt ) override {
		g.m_valid_round = false;
		g.m_cmds.clear( );
	}
public:
	explicit end_listener( IGameEventManager2 *game_events ) : game_events_( game_events ) {
		game_events_->AddListener( this, "round_end", false );
	}
private:
	IGameEventManager2 *game_events_;
} *end_listener;
void events::init ( ) {
	hurt_listener = new class hurt_listener( g.m_interfaces->events( ) );
	impact_listener = new class impact_listener( g.m_interfaces->events( ) );
	start_listener = new class start_listener( g.m_interfaces->events( ) );
	end_listener = new class end_listener( g.m_interfaces->events( ) );
}

void events::player_hurt( IGameEvent *evt ) {
	g_resolver.OnHurt( evt );
}

void events::bullet_impact( IGameEvent *evt ) {
	g_resolver.on_impact( evt );
}

void events::round_start( IGameEvent *evt ) {
	g_resolver.clear( );
	g.m_valid_round = true;
}

void events::round_end ( IGameEvent *evt ) {
	g.m_valid_round = false;
}
