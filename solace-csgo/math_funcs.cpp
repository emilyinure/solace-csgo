#include "math_funcs.h"
#include "includes.h"
#include "view_matrix.hpp"
#include "sdk.h"

bool math::world_to_screen( const vec3_t &origin, vec3_t &screen ) {
	static std::uintptr_t view_matrix;
	if ( !view_matrix )
		view_matrix = *reinterpret_cast< std::uintptr_t * >( reinterpret_cast< std::uintptr_t >( util::find( "client.dll", "0F 10 05 ? ? ? ? 8D 85 ? ? ? ? B9" ) ) + 3 ) + 176;

	const auto &matrix = *reinterpret_cast< view_matrix_t * >( view_matrix );

	const auto w = matrix.m[ 3 ][ 0 ] * origin.x + matrix.m[ 3 ][ 1 ] * origin.y + matrix.m[ 3 ][ 2 ] * origin.z + matrix.m[ 3 ][ 3 ];
	if ( w < 0.001f )
		return false;

	int x, y;
	g.m_interfaces->engine( )->get_screen_size( x, y );

	screen.x = static_cast< float >( x ) / 2.0f;
	screen.y = static_cast< float >( y ) / 2.0f;

	screen.x *= 1.0f + ( matrix.m[ 0 ][ 0 ] * origin.x + matrix.m[ 0 ][ 1 ] * origin.y + matrix.m[ 0 ][ 2 ] * origin.z + matrix.m[ 0 ][ 3 ] ) / w;
	screen.y *= 1.0f - ( matrix.m[ 1 ][ 0 ] * origin.x + matrix.m[ 1 ][ 1 ] * origin.y + matrix.m[ 1 ][ 2 ] * origin.z + matrix.m[ 1 ][ 3 ] ) / w;

	return true;
}
float math::distSegmentToSegment(
	const vec3_t &p1, const vec3_t &p2,
	const vec3_t &q1, const vec3_t &q2,
	float &invariant1, float &invariant2 ) {

	static const auto kSmallNumber = 0.0001f;
	const auto u = p2 - p1;
	const auto v = q2 - q1;
	const auto w = p1 - q1;
	const auto a = u.dot( u );
	const auto b = u.dot( v );
	const auto c = v.dot( v );
	const auto d = u.dot( w );
	const auto e = v.dot( w );
	const auto f = a * c - b * b;
	// s1,s2 and t1,t2 are the parametric representation of the intersection.
		// they will be the invariants at the end of this simple computation.
	float s1;
	auto s2 = f;
	float t1;
	auto t2 = f;

	if ( f < kSmallNumber ) {
		s1 = 0.0;
		s2 = 1.0;
		t1 = e;
		t2 = c;

	} else {
		s1 = ( b * e - c * d );
		t1 = ( a * e - b * d );
		if ( s1 < 0.0 ) {
			s1 = 0.0;
			t1 = e;
			t2 = c;

		} else if ( s1 > s2 ) {
			s1 = s2;
			t1 = e + b;
			t2 = c;

		}

	}

	if ( t1 < 0.0f ) {
		t1 = 0.0f;
		if ( -d < 0.0f )
			s1 = 0.0f;
		else if ( -d > a )
			s1 = s2;
		else {
			s1 = -d;
			s2 = a;

		}

	} else if ( t1 > t2 ) {
		t1 = t2;
		if ( ( -d + b ) < 0.0f )
			s1 = 0;
		else if ( ( -d + b ) > a )
			s1 = s2;
		else {
			s1 = ( -d + b );
			s2 = a;
		}
	}
	invariant1 = ( (std::abs( s1 ) < kSmallNumber) ? 0.0f : s1 / s2 );
	invariant2 = ( std::abs( t1 ) < kSmallNumber ? 0.0f : t1 / t2 );

	return ( w + ( u * invariant1 ) - ( v * invariant2 ) ).length( );

}
void math::correct_movement( cmd_t *cmd ) {
	vec3_t wish_forward, wish_right, wish_up, cmd_forward, cmd_right, cmd_up;

	auto movedata = vec3_t( cmd->m_forwardmove, cmd->m_sidemove, cmd->m_upmove );

	if ( !( g.m_local->flags( ) & fl_onground ) && cmd->m_viewangles.z != 0.f )
		movedata.y = 0.f;

	g.m_view_angles.vectors( &wish_forward, &wish_right, &wish_up );
	cmd->m_viewangles.vectors( &cmd_forward, &cmd_right, &cmd_up );

	const auto v8 = sqrt( wish_forward.x * wish_forward.x + wish_forward.y * wish_forward.y ), v10 = sqrt(
		           wish_right.x * wish_right.x + wish_right.y * wish_right.y ), v12 = sqrt( wish_up.z * wish_up.z );

	const vec3_t wish_forward_norm( 1.0f / v8 * wish_forward.x, 1.0f / v8 * wish_forward.y, 0.f ),
		wish_right_norm( 1.0f / v10 * wish_right.x, 1.0f / v10 * wish_right.y, 0.f ),
		wish_up_norm( 0.f, 0.f, 1.0f / v12 * wish_up.z );

	const auto v14 = sqrt( cmd_forward.x * cmd_forward.x + cmd_forward.y * cmd_forward.y ), v16 = sqrt(
		cmd_right.x * cmd_right.x + cmd_right.y * cmd_right.y ), v18 = sqrt( cmd_up.z * cmd_up.z );

	const vec3_t cmd_forward_norm( 1.0f / v14 * cmd_forward.x, 1.0f / v14 * cmd_forward.y, 1.0f / v14 * 0.0f ),
		cmd_right_norm( 1.0f / v16 * cmd_right.x, 1.0f / v16 * cmd_right.y, 1.0f / v16 * 0.0f ),
		cmd_up_norm( 0.f, 0.f, 1.0f / v18 * cmd_up.z );

	const auto v22 = wish_forward_norm.x * movedata.x, v26 = wish_forward_norm.y * movedata.x, v28 =
		wish_forward_norm.z * movedata.x, v24 = wish_right_norm.x * movedata.y, v23 =
		wish_right_norm.y * movedata.y, v25 = wish_right_norm.z * movedata.y, v30 =
		wish_up_norm.x * movedata.z, v27 = wish_up_norm.z * movedata.z, v29 =
		wish_up_norm.y * movedata.z;

	vec3_t correct_movement;
	correct_movement.x = cmd_forward_norm.x * v24 + cmd_forward_norm.y * v23 + cmd_forward_norm.z * v25
		+ ( cmd_forward_norm.x * v22 + cmd_forward_norm.y * v26 + cmd_forward_norm.z * v28 )
		+ ( cmd_forward_norm.y * v30 + cmd_forward_norm.x * v29 + cmd_forward_norm.z * v27 );
	correct_movement.y = cmd_right_norm.x * v24 + cmd_right_norm.y * v23 + cmd_right_norm.z * v25
		+ ( cmd_right_norm.x * v22 + cmd_right_norm.y * v26 + cmd_right_norm.z * v28 )
		+ ( cmd_right_norm.x * v29 + cmd_right_norm.y * v30 + cmd_right_norm.z * v27 );
	correct_movement.z = cmd_up_norm.x * v23 + cmd_up_norm.y * v24 + cmd_up_norm.z * v25
		+ ( cmd_up_norm.x * v26 + cmd_up_norm.y * v22 + cmd_up_norm.z * v28 )
		+ ( cmd_up_norm.x * v30 + cmd_up_norm.y * v29 + cmd_up_norm.z * v27 );

	cmd->m_forwardmove = std::clamp( correct_movement.x, -450.f, 450.f );
	cmd->m_sidemove = std::clamp( correct_movement.y, -450.f, 450.f );
	cmd->m_upmove = std::clamp( correct_movement.z, -320.f, 320.f );
}

void math::sin_cos ( float r, float *s, float *c ) {
	*s = std::sin( r );
	*c = std::cos( r );
}





math::custom_ray::custom_ray ( vec3_t start, vec3_t end ) {
	m_start = start;
	m_end = end;
	m_ray_dir = (m_end - m_start).normalized( );
}

vec3_t math::get_closest_on_line ( vec3_t start, vec3_t end, vec3_t target ) {
	const auto line_delta = end - start;
	const auto target_delta = target - start;
	const auto line_length_sqr = line_delta.length_sqr( );
	auto fraction = (target_delta.length_sqr( )) / line_length_sqr;
	if ( fraction < 0 )
		fraction = 0;
	if ( fraction > 1 )
		fraction = 1;
	return start + line_delta * fraction;
}

vec3_t math::closest_to_point ( const custom_ray &ray, const vec3_t &point ) {
	const auto delta = point - ray.m_start;
	const auto magnitude = delta.dot( ray.m_ray_dir );

	return ray.m_ray_dir * magnitude + ray.m_start;
}
float math::dist_Segment_to_Segment( const custom_ray &ray, const vec3_t &min, const vec3_t &max ) {
	const auto u = max - min;
	const auto v = ray.m_end - ray.m_start;
	const auto w = min - ray.m_start;
	const auto a = u.dot( u );
	const auto b = u.dot( v );
	const auto c = v.dot( v );
	const auto d = u.dot( w );
	const auto e = v.dot( w );
	const auto D = a * c - b * b;
	float    sc, sN, sD = D;
	float    tc, tN, tD = D;

	if ( D < FLT_MIN ) {
		sN = 0.0;
		sD = 1.0;
		tN = e;
		tD = c;
	} else {
		sN = ( b * e - c * d );
		tN = ( a * e - b * d );
		if ( sN < 0.0 ) {
			sN = 0.0;
			tN = e;
			tD = c;
		} else if ( sN > sD ) {
			sN = sD;
			tN = e + b;
			tD = c;
		}
	}

	if ( tN < 0.0 ) {
		tN = 0.0;

		if ( -d < 0.0 )
			sN = 0.0;
		else if ( -d > a )
			sN = sD;
		else {
			sN = -d;
			sD = a;
		}
	} else if ( tN > tD ) {
		tN = tD;

		if ( ( -d + b ) < 0.0 )
			sN = 0;
		else if ( ( -d + b ) > a )
			sN = sD;
		else {
			sN = ( -d + b );
			sD = a;
		}
	}

	sc = ( abs( sN ) < FLT_MIN ? 0.0 : sN / sD );
	tc = ( abs( tN ) < FLT_MIN ? 0.0 : tN / tD );

	const auto dP = w + ( u * sc ) - ( v * tc );

	return dP.length(  );
}
bool math::intersects_capsule ( const custom_ray &ray, const vec3_t &min, const vec3_t &max,
                                const float &radius ) {
	const auto RC = ray.m_start - min;
	const auto axis = max - min;
	auto n = ray.m_ray_dir.cross( axis );
	float d;
	vec3_t D;
	const auto ln = n.normalize( );
	if ( ln == 0 ) {	/* ray parallel to cyl	*/
		d = RC.dot( axis );
		D.x = RC.x - d * axis.x;
		D.y = RC.y - d * axis.y;
		D.z = RC.z - d * axis.z;
		d = D.length(  );
		return ( d <= radius );		/* true if ray is in cyl*/
	}

	n.normalize(  );
	d = fabs( RC.dot(n) );		/* shortest distance	*/
	return ( d <= radius );
}

typedef float( *RandomFloat_t )( float, float );
typedef void( *RandomSeed_t )( UINT );
float math::RandomFloat( float fMin, float fMax ) {

	static RandomFloat_t m_RandomFloat;

	if ( !m_RandomFloat )
		m_RandomFloat = ( RandomFloat_t )GetProcAddress( GetModuleHandleA( static_cast< LPCSTR >("vstdlib.dll") ), "RandomFloat" );

	return m_RandomFloat( fMin, fMax );
}


float math::RandomInt( int fMin, int fMax ) {

	static RandomFloat_t m_RandomInt;

	if ( !m_RandomInt )
		m_RandomInt = ( RandomFloat_t )GetProcAddress( GetModuleHandleA( static_cast< LPCSTR >("vstdlib.dll") ), "RandomInt" );

	return m_RandomInt( fMin, fMax );
}

void math::RandomSeed( int Seed ) {

	static RandomSeed_t m_RandomSeed;

	if ( !m_RandomSeed )
		m_RandomSeed = ( RandomSeed_t )GetProcAddress( GetModuleHandleA( static_cast< LPCSTR >("vstdlib.dll") ), "RandomSeed" );

	m_RandomSeed( Seed );
}

vec3_t math::calculate_spread ( weapon_t *weapon, int seed, float inaccuracy, float spread, bool revolver ) {
	int item_def_index;
	float recoil_index, r1, r2, r3, r4, s1, c1, s2, c2;

	// if we have no bullets, we have no spread.
	item_def_index = weapon->item_definition_index( );
	const auto wep_info = g.m_interfaces->weapon_system( )->get_weapon_data( weapon->item_definition_index( ) );;
	if ( !wep_info || !wep_info->m_bullets )
		return {};

	// get some data for later.

	// seed randomseed.
	RandomSeed( ( seed & 0xff ) + 1 );

	// generate needed floats.
	r1 = RandomFloat( 0.f, 1.f );
	r2 = RandomFloat( 0.f, M_PI_2 );

	//if ( /*wep_info->m_weapon_type == WEAPONTYPE_SHOTGUN &&*/ g_csgo.weapon_accuracy_shotgun_spread_patterns->GetInt() > 0)
	//	g_csgo.GetShotgunSpread(item_def_index, 0, 0 /*bullet_i*/ + wep_info->m_bullets * recoil_index, &r4, &r3);


	r3 = RandomFloat( 0.f, 1.f );
	r4 = RandomFloat( 0.f, M_PI );

	// revolver secondary spread.
	if ( item_def_index == WEAPON_REVOLVER && revolver ) {
		r1 = 1.f - ( r1 * r1 );
		r3 = 1.f - ( r3 * r3 );
	}

	// get needed sine / cosine values.
	c1 = std::cos( r2 );
	c2 = std::cos( r4 );
	s1 = std::sin( r2 );
	s2 = std::sin( r4 );

	// calculate spread vector.
	return {
		( c1 * ( r1 * inaccuracy ) ) + ( c2 * ( r3 * spread ) ),
		( s1 * ( r1 * inaccuracy ) ) + ( s2 * ( r3 * spread ) ),
		0.f
	};
}

vec3_t math::get_bone_position ( matrix_t bone_matrices ) {
	return vec3_t{bone_matrices[0][3], bone_matrices[1][3], bone_matrices[2][3]};
}

void math::VectorRotate( vec3_t in1, matrix_t in2, vec3_t &out ) {
	out.x = in1.dot( in2[ 0 ] );
	out.y = in1.dot( in2[ 1 ] );
	out.z = in1.dot( in2[ 2 ] );
}

void math::VectorTransform( vec3_t in1, matrix_t &in2, vec3_t &out ) {
	out.x = in1.dot( in2[ 0 ] ) + in2[ 0 ][ 3 ];
	out.y = in1.dot( in2[ 1 ] ) + in2[ 1 ][ 3 ];
	out.z = in1.dot( in2[ 2 ] ) + in2[ 2 ][ 3 ];
}

void math::AngleMatrix ( const ang_t angles, matrix_t *matrix ) {
	auto SinCos = [ ]( float radians, float *sine, float *cosine ) {
		*sine = sin( radians );
		*cosine = cos( radians );
	};

	float sr, sp, sy, cr, cp, cy;

	SinCos( DEG2RAD( angles[ 1 ] ), &sy, &cy );
	SinCos( DEG2RAD( angles[ 0 ] ), &sp, &cp );
	SinCos( DEG2RAD( angles[ 2 ] ), &sr, &cr );

	// matrix = (YAW * PITCH) * ROLL
	matrix->mat_val[ 0 ][ 0 ] = cp * cy;
	matrix->mat_val[ 1 ][ 0 ] = cp * sy;
	matrix->mat_val[ 2 ][ 0 ] = -sp;

	const auto crcy = cr * cy;
	const auto crsy = cr * sy;
	const auto srcy = sr * cy;
	const auto srsy = sr * sy;
	matrix->mat_val[ 0 ][ 1 ] = sp * srcy - crsy;
	matrix->mat_val[ 1 ][ 1 ] = sp * srsy + crcy;
	matrix->mat_val[ 2 ][ 1 ] = sr * cp;

	matrix->mat_val[ 0 ][ 2 ] = sp * crcy + srsy;
	matrix->mat_val[ 1 ][ 2 ] = sp * crsy - srcy;
	matrix->mat_val[ 2 ][ 2 ] = cr * cp;

	matrix->mat_val[ 0 ][ 3 ] = 0.0f;
	matrix->mat_val[ 1 ][ 3 ] = 0.0f;
	matrix->mat_val[ 2 ][ 3 ] = 0.0f;
}

void math::angle_matrix ( const ang_t ang, const vec3_t &pos, matrix_t *out ) {
	AngleMatrix( ang, out );
	out->set_origin( pos );
}

void math::MatrixAngles( matrix_t &matrix, ang_t &angles ) {
	float forward[ 3 ];
	float left[ 3 ];
	float up[ 3 ];
	//
	// Extract the basis vectors from the matrix. Since we only need the Z
	// component of the up vector, we don't get X and Y.
	//
	forward[ 0 ] = matrix[ 0 ][ 0 ];
	forward[ 1 ] = matrix[ 1 ][ 0 ];
	forward[ 2 ] = matrix[ 2 ][ 0 ];
	left[ 0 ] = matrix[ 0 ][ 1 ];
	left[ 1 ] = matrix[ 1 ][ 1 ];
	left[ 2 ] = matrix[ 2 ][ 1 ];
	up[ 2 ] = matrix[ 2 ][ 2 ];

	const auto xyDist = sqrtf( forward[ 0 ] * forward[ 0 ] + forward[ 1 ] * forward[ 1 ] );

	// enough here to get angles?
	if ( xyDist > 0.001f ) {
		// (yaw)	y = ATAN( forward.y, forward.x );		-- in our space, forward is the X axis
		angles.y = RAD2DEG( atan2f( forward[ 1 ], forward[ 0 ] ) );

		// (pitch)	x = ATAN( -forward.z, sqrt(forward.x*forward.x+forward.y*forward.y) );
		angles.x = RAD2DEG( atan2f( -forward[ 2 ], xyDist ) );

		// (roll)	z = ATAN( left.z, up.z );
		angles.z = RAD2DEG( atan2f( left[ 2 ], up[ 2 ] ) );
	} else	// forward is mostly Z, gimbal lock-
	{
		// (yaw)	y = ATAN( -left.x, left.y );			-- forward is mostly z, so use right for yaw
		angles.y = RAD2DEG( atan2f( -left[ 0 ], left[ 1 ] ) );

		// (pitch)	x = ATAN( -forward.z, sqrt(forward.x*forward.x+forward.y*forward.y) );
		angles.x = RAD2DEG( atan2f( -forward[ 2 ], xyDist ) );

		// Assume no roll in this case as one degree of freedom has been lost (i.e. yaw == roll)
		angles.z = 0;
	}
}
void math::MatrixGetColumn( matrix_t &in, int column, vec3_t &out ) {
	out.x = in[ 0 ][ column ];
	out.y = in[ 1 ][ column ];
	out.z = in[ 2 ][ column ];
}
void math::MatrixSetColumn( vec3_t &in, int column, matrix_t &out ) {
	out[ 0 ][ column ] = in.x;
	out[ 1 ][ column ] = in.y;
	out[ 2 ][ column ] = in.z;
}
void math::VectorScale( const float *in, float scale, float *out ) {
	out[ 0 ] = in[ 0 ] * scale;
	out[ 1 ] = in[ 1 ] * scale;
	out[ 2 ] = in[ 2 ] * scale;
}
void math::MatrixAngles( matrix_t &matrix, ang_t &angles, vec3_t &position ) {
	MatrixGetColumn( matrix, 3, position );
	MatrixAngles( matrix, angles );
}

void MatrixCopy( matrix_t *src, matrix_t *dst ) {
	if ( src != dst ) {
		memcpy( dst, src, sizeof( matrix_t ) );
	}
}

void math::QuaternionMatrix( const quaternion_t &q, matrix_t *matrix ) {
	matrix->mat_val[ 0 ][ 0 ] = 1.0 - 2.0 * q.y * q.y - 2.0 * q.z * q.z;
	matrix->mat_val[ 1 ][ 0 ] = 2.0 * q.x * q.y + 2.0 * q.w * q.z;
	matrix->mat_val[ 2 ][ 0 ] = 2.0 * q.x * q.z - 2.0 * q.w * q.y;
	
	matrix->mat_val[ 0 ][ 1 ] = 2.0f * q.x * q.y - 2.0f * q.w * q.z;
	matrix->mat_val[ 1 ][ 1 ] = 1.0f - 2.0f * q.x * q.x - 2.0f * q.z * q.z;
	matrix->mat_val[ 2 ][ 1 ] = 2.0f * q.y * q.z + 2.0f * q.w * q.x;
	
	matrix->mat_val[ 0 ][ 2 ] = 2.0f * q.x * q.z + 2.0f * q.w * q.y;
	matrix->mat_val[ 1 ][ 2 ] = 2.0f * q.y * q.z - 2.0f * q.w * q.x;
	matrix->mat_val[ 2 ][ 2 ] = 1.0f - 2.0f * q.x * q.x - 2.0f * q.y * q.y;
	
	matrix->mat_val[ 0 ][ 3 ] = 0.0f;
	matrix->mat_val[ 1 ][ 3 ] = 0.0f;
	matrix->mat_val[ 2 ][ 3 ] = 0.0f;
}

void math::QuaternionMatrix ( const quaternion_t &q, const vec3_t &pos, matrix_t *matrix ) {

	QuaternionMatrix( q, matrix );

	matrix->mat_val[0][3] = pos.x;
	matrix->mat_val[1][3] = pos.y;
	matrix->mat_val[2][3] = pos.z;
}

void math::ConcatTransforms ( matrix_t in1, matrix_t in2, matrix_t *out ) {
	if ( &in1 == out ) {
		matrix_t in1b;
		MatrixCopy( &in1, &in1b );
		ConcatTransforms( in1b, in2, out );
		return;
	}
	if ( &in2 == out ) {
		matrix_t in2b;
		MatrixCopy( &in2, &in2b );
		ConcatTransforms( in1, in2b, out );
		return;
	}
	out->mat_val[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] +
		in1[0][2] * in2[2][0];
	out->mat_val[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] +
		in1[0][2] * in2[2][1];
	out->mat_val[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] +
		in1[0][2] * in2[2][2];
	out->mat_val[0][3] = in1[0][0] * in2[0][3] + in1[0][1] * in2[1][3] +
		in1[0][2] * in2[2][3] + in1[0][3];
	out->mat_val[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] +
		in1[1][2] * in2[2][0];
	out->mat_val[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] +
		in1[1][2] * in2[2][1];
	out->mat_val[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] +
		in1[1][2] * in2[2][2];
	out->mat_val[1][3] = in1[1][0] * in2[0][3] + in1[1][1] * in2[1][3] +
		in1[1][2] * in2[2][3] + in1[1][3];
	out->mat_val[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] +
		in1[2][2] * in2[2][0];
	out->mat_val[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] +
		in1[2][2] * in2[2][1];
	out->mat_val[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] +
		in1[2][2] * in2[2][2];
	out->mat_val[2][3] = in1[2][0] * in2[0][3] + in1[2][1] * in2[1][3] +
		in1[2][2] * in2[2][3] + in1[2][3];
}

float math::get_fov ( const ang_t &view_angles, const vec3_t &start, const vec3_t &end ) {
	vec3_t dir, fw;

	// get direction and normalize.
	dir = (end - start).normalized( );

	// get the forward direction vector of the view angles.
	fw = view_angles.forward();

	// get the angle between the view angles forward directional vector and the target location.
	return max( RAD2DEG( std::acos( fw.dot( dir ) ) ), 0.f );
}
bool math::IntersectRayWithBox( const vec3_t &start, const vec3_t &delta, const vec3_t &mins, const vec3_t &maxs, float tolerance, custom_ray *out_info ) {
	int   i;
	float d1, d2, f;

	for ( i = 0; i < 6; ++i ) {
		if ( i >= 3 ) {
			d1 = start[ i - 3 ] - maxs[ i - 3 ];
			d2 = d1 + delta[ i - 3 ];
		}

		else {
			d1 = -start[ i ] + mins[ i ];
			d2 = d1 - delta[ i ];
		}

		// if completely in front of face, no intersection.
		if ( d1 > 0.f && d2 > 0.f ) {
			out_info->m_startsolid = false;

			return false;
		}

		// completely inside, check next face.
		if ( d1 <= 0.f && d2 <= 0.f )
			continue;

		if ( d1 > 0.f )
			out_info->m_startsolid = false;

		// crosses face.
		if ( d1 > d2 ) {
			f = max( 0.f, d1 - tolerance );

			f = f / ( d1 - d2 );
			if ( f > out_info->m_t1 ) {
				out_info->m_t1 = f;
				out_info->m_hitside = i;
			}
		}

		// leave.
		else {
			f = ( d1 + tolerance ) / ( d1 - d2 );
			if ( f < out_info->m_t2 )
				out_info->m_t2 = f;
		}
	}

	return out_info->m_startsolid || ( out_info->m_t1 < out_info->m_t2 &&out_info->m_t1 >= 0.f );
}

bool math::IntersectRayWithBox( const vec3_t &start, const vec3_t &delta, const vec3_t &mins, const vec3_t &maxs, float tolerance, trace_t *out_tr, float *fraction_left_solid ) {
	custom_ray box_tr;

	// note - dex; this is Collision_ClearTrace.
	out_tr->start = start;
	out_tr->end = start;
	out_tr->end += delta;
	out_tr->startSolid = false;
	out_tr->allsolid = false;
	out_tr->flFraction = 1.f;
	out_tr->contents = 0;

	if ( IntersectRayWithBox( start, delta, mins, maxs, tolerance, &box_tr ) ) {
		out_tr->startSolid = box_tr.m_startsolid;

		if ( box_tr.m_t1 < box_tr.m_t2 && box_tr.m_t1 >= 0.f ) {
			out_tr->flFraction = box_tr.m_t1;

			// VectorMA( pTrace->startpos, trace.t1, vecRayDelta, pTrace->endpos );

			out_tr->contents = CONTENTS_SOLID;
			out_tr->plane.normal = vec3_t{};

			if ( box_tr.m_hitside >= 3 ) {
				box_tr.m_hitside -= 3;

				out_tr->plane.m_dist = maxs[ box_tr.m_hitside ];
				out_tr->plane.normal[ box_tr.m_hitside ] = 1.f;
				out_tr->plane.m_type = box_tr.m_hitside;
			}

			else {
				out_tr->plane.m_dist = -mins[ box_tr.m_hitside ];
				out_tr->plane.normal[ box_tr.m_hitside ] = -1.f;
				out_tr->plane.m_type = box_tr.m_hitside;
			}

			return true;
		}

		if ( out_tr->startSolid ) {
			out_tr->allsolid = ( box_tr.m_t2 <= 0.f ) || ( box_tr.m_t2 >= 1.f );
			out_tr->flFraction = 0.f;

			if ( fraction_left_solid )
				*fraction_left_solid = box_tr.m_t2;

			out_tr->end = out_tr->start;
			out_tr->contents = CONTENTS_SOLID;
			out_tr->plane.m_dist = out_tr->start.x;
			out_tr->plane.normal = { 1.f, 0.f, 0.f };
			out_tr->plane.m_type = 0;
			out_tr->start = start + ( delta * box_tr.m_t2 );

			return true;
		}
	}

	return false;
}
void math::VectorITransform( const vec3_t &in, const matrix_t &matrix, vec3_t &out ) {
	vec3_t diff;

	diff = {
		in.x - matrix[ 0 ][ 3 ],
		in.y - matrix[ 1 ][ 3 ],
		in.z - matrix[ 2 ][ 3 ]
	};

	out = {
		diff.x * matrix[ 0 ][ 0 ] + diff.y * matrix[ 1 ][ 0 ] + diff.z * matrix[ 2 ][ 0 ],
		diff.x * matrix[ 0 ][ 1 ] + diff.y * matrix[ 1 ][ 1 ] + diff.z * matrix[ 2 ][ 1 ],
		diff.x * matrix[ 0 ][ 2 ] + diff.y * matrix[ 1 ][ 2 ] + diff.z * matrix[ 2 ][ 2 ]
	};
}

using VMatrixRaw_t = float[ 4 ];

void math::MatrixMultiply( matrix_t &src1, matrix_t &src2, matrix_t &dst ) {
	// Make sure it works if src1 == dst or src2 == dst
	matrix_t tmp1, tmp2;
	const VMatrixRaw_t *s1 = ( &src1 == &dst ) ? tmp1.mat_val : src1.mat_val;
	const VMatrixRaw_t *s2 = ( &src2 == &dst ) ? tmp2.mat_val : src2.mat_val;

	if ( &src1 == &dst ) {
		MatrixCopy( &src1, &tmp1 );
	}
	if ( &src2 == &dst ) {
		MatrixCopy( &src2, &tmp2 );
	}

	dst[ 0 ][ 0 ] = s1[ 0 ][ 0 ] * s2[ 0 ][ 0 ] + s1[ 0 ][ 1 ] * s2[ 1 ][ 0 ] + s1[ 0 ][ 2 ] * s2[ 2 ][ 0 ] + s1[ 0 ][ 3 ] * s2[ 3 ][ 0 ];
	dst[ 0 ][ 1 ] = s1[ 0 ][ 0 ] * s2[ 0 ][ 1 ] + s1[ 0 ][ 1 ] * s2[ 1 ][ 1 ] + s1[ 0 ][ 2 ] * s2[ 2 ][ 1 ] + s1[ 0 ][ 3 ] * s2[ 3 ][ 1 ];
	dst[ 0 ][ 2 ] = s1[ 0 ][ 0 ] * s2[ 0 ][ 2 ] + s1[ 0 ][ 1 ] * s2[ 1 ][ 2 ] + s1[ 0 ][ 2 ] * s2[ 2 ][ 2 ] + s1[ 0 ][ 3 ] * s2[ 3 ][ 2 ];
	dst[ 0 ][ 3 ] = s1[ 0 ][ 0 ] * s2[ 0 ][ 3 ] + s1[ 0 ][ 1 ] * s2[ 1 ][ 3 ] + s1[ 0 ][ 2 ] * s2[ 2 ][ 3 ] + s1[ 0 ][ 3 ] * s2[ 3 ][ 3 ];

	dst[ 1 ][ 0 ] = s1[ 1 ][ 0 ] * s2[ 0 ][ 0 ] + s1[ 1 ][ 1 ] * s2[ 1 ][ 0 ] + s1[ 1 ][ 2 ] * s2[ 2 ][ 0 ] + s1[ 1 ][ 3 ] * s2[ 3 ][ 0 ];
	dst[ 1 ][ 1 ] = s1[ 1 ][ 0 ] * s2[ 0 ][ 1 ] + s1[ 1 ][ 1 ] * s2[ 1 ][ 1 ] + s1[ 1 ][ 2 ] * s2[ 2 ][ 1 ] + s1[ 1 ][ 3 ] * s2[ 3 ][ 1 ];
	dst[ 1 ][ 2 ] = s1[ 1 ][ 0 ] * s2[ 0 ][ 2 ] + s1[ 1 ][ 1 ] * s2[ 1 ][ 2 ] + s1[ 1 ][ 2 ] * s2[ 2 ][ 2 ] + s1[ 1 ][ 3 ] * s2[ 3 ][ 2 ];
	dst[ 1 ][ 3 ] = s1[ 1 ][ 0 ] * s2[ 0 ][ 3 ] + s1[ 1 ][ 1 ] * s2[ 1 ][ 3 ] + s1[ 1 ][ 2 ] * s2[ 2 ][ 3 ] + s1[ 1 ][ 3 ] * s2[ 3 ][ 3 ];

	dst[ 2 ][ 0 ] = s1[ 2 ][ 0 ] * s2[ 0 ][ 0 ] + s1[ 2 ][ 1 ] * s2[ 1 ][ 0 ] + s1[ 2 ][ 2 ] * s2[ 2 ][ 0 ] + s1[ 2 ][ 3 ] * s2[ 3 ][ 0 ];
	dst[ 2 ][ 1 ] = s1[ 2 ][ 0 ] * s2[ 0 ][ 1 ] + s1[ 2 ][ 1 ] * s2[ 1 ][ 1 ] + s1[ 2 ][ 2 ] * s2[ 2 ][ 1 ] + s1[ 2 ][ 3 ] * s2[ 3 ][ 1 ];
	dst[ 2 ][ 2 ] = s1[ 2 ][ 0 ] * s2[ 0 ][ 2 ] + s1[ 2 ][ 1 ] * s2[ 1 ][ 2 ] + s1[ 2 ][ 2 ] * s2[ 2 ][ 2 ] + s1[ 2 ][ 3 ] * s2[ 3 ][ 2 ];
	dst[ 2 ][ 3 ] = s1[ 2 ][ 0 ] * s2[ 0 ][ 3 ] + s1[ 2 ][ 1 ] * s2[ 1 ][ 3 ] + s1[ 2 ][ 2 ] * s2[ 2 ][ 3 ] + s1[ 2 ][ 3 ] * s2[ 3 ][ 3 ];

	dst[ 3 ][ 0 ] = s1[ 3 ][ 0 ] * s2[ 0 ][ 0 ] + s1[ 3 ][ 1 ] * s2[ 1 ][ 0 ] + s1[ 3 ][ 2 ] * s2[ 2 ][ 0 ] + s1[ 3 ][ 3 ] * s2[ 3 ][ 0 ];
	dst[ 3 ][ 1 ] = s1[ 3 ][ 0 ] * s2[ 0 ][ 1 ] + s1[ 3 ][ 1 ] * s2[ 1 ][ 1 ] + s1[ 3 ][ 2 ] * s2[ 2 ][ 1 ] + s1[ 3 ][ 3 ] * s2[ 3 ][ 1 ];
	dst[ 3 ][ 2 ] = s1[ 3 ][ 0 ] * s2[ 0 ][ 2 ] + s1[ 3 ][ 1 ] * s2[ 1 ][ 2 ] + s1[ 3 ][ 2 ] * s2[ 2 ][ 2 ] + s1[ 3 ][ 3 ] * s2[ 3 ][ 2 ];
	dst[ 3 ][ 3 ] = s1[ 3 ][ 0 ] * s2[ 0 ][ 3 ] + s1[ 3 ][ 1 ] * s2[ 1 ][ 3 ] + s1[ 3 ][ 2 ] * s2[ 2 ][ 3 ] + s1[ 3 ][ 3 ] * s2[ 3 ][ 3 ];
}

bool math::IntersectRayWithOBB( const vec3_t &start, const vec3_t &delta, matrix_t &obb_to_world, const vec3_t &mins, const vec3_t &maxs, float tolerance, trace_t *out_tr ) {
	vec3_t box_extents, box_center, extent{}, uextent, segment_center, cross, new_start, tmp_end;
	float  coord, tmp, cextent, sign;

	// note - dex; this is Collision_ClearTrace.
	out_tr->start = start;
	out_tr->end = start;
	out_tr->end += delta;
	out_tr->startSolid = false;
	out_tr->allsolid = false;
	out_tr->flFraction = 1.f;
	out_tr->contents = 0;

	// compute center in local space and transform to world space.
	box_extents = ( mins + maxs ) / 2.f;
	VectorTransform( box_extents, obb_to_world, box_center );

	// calculate extents from local center.
	box_extents = maxs - box_extents;

	// save the extents of the ray.
	segment_center = start + delta - box_center;

	// check box axes for separation.
	for ( auto i = 0; i < 3; ++i ) {
		extent[ i ] = delta.x * obb_to_world[ 0 ][ i ] + delta.y * obb_to_world[ 1 ][ i ] + delta.z * obb_to_world[ 2 ][ i ];
		uextent[ i ] = std::abs( extent[ i ] );

		coord = segment_center.x * obb_to_world[ 0 ][ i ] + segment_center.y * obb_to_world[ 1 ][ i ] + segment_center.z * obb_to_world[ 2 ][ i ];
		coord = std::abs( coord );
		if ( coord > ( box_extents[ i ] + uextent[ i ] ) )
			return false;
	}

	// now check cross axes for separation.
	cross = delta.cross( segment_center );
	
	cextent = cross.x * obb_to_world[ 0 ][ 0 ] + cross.y * obb_to_world[ 1 ][ 0 ] + cross.z * obb_to_world[ 2 ][ 0 ];
	cextent = std::abs( cextent );
	tmp = box_extents.y * uextent.z + box_extents.z * uextent.y;
	if ( cextent > tmp )
		return false;

	cextent = cross.x * obb_to_world[ 0 ][ 1 ] + cross.y * obb_to_world[ 1 ][ 1 ] + cross.z * obb_to_world[ 2 ][ 1 ];
	cextent = std::abs( cextent );
	tmp = box_extents.x * uextent.z + box_extents.z * uextent.x;
	if ( cextent > tmp )
		return false;

	cextent = cross.x * obb_to_world[ 0 ][ 2 ] + cross.y * obb_to_world[ 1 ][ 2 ] + cross.z * obb_to_world[ 2 ][ 2 ];
	cextent = std::abs( cextent );
	tmp = box_extents.x * uextent.y + box_extents.y * uextent.x;
	if ( cextent > tmp )
		return false;

	// we hit this box, compute intersection point and return.
	// compute ray start in bone space.
	VectorITransform( start, obb_to_world, new_start );

	// extent is ray.m_Delta in bone space, recompute delta in bone space.
	extent *= 2.f;

	// delta was prescaled by the current t, so no need to see if this intersection is closer.
	if ( !IntersectRayWithBox( start, extent, mins, maxs, tolerance, out_tr ) )
		return false;

	// fix up the start/end pos and fraction
	VectorTransform( out_tr->end, obb_to_world, tmp_end );

	out_tr->end = tmp_end;
	out_tr->start = start;
	out_tr->flFraction *= 2.f;

	// fix up the plane information
	sign = out_tr->plane.normal[ out_tr->plane.m_type ];

	out_tr->plane.normal.x = sign * obb_to_world[ 0 ][ out_tr->plane.m_type ];
	out_tr->plane.normal.y = sign * obb_to_world[ 1 ][ out_tr->plane.m_type ];
	out_tr->plane.normal.z = sign * obb_to_world[ 2 ][ out_tr->plane.m_type ];
	out_tr->plane.m_dist = out_tr->end.dot( out_tr->plane.normal );
	out_tr->plane.m_type = 3;

	return true;
}
bool math::IntersectRayWithOBB( custom_ray ray,
						  const vec3_t &vecBoxOrigin, const ang_t &angBoxRotation,
						  const vec3_t &vecOBBMins, const vec3_t &vecOBBMaxs, float flTolerance, trace_t *pTrace ) {
	if ( angBoxRotation == ang_t{} ) {
		vec3_t vecAbsMins, vecAbsMaxs;
		vecAbsMins = vecBoxOrigin + vecOBBMins;
		vecAbsMaxs = vecBoxOrigin + vecOBBMaxs;
		return math::IntersectRayWithBox( ray.m_start, ray.m_ray_dir, vecAbsMins, vecAbsMaxs, flTolerance, pTrace );
	}

	matrix_t obbToWorld;
	angle_matrix( angBoxRotation, vecBoxOrigin, &obbToWorld );
	return IntersectRayWithOBB( ray.m_start, ray.m_ray_dir, obbToWorld, vecOBBMins, vecOBBMaxs, flTolerance, pTrace );
}