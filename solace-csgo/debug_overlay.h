#pragma once
#include "includes.h"
#include "vec3.h"

class debug_overlay_t {
public:
	using world_to_screen_t = int( __thiscall * )( debug_overlay_t *, const vec3_t &, vec3_t & );
	bool world_to_screen( const vec3_t &in, vec3_t &out ) {
		auto return_value = ( *( world_to_screen_t ** )this )[ 13 ]( this, in, out );
		return static_cast< bool >( return_value != 1 );
	}

	using screen_position_t = bool( __thiscall * )( debug_overlay_t *, const vec3_t &, vec3_t & );
	bool screen_position( const vec3_t &in, vec3_t &out ) {
		return ( *( screen_position_t ** )this )[ 11 ]( this, std::ref( in ), std::ref( out ) );
	}

	using AddLineOverlay_t = void( __thiscall * )( debug_overlay_t *, const vec3_t &origin, const vec3_t &dest, int r, int g, int b, bool noDepthTest, float duration );
	void AddLineOverlay( const vec3_t &origin, const vec3_t &dest, int r, int g, int b, bool noDepthTest, float duration ) {
		return util::get_virtual_function< AddLineOverlay_t >( this, 5 )( this, origin, dest, r, g, b, noDepthTest, duration );
	}
	using AddBoxOverlay_t = void( __thiscall * )( void *, vec3_t const &start, vec3_t const &min, vec3_t const &max, ang_t const &angle, int r, int g, int b, int a, float duration );
	void AddBoxOverlay( vec3_t const &start, vec3_t const &min, vec3_t const &max, ang_t const &angle, int r, int g, int b, int a, float duration ) {
		return ( *( AddBoxOverlay_t ** )this )[ 1 ]( this, start, min, max, angle, r, g, b, a, duration );
	}
	using original_fn = void( __thiscall * )( void *, vec3_t const &start, vec3_t const &end, vec3_t const &min, vec3_t const &max, ang_t const &angle, int r, int g, int b, int a, float duration );
	void AddSweptBoxOverlay( vec3_t const &start, vec3_t const &end, vec3_t const &min, vec3_t const &max, ang_t const &angle, int r, int g, int b, int a, float duration ) {
		return ( *( original_fn ** )this )[ 9 ]( this, start, end, min, max, angle, r, g, b, a, duration );
	}
	using AddCapsuleOverlay_t = void( __thiscall * )( void *, vec3_t const &, vec3_t const &, float const &, int, int, int, int, float );
	void AddCapsuleOverlay( vec3_t const &start, vec3_t const &end, float const &radius, int r, int g, int b, int a, float duration ) {
		return ( *( AddCapsuleOverlay_t ** )this )[ 24 ]( this, start, end, radius, r, g, b, a, duration );
	}
};

class panel_t {
public:
	enum indices : size_t {
		GETNAME = 36,
		PAINTTRAVERSE = 41,
	};

public:
	__forceinline const char *GetName( long vgui_panel ) {
		return util::get_virtual_function< const char *( __thiscall * )( decltype( this ), uint32_t ) >( this, GETNAME )( this, vgui_panel );
	}
};