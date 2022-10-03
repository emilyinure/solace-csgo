#pragma once
#include "vec3.h"

struct trace_t;
class ang_t;
class cmd_t;
class weapon_t;
class vec3_t;
struct matrix_t;
namespace math {
	class custom_ray {
	public:
		vec3_t m_ray_dir;
		vec3_t m_start;
		vec3_t m_end;
		bool m_startsolid;
		float m_t1;
		int m_hitside;
		float m_t2;
		custom_ray( ) : m_t1 { -1.f }, m_t2{ 1.f }, m_hitside{ -1 }, m_startsolid{ true } {}
		custom_ray( vec3_t start, vec3_t end );
	};
	bool world_to_screen( const vec3_t &point, vec3_t &screen );
	float distSegmentToSegment( const vec3_t &p1, const vec3_t &p2, const vec3_t &q1, const vec3_t &q2, float &invariant1, float &invariant2 );
	void correct_movement( cmd_t *cmd );
	float minimum_distance( vec3_t v, vec3_t w, vec3_t p );
	inline float normalize_angle( float ang, float max ) {
		while ( ang > max )
			ang -= max * 2;
		while ( ang < -max )
			ang += max * 2;
		return ang;
	}

	void sin_cos ( float r, float *s, float *c );

	vec3_t get_closest_on_line ( vec3_t start, vec3_t end, vec3_t target );

	vec3_t closest_to_point ( const custom_ray &ray, const vec3_t &point );

	float dist_Segment_to_Segment( const custom_ray &ray, const vec3_t &min, const vec3_t &max );

	bool intersects_capsule( const custom_ray &ray, const vec3_t &min, const vec3_t &max, const float &radius );

	float RandomFloat( float fMin, float fMax );
	float RandomInt( int fMin, int fMax );
	void RandomSeed( int Seed );
	vec3_t calculate_spread( weapon_t *weapon, int seed, float inaccuracy, float spread, bool revolver );

	vec3_t get_bone_position ( matrix_t bone_matrices );
	void VectorRotate ( vec3_t in1, matrix_t in2, vec3_t &out );
	void VectorTransform( vec3_t in1, matrix_t &in2, vec3_t &out );
	void MatrixGetColumn( matrix_t &in, int column, vec3_t &out );
	void MatrixSetColumn( vec3_t &in, int column, matrix_t &out );
	void VectorScale( const float *in, float scale, float *out );
	void AngleMatrix ( const ang_t angles, matrix_t *matrix );

	using AngleMatrix_t = void( __fastcall * )( const vec3_t &, matrix_t & );
	void angle_matrix( const ang_t ang, const vec3_t &pos, matrix_t *out );
	void MatrixAngles( matrix_t &matrix, ang_t &angles );
	void MatrixAngles( matrix_t &matrix, ang_t &angles, vec3_t &position );
	void QuaternionMatrix( const quaternion_t &q, matrix_t *matrix );
	void QuaternionMatrix ( const quaternion_t &q, const vec3_t &pos, matrix_t *matrix );
	void ConcatTransforms ( matrix_t in1, matrix_t in2, matrix_t *out );
	float get_fov ( const ang_t &view_angles, const vec3_t &start, const vec3_t &end );
	bool IntersectRayWithBox( const vec3_t &start, const vec3_t &delta, const vec3_t &mins, const vec3_t &maxs, float tolerance, trace_t *out_tr, float *fraction_left_solid = nullptr );
	void VectorITransform( const vec3_t &in, const matrix_t &matrix, vec3_t &out );
	void MatrixMultiply( matrix_t &src1, matrix_t &src2, matrix_t &dst );
	bool IntersectRayWithOBB( const vec3_t &start, const vec3_t &delta, matrix_t &obb_to_world, const vec3_t &mins, const vec3_t &maxs, float
	                          tolerance, trace_t *out_tr );
	bool IntersectRayWithOBB( custom_ray ray, const vec3_t &vecBoxOrigin, const ang_t &angBoxRotation, const vec3_t &vecOBBMins, const vec3_t &
	                          vecOBBMaxs, float flTolerance, trace_t *pTrace );
	bool IntersectRayWithBox( const vec3_t &start, const vec3_t &delta, const vec3_t &mins, const vec3_t &maxs, float tolerance, custom_ray *
	                          out_info );
}
