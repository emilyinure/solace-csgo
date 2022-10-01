#include "vec3.h"

#include <cmath>

#include "math_funcs.h"

void ang_t::vectors( vec3_t *forward, vec3_t *right, vec3_t *up ) const {
	float sp, sy, sr, cp, cy, cr;

	math::sin_cos( DEG2RAD( x ), &sp, &cp );
	math::sin_cos( DEG2RAD( y ), &sy, &cy );
	math::sin_cos( DEG2RAD( z ), &sr, &cr );

	if ( forward ) {
		forward->x = cp * cy;
		forward->y = cp * sy;
		forward->z = -sp;
	}

	if ( right ) {
		right->x = -1 * sr * sp * cy + -1 * cr * -sy;
		right->y = -1 * sr * sp * sy + -1 * cr * cy;
		right->z = -1 * sr * cp;
	}

	if ( up ) {
		up->x = cr * sp * cy + -sr * -sy;
		up->y = cr * sp * sy + -sr * cy;
		up->z = cr * cp;
	}
}

vec3_t ang_t::forward( ) const {
	vec3_t forward;

	const auto sy = sin( DEG2RAD( y ) );
	const auto sp = sin( DEG2RAD( x ) );
	const auto cy = cos( DEG2RAD( y ) );
	const auto cp = cos( DEG2RAD( x ) );

	forward.x = cp * cy;
	forward.y = cp * sy;
	forward.z = -sp;
	return forward;
}

vec3_t::vec3_t( void ) {
	x = y = z = 0.0f;
}

vec3_t::vec3_t( float _x, float _y, float _z ) {
	x = _x;
	y = _y;
	z = _z;
}

vec3_t::~vec3_t( void ) { }

matrix_t matrix_t::operator* ( matrix_t other ) const {
	matrix_t ret;
	math::ConcatTransforms( *this, other, &ret );
	return ret;
}
matrix_t matrix_t::operator* ( float other ) const {
	matrix_t ret;
	for ( int i = 0; i < 3; i++ ) {
		for ( int j = 0; j < 4; j++ ) {
			ret[i][j] = other * (*this)[ i ][ j ];
		}
	}
	return ret;
}

matrix_t matrix_t::operator+ ( matrix_t other ) const {
	matrix_t ret;
	for ( int i = 0; i < 3; i++ ) {
		for ( int j = 0; j < 4; j++ ) {
			ret[ i ][ j ] = (*this)[ i ][ j ] + other[ i ][ j ];
		}
	}
	return ret;
};
