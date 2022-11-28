#include "events.h"

#include "resolver.h"

#include "includes.h"

class hurt_listener final : IGameEventListener2 {
	void FireGameEvent( IGameEvent *evt ) override {
		g_resolver.OnHurt( evt );
	}
public:
	void init( IGameEventManager2 *game_events ) {
		game_events_ = ( game_events );
		game_events_->AddListener( this, "player_hurt", false );
	}
	~hurt_listener( ) {
		game_events_->RemoveListener( this );
	}
private:
	IGameEventManager2 *game_events_;
} hurt_listener;

class impact_listener final : IGameEventListener2 {
	void FireGameEvent( IGameEvent *evt ) override {
		g_resolver.on_impact( evt );
	}
public:
	void init( IGameEventManager2 *game_events ) {
		game_events_ = ( game_events );
		game_events_->AddListener( this, "bullet_impact", false );
	}
	~impact_listener( ) {
		game_events_->RemoveListener( this );
	}
private:
	IGameEventManager2 *game_events_;
} impact_listener;

class start_listener final : public IGameEventListener2 {
	void FireGameEvent( IGameEvent *evt ) override {
		g_resolver.clear( );
		g.m_valid_round = true;
	}
public:
	void init( IGameEventManager2 *game_events ) {
		game_events_ = ( game_events );
		game_events_->AddListener( this, "round_start", false );
	}
	~start_listener( ) {
		game_events_->RemoveListener( this );
	}
private:
	IGameEventManager2 *game_events_;
} start_listener;

class end_listener final : public IGameEventListener2 {
	void FireGameEvent( IGameEvent *evt ) override {
		g.m_valid_round = false;
		g.m_cmds.clear( );
	}
public:
	void init( IGameEventManager2* game_events ) {
		game_events_ = ( game_events );
		game_events_->AddListener( this, "round_end", false );
	}
	~end_listener( ) {
		game_events_->RemoveListener( this );
	}
private:
	IGameEventManager2 *game_events_;
} end_listener;

void events::init( ) {
	hurt_listener.init( g.m_interfaces->events( ) );
	impact_listener.init( g.m_interfaces->events( ) );
	start_listener.init( g.m_interfaces->events( ) );
	end_listener.init( g.m_interfaces->events( ) );
}

void events::destroy( ) {
	hurt_listener.~hurt_listener( );
	impact_listener.~impact_listener( );
	start_listener.~start_listener( );
	end_listener.~end_listener( );
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
