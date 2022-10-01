#pragma once
#include <tuple>

#include "vec3.h"

/*
	RayTracer provides the same functionality as ClipRayToEntity, only
	it allows to trace for a specific hitbox through all others and provides
	exceptional performance compared to the game's RayTracing engine due to its
	inherent specialization
*/

class RayTracer {
public:
	struct Ray {
		Ray( const vec3_t &direction );
		Ray( const vec3_t &startPoint, const vec3_t &endPoint );
		vec3_t m_startPoint;
		vec3_t m_direction;
		float m_length;
	};

	struct Hitbox {
		Hitbox( );
		Hitbox( const vec3_t &mins, const vec3_t &maxs, const float radius );
		Hitbox( const std::tuple<vec3_t, vec3_t, float> &initTuple );
		vec3_t m_mins;
		vec3_t m_maxs;
		float m_radius;
		float m_len;
	};

	struct Trace {
		Trace( );
		bool m_hit;
		float m_fraction;
		vec3_t m_traceEnd;
		vec3_t m_traceOffset;
	};

	enum Flags {
		Flags_NONE = 0,
		Flags_RETURNEND = ( 1 << 0 ),
		Flags_RETURNOFFSET = ( 1 << 1 )
	};

	// This is a specialization that starts from the center, as calculations are much simpler from the center of the hitbox
	static void TraceFromCenter( const Ray &ray, const Hitbox &hitbox, Trace &trace, int flags = 0 );
	// This is for the general case, tracing against the hitbox
	static void TraceHitbox( const Ray &ray, const Hitbox &hitbox, Trace &trace, int flags = 0 );
};