#pragma once
#include <cmath>
#include <future>

inline float bits_to_float( std::uint32_t i ) {
	union convertor_t {
		float f; unsigned long ul;
	} tmp;

	tmp.ul = i;
	return tmp.f;
}

constexpr double M_PI = 3.14159265358979323846;
constexpr double M_PI_SQUARED = 9.869604401089358;

constexpr double M_PI_2 = M_PI * 2;
constexpr double M_PI_2_F = static_cast< float >( M_PI_2 );
constexpr float M_PI_F = static_cast< float >( M_PI );
constexpr float RAD2DEG( const float x ) {
	return static_cast< float >(x) * static_cast< float >(180.f / M_PI_F);
}
constexpr float DEG2RAD( const float x ) {
	return static_cast< float >(x) * static_cast< float >(M_PI_F / 180.f);
}

constexpr std::uint32_t FLOAT32_NAN_BITS = 0x7FC00000;
const float FLOAT32_NAN = bits_to_float( FLOAT32_NAN_BITS );
#define VEC_T_NAN FLOAT32_NAN
#define ASSERT( _exp ) ( (void ) 0 )
class vec3_t;
class ang_t {
public:
	float x, y, z;
	__forceinline ang_t( ) : x{0}, y{0}, z{0} {
	}
	ang_t( float x, float y, float z ) : x( x ), y( y ), z( z ) { }
	float &operator[]( int i ) { return reinterpret_cast< float * >(this)[ i ]; }
	float operator[]( int i ) const { return ( ( float * )this )[ i ]; }
	__forceinline bool operator==( const ang_t &v ) const {
		return v.x == x && v.y == y && v.z == z;
	}
	__forceinline bool operator!=( const ang_t &v ) const {
		return v.x != x || v.y != y || v.z != z;
	}
	ang_t &operator+=( const ang_t &v ) {
		x += v.x; y += v.y; z += v.z; return *this;
	}
	ang_t &operator-=( const ang_t &v ) {
		x -= v.x; y -= v.y; z -= v.z; return *this;
	}
	ang_t &operator*=( float v ) {
		x *= v; y *= v; z *= v; return *this;
	}
	ang_t operator+( const ang_t &v ) const {
		return ang_t{ x + v.x, y + v.y, z + v.z };
	}
	ang_t operator-( const ang_t &v ) {
		return ang_t{ x - v.x, y - v.y, z - v.z };
	}
	ang_t operator-( ) const {
		return ang_t{ -x, -y, -z };
	}
	ang_t operator*( float fl ) const {
		return ang_t( x * fl, y * fl, z * fl );
	}
	ang_t operator*( const ang_t &v ) const {
		return ang_t( x * v.x, y * v.y, z * v.z );
	}
	ang_t &operator/=( float fl ) {
		x /= fl;
		y /= fl;
		z /= fl;
		return *this;
	}
	auto operator-( const ang_t &other ) const -> ang_t {
		auto buf = *this;

		buf.x -= other.x;
		buf.y -= other.y;
		buf.z -= other.z;

		return buf;
	}

	auto operator/( float other ) const {
		ang_t vec;
		vec.x = x / other;
		vec.y = y / other;
		vec.z = z / other;
		return vec;
	}
	
	[[nodiscard]] float delta( const ang_t target ) const {
		float ret = target.y - y;
		while ( ret > 180 )
			ret -= 360;
		while ( ret < -180 )
			ret += 360;
		ret += target.x - x;
		return ret;
	}

	ang_t rotate( const float x, const float y, const float z ) {
		this->x += x;
		this->y += y;
		this->z += z;
		return *this;
	}
	
	void vectors ( vec3_t *forward, vec3_t *right, vec3_t *up ) const;
	vec3_t forward ( ) const;
};

class quaternion_t {
public:
	float x = 0, y = 0, z = 0, w = 0;
	void init( float x, float y, float z, float w ) {
		this->x = ( x );
		this->y = ( y );
		this->z = ( z );
		this->w = ( w );
	}
	quaternion_t( ) {};
	quaternion_t( float x, float y, float z, float w ) : x( x ), y( y ), z( z ), w( w ) {}
	void operator=( quaternion_t& other ) 		{
		init( other.x, other.y, other.z, other.w );
	}
};

class vec3_t {
public:
	float x, y, z;
	vec3_t( );
	vec3_t( float, float, float );
	~vec3_t( );


	vec3_t &operator=( const vec3_t &vOther ) {
		init( vOther.x, vOther.y, vOther.z );
		return *this;
	}
	[[nodiscard]] __forceinline vec3_t cross( const vec3_t &v ) const {
		return {
			( y * v.z ) - ( z * v.y ),
			( z * v.x ) - ( x * v.z ),
			( x * v.y ) - ( y * v.x )
		};
	}


	vec3_t( const float *clr ) {
		x = clr[ 0 ];
		y = clr[ 1 ];
		z = clr[ 2 ];
	}
	
	void abs() {
		x = fabsf(x);
		y = fabsf(y);
		z = fabsf(z);
	}

	void init( float _x, float _y, float _z ) {
		x = _x; y = _y; z = _z;
	}
	void reset() {
		x = 0;
		y = 0;
		z = 0;
	}
	float &operator[]( int i ) { return reinterpret_cast< float * >(this)[ i ]; }
	float operator[]( int i ) const { return ( ( float * )this )[ i ]; }
	__forceinline bool operator==( const vec3_t &v ) const {
		return v.x == x && v.y == y && v.z == z;
	}
	__forceinline bool operator!=( const vec3_t &v ) const {
		return v.x != x || v.y != y || v.z != z;
	}
	vec3_t &operator+=( const vec3_t &v ) {
		x += v.x; y += v.y; z += v.z; return *this;
	}
	vec3_t &operator-=( const vec3_t &v ) {
		x -= v.x; y -= v.y; z -= v.z; return *this;
	}
	vec3_t &operator*=( float v ) {
		x *= v; y *= v; z *= v; return *this;
	}
	vec3_t operator+( const vec3_t &v ) const {
		return vec3_t{ x + v.x, y + v.y, z + v.z };
	}
	vec3_t operator-( const vec3_t &v ) {
		return vec3_t{ x - v.x, y - v.y, z - v.z };
	}
	vec3_t operator-( ) const {
		return vec3_t{ -x, -y, -z };
	}
	vec3_t operator*( float fl ) const {
		return vec3_t( x * fl, y * fl, z * fl );
	}
	vec3_t operator*( const vec3_t &v ) const {
		return vec3_t( x * v.x, y * v.y, z * v.z );
	}
	vec3_t &operator/=( float fl ) {
		x /= fl;
		y /= fl;
		z /= fl;
		return *this;
	}
	auto operator-( const vec3_t &other ) const -> vec3_t {
		auto buf = *this;

		buf.x -= other.x;
		buf.y -= other.y;
		buf.z -= other.z;

		return buf;
	}

	auto operator/( float other ) const {
		vec3_t vec;
		vec.x = x / other;
		vec.y = y / other;
		vec.z = z / other;
		return vec;
	}
	auto operator<=( float other ) const {
		return ( fabsf( x ) <= other &&
			fabsf( y ) <= other &&
			fabsf( z ) <= other );
	}

	auto operator!=( vec3_t other ) const {
		return ( fabsf( x - other.x ) >= 0.f ||
			fabsf( y - other.y ) >= 0.f ||
			fabsf( z - other.z ) >= 0.f );
	}
	auto operator==( vec3_t other ) const {
		return ( fabsf( x - other.x ) <= 0.f &&
			fabsf( y - other.y ) <= 0.f &&
			fabsf( z - other.z ) <= 0.f );
	}

	__forceinline float dot( const vec3_t &b ) const {
		return( this->x * b.x + this->y * b.y + this->z * b.z );
	}

	[[nodiscard]] float length( void ) const {
		const auto sqsr = length_sqr( );

		if ( sqsr == 0 )
			return 0.f;

		return sqrt( sqsr );
	}
	[[nodiscard]] float length_2d_sqr() const {
		auto sqr = [](float n) {
			return static_cast<float>(n * n);
		};
		const auto sqsr = sqr(x) + sqr(y);

		return sqsr;
	}

	[[nodiscard]] float length_2d( ) const {
		float len = length_2d_sqr();
		if ( isnan( len ) || len == 0 )
			return 0.f;
		return sqrt( len );
	}

	[[nodiscard]] float length_sqr( ) const {
		auto sqr = [ ]( float n ) {
			return static_cast< float >( n * n );
		};

		return ( sqr( x ) + sqr( y ) + sqr( z ) );
	}


	float normalize( ) {
		const float ln = length( );
		if (isnan(ln) || ln == 0)
			return 0;
		x /= ln;
		y /= ln;
		z /= ln;
		return ln;
	};
	
	vec3_t normalized( ) const {
		float len = length();
		if ( isnan( len ) || len == 0 )
			return vec3_t(0,0,0 );
		return ( *this ) / len;
	}

	[[nodiscard]] ang_t look( vec3_t target ) const {
		target -= *this;
		ang_t angles;
		if ( target.y == 0.0f && target.x == 0.0f ) {
			angles.x = ( target.z > 0.0f ) ? 270.0f : 90.0f;
			angles.y = 0.0f;
		} else {
			angles.x = static_cast<float>(-atan2( -target.z, target.length_2d( ) ) * -180.f / M_PI);
			angles.y = static_cast< float >(atan2( target.y, target.x ) * 180.f / M_PI);

			//if ( angles.y > 90 )
			//	angles.y -= 180;
			//else if ( angles.y < 90 )
			//	angles.y += 180;
			//else if ( angles.y == 90 )
			//	angles.y = 0;
		}

		angles.z = 0.0f;
		return angles;
	}
};

class __declspec( align( 16 ) ) vec_aligned_t : public vec3_t {
public:
	vec_aligned_t &operator=( const vec3_t &vOther ) {
		init( vOther.x, vOther.y, vOther.z );
		return *this;
	}

	float w = 0;
};

#define VEC_T_NAN FLOAT32_NAN
#define ASSERT( _exp ) ( (void ) 0 )
struct matrix_t {
	matrix_t( ) { }
	matrix_t(
		float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23 ) {
		mat_val[ 0 ][ 0 ] = m00;	mat_val[ 0 ][ 1 ] = m01; mat_val[ 0 ][ 2 ] = m02; mat_val[ 0 ][ 3 ] = m03;
		mat_val[ 1 ][ 0 ] = m10;	mat_val[ 1 ][ 1 ] = m11; mat_val[ 1 ][ 2 ] = m12; mat_val[ 1 ][ 3 ] = m13;
		mat_val[ 2 ][ 0 ] = m20;	mat_val[ 2 ][ 1 ] = m21; mat_val[ 2 ][ 2 ] = m22; mat_val[ 2 ][ 3 ] = m23;
	}

	//-----------------------------------------------------------------------------
	// Creates a matrix where the X axis = forward
	// the Y axis = left, and the Z axis = up
	//-----------------------------------------------------------------------------
	void init( const vec3_t &x, const vec3_t &y, const vec3_t &z, const vec3_t &origin ) {
		mat_val[ 0 ][ 0 ] = x.x; mat_val[ 0 ][ 1 ] = y.x; mat_val[ 0 ][ 2 ] = z.x; mat_val[ 0 ][ 3 ] = origin.x;
		mat_val[ 1 ][ 0 ] = x.y; mat_val[ 1 ][ 1 ] = y.y; mat_val[ 1 ][ 2 ] = z.y; mat_val[ 1 ][ 3 ] = origin.y;
		mat_val[ 2 ][ 0 ] = x.z; mat_val[ 2 ][ 1 ] = y.z; mat_val[ 2 ][ 2 ] = z.z; mat_val[ 2 ][ 3 ] = origin.z;
	}

	//-----------------------------------------------------------------------------
	// Creates a matrix where the X axis = forward
	// the Y axis = left, and the Z axis = up
	//-----------------------------------------------------------------------------
	matrix_t( const vec3_t &x, const vec3_t &y, const vec3_t &z, const vec3_t &origin ) {
		init( x, y, z, origin );
	}

	inline void set_origin( vec3_t const &p ) {
		mat_val[ 0 ][ 3 ] = p.x;
		mat_val[ 1 ][ 3 ] = p.y;
		mat_val[ 2 ][ 3 ] = p.z;
	}

	inline vec3_t get_origin( ) {
		return {
		mat_val[ 0 ][ 3 ],
		mat_val[ 1 ][ 3 ],
		mat_val[ 2 ][ 3 ]
		};
	}

	inline void invalidate( void ) {
		for ( auto i = 0; i < 3; i++ ) {
			for ( auto j = 0; j < 4; j++ ) {
				mat_val[ i ][ j ] = VEC_T_NAN;
			}
		}
	}

	float *operator[]( int i ) { ASSERT( ( i >= 0 ) && ( i < 3 ) ); return mat_val[ i ]; }
	matrix_t operator* ( matrix_t other ) const;
	matrix_t operator* ( float other ) const;
	matrix_t operator+ ( matrix_t other ) const;
	const float *operator[]( int i ) const { ASSERT( ( i >= 0 ) && ( i < 3 ) ); return mat_val[ i ]; }
	float *base( ) { return &mat_val[ 0 ][ 0 ]; }
	const float *base( ) const { return &mat_val[ 0 ][ 0 ]; }

	float mat_val[ 3 ][ 4 ];
};
class bone_array_t : public matrix_t {
public:
	bool get_bone( vec3_t &out, int bone = 0 ) {
		if ( bone < 0 || bone >= 128 )
			return false;

		matrix_t *bone_matrix = &this[ bone ];

		if ( !bone_matrix )
			return false;

		out = { bone_matrix->mat_val[ 0 ][ 3 ], bone_matrix->mat_val[ 1 ][ 3 ], bone_matrix->mat_val[ 2 ][ 3 ] };

		return true;
	}
};