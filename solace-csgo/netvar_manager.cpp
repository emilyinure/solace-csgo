#include "netvar_manager.h"
#include "includes.h"

float get_new( float old, float new_, float tol ) {
	const float delta = new_ - old;
	if ( fabsf( delta ) <= tol )
		return old;
	return new_;
}

void shared_netvar::post_update ( player_t *player ) {
	m_value = *reinterpret_cast< float* >( player + m_offset );
	*reinterpret_cast< float* >( player + m_offset ) = get_new( m_old_value, m_value, m_tolerance );
}

void shared_netvar::pre_update( player_t *player ) {
	m_old_value = *reinterpret_cast< float * >( player + m_offset );
}

void managed_vec::pre_update ( player_t *player ) {
	m_old_value = *reinterpret_cast< vec3_t * >(player + m_offset);
}

void managed_vec::post_update( player_t *player ) {
	m_value = *reinterpret_cast< vec3_t * >( player + m_offset );

	*reinterpret_cast< float* >( player + m_offset ) = get_new( m_old_value.x, m_value.x, m_tolerance );
	*reinterpret_cast< float* >( player + m_offset + 0x4 ) = get_new( m_old_value.y, m_value.y, m_tolerance );
	*reinterpret_cast< float* >( player + m_offset + 0x8 ) = get_new( m_old_value.z, m_value.z, m_tolerance );
}

void prediction_netvar_manager::pre_update( player_t *player ) {
	for ( auto *var : vars )
		var->pre_update( player );
}

void prediction_netvar_manager::post_update( player_t *player ) {
	for ( auto *var : vars )
		var->post_update( player );
}
#define	EQUAL_EPSILON	0.001 
inline bool CloseEnough( float a, float b, float epsilon = EQUAL_EPSILON ) {
	return fabs( a - b ) <= epsilon;
}
float AssignRangeMultiplier( int nBits, double range ) {
	unsigned long iHighValue;
	if ( nBits == 32 )
		iHighValue = 0xFFFFFFFE;
	else
		iHighValue = ( ( 1 << ( unsigned long )nBits ) - 1 );

	float fHighLowMul = iHighValue / range;
	if ( CloseEnough( range, 0 ) )
		fHighLowMul = iHighValue;

	// If the precision is messing us up, then adjust it so it won't.
	if ( ( unsigned long )( fHighLowMul * range ) > iHighValue ||
		( fHighLowMul * range ) > ( double )iHighValue ) {
		// Squeeze it down smaller and smaller until it's going to produce an integer
		// in the valid range when given the highest value.
		float multipliers[ ] = { 0.9999f, 0.99f, 0.9f, 0.8f, 0.7f };
		int i;
		for ( i = 0; i < ARRAYSIZE( multipliers ); i++ ) {
			fHighLowMul = ( float )( iHighValue / range ) * multipliers[ i ];
			if ( ( unsigned long )( fHighLowMul * range ) > iHighValue ||
				( fHighLowMul * range ) > ( double )iHighValue ) {
			}
			else {
				break;
			}
		}

		if ( i == ARRAYSIZE( multipliers ) ) {
			// Doh! We seem to be unable to represent this range.
			return 0;
		}
	}

	return fHighLowMul;
}

void prediction_netvar_manager::init ( datamap_t *map ) {
	float val = ( 1.f / AssignRangeMultiplier( 17, 4096.f - ( -4096.f ) ) );
	vars.push_back( new shared_netvar( g.m_offsets->m_player.m_fall_velocity, val, "m_flFallVelocity" ) );
	val = ( 1.f / AssignRangeMultiplier( 16, 128.f ) );
	vars.push_back( new shared_netvar( g.m_offsets->m_player.m_step_size, val, "m_flStepSize" ) );
	val = ( 1.f / AssignRangeMultiplier( 12, 2048.0f ) );
	vars.push_back( new shared_netvar( g.m_offsets->m_player.m_max_speed, val, "max_speed" ) );
	val = ( 1.f / AssignRangeMultiplier( 14, 100.f ) );
	vars.push_back( new shared_netvar( g.m_offsets->m_player.m_stamina, val, "stamina" ) );
	val = ( 1.f / AssignRangeMultiplier( 8.f, 1.f ) );
	vars.push_back( new shared_netvar( g.m_offsets->m_player.velocity_modifier, val, "velocity_modifier" ) );
	val = ( 1.f / AssignRangeMultiplier( 8, 32.0f - ( -32.0f ) ) );
	vars.push_back( new shared_netvar( g.m_offsets->m_player.m_view_offset, val, "view_offset_x" ) );
	vars.push_back( new shared_netvar( g.m_offsets->m_player.m_view_offset + 0x4, val, "view_offset_y" ) );
	val = ( 1.f / AssignRangeMultiplier( 10, 128.f ));
	vars.push_back( new shared_netvar( g.m_offsets->m_player.m_view_offset + 0x8, val, "view_offset_z" ) );
	//val = ( 1.f / AssignRangeMultiplier( 8, 1. ) );
	//vars.push_back( new shared_netvar( g.m_offsets->m_player.velocity_modifier, val, "m_velocityModifier" ) );
	val = ( 1.f / AssignRangeMultiplier( 20, 2000. ) );
	vars.push_back( new managed_vec( g.m_offsets->m_player.m_vecBaseVelocity, val, "m_vecBaseVelocity" ) );

	//vars.push_back( new managed_vec( g.m_offsets->m_player.m_network_origin, 0.031250, "network_origin", true ) );
	vars.push_back( new managed_vec( g.m_offsets->m_player.m_velocity, 0.031250, "m_velocity", true ) );

	vars.push_back( new managed_vec( g.m_offsets->m_player.m_punch_angle, 0.031250, "m_viewPunchAngle", true ) );
	vars.push_back( new managed_vec( g.m_offsets->m_player.m_aim_punch_angle, 0.031250, "m_aimPunchAngle", true) );
	vars.push_back( new managed_vec( g.m_offsets->m_player.m_aim_punch_angle_vel, 0.031250, "m_aimPunchAngleVel", true) );
	//var = map->find_var( "m_nDuckTimeMsecs" );
	//vars.push_back( new shared_netvar( g.m_offsets->m_player.m_nDuckTimeMsecs, var->m_tolerance, "m_nDuckTimeMsecs" ) );
	//var = map->find_var( "m_nDuckJumpTimeMsecs" );
	//vars.push_back( new shared_netvar( g.m_offsets->m_player.m_nDuckJumpTimeMsecs, var->m_tolerance, "m_nDuckJumpTimeMsecs" ) );
	//var = map->find_var( "m_nJumpTimeMsecs" );
	//vars.push_back( new shared_netvar( g.m_offsets->m_player.m_nJumpTimeMsecs, var->m_tolerance, "m_nJumpTimeMsecs" ) );
}
