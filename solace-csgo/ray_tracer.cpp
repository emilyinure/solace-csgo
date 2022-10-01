#include "includes.h"
#include "ray_tracer.h"
#include <cassert>
#include <cfloat>

RayTracer::Ray::Ray( const vec3_t &direction ) : m_startPoint( ), m_direction( direction ), m_length( 0.f ) { }

RayTracer::Ray::Ray( const vec3_t &startPoint, const vec3_t &endPoint ) : m_startPoint( startPoint ) {
	assert( startPoint != endPoint );

	const auto vectorDiff = endPoint - startPoint;
	m_direction = vectorDiff;
	m_length = m_direction.normalize( );
}

RayTracer::Hitbox::Hitbox( ) : m_mins( ), m_maxs( ), m_radius( ) { }

RayTracer::Hitbox::Hitbox( const vec3_t &mins, const vec3_t &maxs, const float radius ) : m_mins( mins ), m_maxs( maxs ), m_radius( radius ) {
	m_len = ( maxs - mins ).length( );
}

RayTracer::Hitbox::Hitbox( const std::tuple<vec3_t, vec3_t, float> &initTuple ) : m_mins( std::get<0>( initTuple ) ), m_maxs( std::get<1>( initTuple ) ), m_radius( std::get<2>( initTuple ) ) { }

RayTracer::Trace::Trace( ) : m_hit( false ), m_fraction( 0.f ), m_traceEnd( ) { }

void RayTracer::TraceHitbox( const Ray &ray, const Hitbox &hitbox, Trace &trace, int flags ) {
	assert( ray.m_direction.length( ) > 0.f );

	trace.m_fraction = 1.f;

	// we are treating the cylinder as a cylinder y^2+z^2=r^2, in the x-direction, so we will make it the x direction
	vec3_t unitDesired( 1.f, 0.f, 0.f );
	matrix_t identityMatrix(
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f
	);

	auto center = ( hitbox.m_mins + hitbox.m_maxs ) / 2.f;

	auto minsOffset = hitbox.m_mins - center;
	auto maxsOffset = hitbox.m_maxs - center;

	auto vecHitbox = maxsOffset - minsOffset;
	auto hitboxLength = hitbox.m_len;

	// now we calculate the transformation matrix to get our normalized hitbox to center
	auto unitHitbox = vecHitbox / hitboxLength;

	// dot == cos because they are both unit vectors
	auto dot = unitHitbox.dot( unitDesired );
	auto cross = unitHitbox.cross( unitDesired );

	vec3_t rotatedDirection;
	vec3_t rotatedStart;

	// offset the position
	auto adjustedStart = ray.m_startPoint - center;

	// if cross is 0, then we don't have to rotate because they are parallel
	if ( cross.length( ) > 0.f ) {
		matrix_t crossMatrix(
			0.f, -cross.z, cross.y, 0.f,
			cross.z, 0.f, -cross.x, 0.f,
			-cross.y, cross.x, 0.f, 0.f
		);

		auto rotationMatrix = identityMatrix + crossMatrix +
			( crossMatrix * crossMatrix ) * ( 1.f / ( 1.f + dot ) );

		math::VectorRotate(ray.m_direction, rotationMatrix, rotatedDirection);
		math::VectorRotate( adjustedStart, rotationMatrix, rotatedStart );
	} else {
		// cross is 0, they are parallel, if dot is 1.f they are same, else they are opposite
		if ( dot > 0.f ) {
			rotatedDirection = ray.m_direction;
			rotatedStart = adjustedStart;
		} else {
			rotatedDirection = -ray.m_direction;
			rotatedStart = -adjustedStart;
		}
	}

	auto a_c = rotatedDirection.y * rotatedDirection.y +
		rotatedDirection.z * rotatedDirection.z;

	/// TODO: Detect the plane!
	// this is INCREDIBLY RARE
	if ( a_c == 0.f ) {
		// the ray goes through both caps, easy case because we don't actually need to trace the ray because they are circles
		if ( rotatedDirection.x > 0 ) {
			// through the right cap, scale by radius and call it a day
			auto newLength = minsOffset.length( ) + hitbox.m_radius;
			auto offset = ( minsOffset * ( newLength ) / minsOffset.length( ) );

			if ( flags & Flags_RETURNEND )
				trace.m_traceEnd = center + offset;
			if ( flags & Flags_RETURNOFFSET )
				trace.m_traceOffset = offset;
		} else {
			// through the left cap, scale by radius again
			auto newLength = maxsOffset.length( ) + hitbox.m_radius;
			auto offset = ( maxsOffset * ( newLength ) / maxsOffset.length( ) );

			if ( flags & Flags_RETURNEND )
				trace.m_traceEnd = center + offset;
			if ( flags & Flags_RETURNOFFSET )
				trace.m_traceOffset = offset;
		}
		trace.m_hit = true;
		return;
	}

	constexpr auto a_s = 1.f;

	auto minsAdjusted = -hitboxLength / 2.f;
	auto maxsAdjusted = hitboxLength / 2.f;

	auto minsStart = vec3_t( rotatedStart.x - minsAdjusted, rotatedStart.y, rotatedStart.z );
	auto maxsStart = vec3_t( rotatedStart.x - maxsAdjusted, rotatedStart.y, rotatedStart.z );

	auto b_c = 2.f *
		( rotatedStart.y * rotatedDirection.y +
		  rotatedStart.z * rotatedDirection.z );

	auto b_smins = 2.f * ( minsStart.dot( rotatedDirection ) );
	auto b_smaxs = 2.f * ( maxsStart.dot( rotatedDirection ) );

	auto c_c = rotatedStart.y * rotatedStart.y +
		rotatedStart.z * rotatedStart.z -
		hitbox.m_radius * hitbox.m_radius;

	auto c_smins = minsStart.dot( minsStart ) -
		hitbox.m_radius * hitbox.m_radius;
	auto c_smaxs = maxsStart.dot( maxsStart ) -
		hitbox.m_radius * hitbox.m_radius;

	auto cylOperand = b_c * b_c - 4 * a_c * c_c;
	auto sphMinsOperand = b_smins * b_smins - 4 * a_s * c_smins;
	auto sphMaxsOperand = b_smaxs * b_smaxs - 4 * a_s * c_smaxs;

	auto tCyl = 0.f;
	auto tSphMins = 0.f;
	auto tSphMaxs = 0.f;

	auto cylHit = false;
	auto sphMinsHit = false;
	auto sphMaxsHit = false;

	// if we don't hit, operand is negative
	if ( cylOperand > 0.f ) {
		tCyl = ( -b_c - sqrtf( cylOperand ) ) / ( 2.f * a_c );

		if ( tCyl - FLT_EPSILON > 0.f ) {
			// make sure we hit within our bounds, and not outside of the cylinder's bore
			auto virtualPos = rotatedDirection * tCyl;

			auto outOfMinsSide = virtualPos.x < minsAdjusted;
			auto outOfMaxsSide = virtualPos.x > maxsAdjusted;

			if ( !outOfMinsSide &&
				 !outOfMaxsSide )
				cylHit = true;
		}
	}

	if ( sphMinsOperand > 0.f ) {
		tSphMins = ( -b_smins - sqrtf( sphMinsOperand ) ) / ( 2.f * a_s );

		if ( tSphMins - FLT_EPSILON > 0.f )
			sphMinsHit = true;
	}

	if ( sphMaxsOperand > 0.f ) {
		tSphMaxs = ( -b_smaxs - sqrtf( sphMaxsOperand ) ) / ( 2.f * a_s );

		if ( tSphMaxs - FLT_EPSILON > 0.f )
			sphMaxsHit = true;
	}

	// see which one hit first, then return accordingly
	if ( cylHit ) {
		trace.m_fraction = tCyl / ray.m_length;
		trace.m_hit = true;
	} else if ( sphMinsHit ) {
		trace.m_fraction = tSphMins / ray.m_length;
		trace.m_hit = true;
	} else if ( sphMaxsHit ) {
		trace.m_fraction = tSphMaxs / ray.m_length;
		trace.m_hit = true;
	}
}