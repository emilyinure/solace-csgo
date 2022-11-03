#include "player_manager.h"

#include "bones.h"
#include "resolver.h"
#include "thread_handler.h"

player_record_t::~player_record_t ( ) {
	// free heap allocated game mem.
	g.m_interfaces->mem_alloc( )->free( m_bones );
	for ( auto i = 0; i < 10; i++ )
		g.m_interfaces->mem_alloc( )->free( m_fake_bones[ i ] );
	g.m_interfaces->mem_alloc( )->free( m_fake_bones );
}

void player_record_t::cache ( int index ) const {
	// get bone cache ptr.
	auto *const cache = &m_ent->bone_cache( );

	if ( index == -1 )
		cache->m_pCachedBones = m_bones;
	else
		cache->m_pCachedBones = m_fake_bones[ index ];
	cache->m_CachedBoneCount = 128;

	m_ent->origin( ) = m_pred_origin;
	m_ent->mins( ) = m_mins;
	m_ent->maxs( ) = m_maxs;
	
	m_ent->set_abs_angles( m_abs_angles );
	m_ent->set_abs_origin( m_pred_origin );
}

bool player_record_t::valid() const {
	if (!g.m_interfaces->client_state()->m_net_channel)
		return false;
	// use prediction curtime for this.
	float curtime = g.ticks_to_time(g.m_local->tick_base() + 1);

	// correct is the amount of time we have to correct game time,
	float correct = g.m_lerp;

	// stupid fake latency goes into the incoming latency.
	auto* nci = g.m_interfaces->engine()->get_net_channel_info();
	if (nci)
		correct += nci->GetLatency(2);
	// check bounds [ 0, sv_maxunlag ]
	static auto* sv_maxunlag = g.m_interfaces->console()->get_convar("sv_maxunlag");
	correct = std::clamp<float>(correct, 0.f, sv_maxunlag->GetFloat());

	// calculate difference between tick sent by player and our latency based tick.
	// ensure this record isn't too old.
	return std::abs(correct - (curtime - m_pred_time)) < 0.195f;
}

//bool player_record_t::valid() const {
//	if (!m_setup)
//		return false;
//	// use prediction curtime for this.
//	static auto* sv_maxunlag = g.m_interfaces->console()->get_convar("sv_maxunlag");
//	auto* net = g.m_interfaces->engine()->get_net_channel_info();
//	//const auto curtime = g.ticks_to_time( g.m_local->tick_base( ) );
//	//
//	//// correct is the amount of time we have to correct game time,
//	//float correct = g.m_lerp;
//	//
//	//// stupid fake latency goes into the incoming latency.
//	//float in = net->GetAvgLatency( 1 );;
//	//correct += in;
//	//
//	//// check bounds [ 0, sv_maxunlag ]
//	//correct = std::clamp( correct, 0.f, sv_maxunlag->GetFloat( ) );
//	//
//	//// calculate difference between tick sent by player and our latency based tick.
//	//// ensure this record isn't too old.
//	//return std::abs( correct - (curtime - m_pred_time) ) <= 0.2f;
//
//	const auto tickcount = g.time_to_ticks(m_pred_time + g.m_lerp);
//
//	const auto avg_latency = net->GetAvgLatency(1) + net->GetAvgLatency(0);
//
//	int tick_delay = 1;
//	if (settings::hvh::antiaim::fakewalk)
//		tick_delay = 15 - g.m_lag;
//	const auto arrival_tick = g.m_local->tick_base() + g.time_to_ticks(avg_latency) + tick_delay;
//
//	const auto correct = std::clamp( g.m_lerp + net->GetLatency( 0 ), 0.f, sv_maxunlag->GetFloat( ) ) - g.ticks_to_time( arrival_tick + g.time_to_ticks( g.m_lerp ) - tickcount );
//
//	return std::abs( correct ) < 0.2f - g.m_interfaces->globals( )->m_interval_per_tick;
//}

player_record_t::player_record_t (ent_info_t * info, float last_sim ) {
	m_info = info;
	m_ent = info->m_ent;

	m_pred_time = m_sim_time = info->m_ent->sim_time( );
	m_old_sim_time = last_sim;
	m_anim_time = m_old_sim_time + g.m_interfaces->globals( )->m_interval_per_tick;
	m_lag = g.time_to_ticks( m_sim_time - m_old_sim_time );
	m_pred_origin = m_origin = info->m_ent->origin( );
	m_abs_origin = info->m_ent->abs_origin( );
	m_eye_angles = info->m_ent->eye_angles( );
	m_maxs = info->m_ent->maxs( );
	m_mins = info->m_ent->mins( );
	m_duck = info->m_ent->duck_amount( );
	m_duck_speed = info->m_ent->duck_speed();
	m_pred_flags = m_flags = info->m_ent->flags( );
	m_setup = false;
	m_pred_velocity = m_velocity = info->m_ent->velocity( );
	m_body = info->m_ent->lower_body_yaw();
	
	m_tick = g.m_interfaces->client_state(  )->m_server_tick;
	m_ent->GetAnimLayers( m_layers );
	m_ent->GetPoseParameters( m_poses );
	m_cycle = m_ent->cycle( );
	m_sequence = m_ent->sequence( );

	m_bones = static_cast< bone_array_t * >( g.m_interfaces->mem_alloc( )->alloc( sizeof( bone_array_t ) * 128 ) );
	m_fake_bones = static_cast< bone_array_t** >( g.m_interfaces->mem_alloc( )->alloc( sizeof( bone_array_t* ) * 10 ) );
	for ( auto i = 0; i < 10; i++ )
		m_fake_bones[ i ] = static_cast< bone_array_t* >( g.m_interfaces->mem_alloc( )->alloc( sizeof( bone_array_t ) * 128 ) );
}

template <class T>
T Lerp(float flPercent, T const& A, T const& B)
{
	return (A - B) * flPercent;
}

const char* const g_szWeaponPrefixLookupTable[] = {
	"knife",
	"pistol",
	"smg",
	"rifle",
	"shotgun",
	"sniper",
	"heavy",
	"c4",
	"grenade",
	"knife"
};
const char* GetWeaponPrefix(std::shared_ptr<player_record_t> record)
{
	static auto sig = util::find("client.dll", "53 56 8B F1 57 33 FF 8B 4E ? 8B 01");
	return ((const char* (__thiscall*)(void*))sig)(record->m_ent->get_anim_state());
	int nWeaponType = 0; // knife

	auto m_pWeapon = (weapon_t *)g.m_interfaces->entity_list()->get_client_entity_handle(record->m_ent->active_weapon());
	if (m_pWeapon)
	{
		auto m_weapon_info = g.m_interfaces->weapon_system()->get_weapon_data(m_pWeapon->item_definition_index());
		if (m_weapon_info) {
			nWeaponType = (int)m_weapon_info->m_weapon_type;

			int nWeaponID = m_pWeapon->item_definition_index();
			if (nWeaponID == WEAPON_MAG7)
			{
				nWeaponType = WEAPONTYPE_RIFLE;
			}
			else if (nWeaponID == WEAPON_TASER)
			{
				nWeaponType = WEAPONTYPE_PISTOL;
			}
		}
	}

	return g_szWeaponPrefixLookupTable[std::clamp(nWeaponType, 0, 9)];
}
float Approach(float target, float value, float speed)
{
	float delta = target - value;

	if (delta > speed)
		value += speed;
	else if (delta < -speed)
		value -= speed;
	else
		value = target;

	return value;
}   
float InvSqrt(float x) {
	float xhalf = 0.5f * x;
	int i = *(int*)&x;            // store floating-point bits in integer
	i = 0x5f3759df - (i >> 1);    // initial guess for Newton's method
	x = *(float*)&i;              // convert new bits into float
	x = x * (1.5f - xhalf * x * x);     // One round of Newton's method
	return x;
}
float ent_info_t::test_velocity(std::shared_ptr<player_record_t> record, float speed, float pVec) {
	auto weapon = (weapon_t*)g.m_interfaces->entity_list()->get_client_entity_handle(record->m_ent->active_weapon()); 
	float max_speed = record->m_ent->max_speed();
	auto info = weapon ? g.m_interfaces->weapon_system()->get_weapon_data(weapon->item_definition_index()) : nullptr;
	if (info) {
		max_speed = info->m_max_player_speed;
	}




	float flMoveCycleRate = 1.f / speed;
	float flSequenceGroundSpeed = 0.001f;

	float ukn = 1.0 / (float)(1.f / flMoveCycleRate);
	if ((pVec * (float)(1.f / (float)(1.0f / ukn)), 2) > 0.001f) {
		flSequenceGroundSpeed = powf(pVec, 2.f) * ukn;
	}
	flSequenceGroundSpeed = fmaxf(flSequenceGroundSpeed, 0.001f);
	return (speed / flSequenceGroundSpeed) * flMoveCycleRate;
}
inline float SimpleSpline( float value ) {
	float valueSquared = value * value;

	// Nice little ease-in, ease-out spline-like curve
	return ( 3 * valueSquared - 2 * valueSquared * value );
}
// remaps a value in [startInterval, startInterval+rangeInterval] from linear to
// spline using SimpleSpline
inline float SimpleSplineRemapValClamped( float val, float A, float B, float C, float D ) {
	if ( A == B )
		return val >= B ? D : C;
	float cVal = ( val - A ) / ( B - A );
	cVal = std::clamp<float>( cVal, 0.0f, 1.0f );
	return C + ( D - C ) * SimpleSpline( cVal );
}
float ent_info_t::solve_velocity_len(std::shared_ptr<player_record_t> record) {
	auto weapon = ( weapon_t* )g.m_interfaces->entity_list( )->get_client_entity_handle( record->m_ent->active_weapon( ) );
	float max_speed = 260.0f;
	auto info = weapon ? g.m_interfaces->weapon_system( )->get_weapon_data( weapon->item_definition_index( ) ) : nullptr;
	if ( info ) {
		max_speed = fmaxf(0.001f, info->m_max_player_speed);
	}

	float comp_value = record->m_layers[ ANIMATION_LAYER_ALIVELOOP ].m_weight;
	if ( comp_value == 0.f )
		return -1;
	float best = -1;
	float closest = -1;
	
	for ( float i = 0.f; i < max_speed; i += 0.5f ) 	{
		float val = SimpleSplineRemapValClamped( i / max_speed, 0.55f, 0.9f, 1.0f, 0.f );
		if ( closest == -1 || fabsf( comp_value - val ) < best ) 			{
			best = fabsf( comp_value - val );
			closest = i;
		}
	}

	return closest;




	auto state = record->m_ent->get_anim_state();
	if (!state)
		return record->m_anim_velocity.length();
	float cycle_dif = record->m_layers[6].m_playback_rate;
	if (cycle_dif == 0.f) {
		if (record->m_layers[6].m_cycle >= 0.999f) {
			return (record->m_anim_velocity.length_2d_sqr() != 0.f) ? record->m_anim_velocity.length_2d() : 0.f;
		}
		return 0.f;
	}

	float flMoveCycleRate = cycle_dif / g.ticks_to_time(record->m_lag);
	float walk_to_run_max = 1;
	float walk_to_run_min = 0.f;
	float walk_to_run = 0;
	bool switch_1 = false;
	static auto sv_friction = g.m_interfaces->console()->get_convar("sv_friction");
	const auto friction = sv_friction->GetFloat() * g.m_local->surface_friction();
	//while (fabsf(walk_to_run_max - walk_to_run_min) > 0.00001) {
	//	float test = ((walk_to_run_max - walk_to_run_min) / 2) + walk_to_run_min;
	//
	//	float min_dif = fabsf(Lerp(walk_to_run_min, 1.f, 0.14999998f * *(float*)(state + 284)) - flMoveCycleRate);
	//	float max_dif = fabsf(Lerp(walk_to_run_max, 1.f, 0.14999998f * *(float*)(state + 284)) - flMoveCycleRate);
	//
	//	if (min_dif > max_dif) {
	//		walk_to_run = test;
	//		walk_to_run_min = test;
	//	}
	//	else if (min_dif < max_dif) {
	//		walk_to_run = test;
	//		walk_to_run_max = test;
	//	}
	//	else {
	//		float dif = fabsf(Lerp(test, 1.f, 0.14999998f * *(float*)(state + 284)) - flMoveCycleRate);
	//		float test_2 = ((walk_to_run_max - test) / 2) + walk_to_run_min;
	//
	//		float dif_2 = fabsf(Lerp(test_2, 1.f, 0.14999998f * *(float*)(state + 284)) - flMoveCycleRate);
	//
	//		if (dif > dif_2) {
	//			walk_to_run = test_2;
	//			walk_to_run_min = test;
	//			continue;
	//		}
	//
	//		test_2 = ((test - walk_to_run_min) / 2) + test;
	//		float idk = flMoveCycleRate / Lerp(test_2, 1.f, 0.14999998f * *(float*)(state + 284));
	//		dif_2 = fabsf(Lerp(test_2, 1.f, 0.14999998f * *(float*)(state + 284) ) - flMoveCycleRate);
	//
	//		if (dif > dif_2) {
	//			walk_to_run = test_2;
	//			walk_to_run_max = test;
	//			continue;
	//		}
	//		break;
	//	}
	//}
	//float best_2 = -1;
	//float test_2 = walk_to_run_min;
	//while (test_2 <= walk_to_run_max)
	//{
	//	float val = fabsf(Lerp(test_2, 1.f, 0.14999998f * *(float*)(state + 284)) - flMoveCycleRate);
	//	if (best_2 == -1 || val < best_2) {
	//		walk_to_run = test_2;
	//		best_2 = val;
	//	}
	//	test_2 += 0.000005f;
	//}

	//auto state = record->m_ent->get_anim_state();

	//float m_flAnimDuckAmount = std::clamp(Approach(std::clamp(record->m_duck + *(float*)(state + 0xA8), 0.f, 1.f), *(float*)(state + 164), g.ticks_to_time(record->m_lag) * 6.0f), 0.f, 1.f);

	//record->m_poses[(int)PoseParam_t::MOVE_BLEND_WALK] = (1.0f - walk_to_run) * (1.0f - m_flAnimDuckAmount);
	//record->m_poses[(int)PoseParam_t::MOVE_BLEND_RUN] = (walk_to_run) * (1.0f - m_flAnimDuckAmount);
	//record->m_poses[(int)PoseParam_t::MOVE_BLEND_CROUCH] = m_flAnimDuckAmount;

	auto hdr = record->m_ent->GetModelPtr();
	if (!hdr) {
		return (record->m_anim_velocity.length_2d_sqr() != 0.f) ? record->m_anim_velocity.length_2d() : 0.f;
	}
	//if (flMoveCycleRate == 0)
	//	return 0;
	//return 1 / flMoveCycleRate;

	int nWeaponMoveSeq = record->m_layers[6].m_sequence;

	float m_flWalkToRunTransition = *(float*)(record->m_ent->get_anim_state() + 284);

	bool set = (*(BYTE*)(record->m_ent->get_anim_state() + 308)) == 1;
	if (m_flWalkToRunTransition > 0 && m_flWalkToRunTransition < 1)
	{
		//currently transitioning between walk and run
		if (set)
		{
			m_flWalkToRunTransition += g.ticks_to_time(record->m_lag) * 2.f;
		}
		else // m_bWalkToRunTransitionState == ANIM_TRANSITION_RUN_TO_WALK
		{
			m_flWalkToRunTransition -= g.ticks_to_time(record->m_lag) * 2.f;
		}
		m_flWalkToRunTransition = std::clamp(m_flWalkToRunTransition, 0.f, 1.f);
	}

	if (!set)
		m_flWalkToRunTransition = fmaxf(0.99f, m_flWalkToRunTransition);

	float m_flAnimDuckAmount = std::clamp(Approach(std::clamp(record->m_duck + *(float*)(record->m_ent->get_anim_state() + 168), 0.f, 1.f), *(float*)(record->m_ent->get_anim_state() + 164), g.ticks_to_time(record->m_lag) * 6.0f), 0.f, 1.f);

	record->m_poses[(int)PoseParam_t::MOVE_BLEND_WALK] = (1.0f - m_flWalkToRunTransition) * (1.0f - m_flAnimDuckAmount);
	record->m_poses[(int)PoseParam_t::MOVE_BLEND_RUN] = (m_flWalkToRunTransition) * (1.0f - m_flAnimDuckAmount);
	record->m_poses[(int)PoseParam_t::AIM_BLEND_CROUCH_WALK] = m_flAnimDuckAmount;

	vec3_t pVec1;
	pVec1 = record->m_ent->GetSequenceLinearMotion(hdr, nWeaponMoveSeq, record->m_poses);
	for (auto i = 0; i < 3; i++)
		if (isnan(fabsf(pVec1[i])))
			return (record->m_anim_velocity.length_2d_sqr() != 0.f) ? record->m_anim_velocity.length_2d() : 0.f;

	float m_flWalkToRunTransition_1 = *(float*)(record->m_ent->get_anim_state() + 284);

	set = (*(BYTE*)(record->m_ent->get_anim_state() + 308)) == 1;
	if (m_flWalkToRunTransition_1 > 0 && m_flWalkToRunTransition_1 < 1)
	{
		//currently transitioning between walk and run
		if (set)
		{
			m_flWalkToRunTransition_1 += g.ticks_to_time(record->m_lag) * 2.f;
		}
		else // m_bWalkToRunTransitionState == ANIM_TRANSITION_RUN_TO_WALK
		{
			m_flWalkToRunTransition_1 -= g.ticks_to_time(record->m_lag) * 2.f;
		}
		m_flWalkToRunTransition_1 = std::clamp(m_flWalkToRunTransition_1, 0.f, 1.f);
	}
	if (set)
		m_flWalkToRunTransition_1 = fmaxf(0.01f, m_flWalkToRunTransition_1);

	record->m_poses[(int)PoseParam_t::MOVE_BLEND_WALK] = (1.0f - m_flWalkToRunTransition_1) * (1.0f - m_flAnimDuckAmount);
	record->m_poses[(int)PoseParam_t::MOVE_BLEND_RUN] = (m_flWalkToRunTransition_1) * (1.0f - m_flAnimDuckAmount);

	vec3_t pVec2;
	pVec2 = record->m_ent->GetSequenceLinearMotion(hdr, nWeaponMoveSeq, record->m_poses);
	for (auto i = 0; i < 3; i++)
		if (isnan(fabsf(pVec1[i])))
			return (record->m_anim_velocity.length_2d_sqr() != 0.f) ? record->m_anim_velocity.length_2d() : 0.f;

	float pVec_len = pVec1.length();
	float pVec1_len = pVec2.length();

	float vel_x_y = 0.f;
	//float min_x_y = 0.1f - FLT_MIN;
	float max_x_y = 300.f;
	double test_1 = 0.1 - 0.01;
	while (test_1 <= max_x_y)
	{
		float w = flMoveCycleRate / (float)(1.0 - (float)((test_1 > 135.2) ? m_flWalkToRunTransition : m_flWalkToRunTransition_1) * 0.14999998);
		float val = test_velocity(record, test_1, (test_1 > 135.2) ? pVec1_len : pVec_len);
		if (val != -1) {
			const auto comp = fabsf(val - w);

			if (best == -1 || comp < best) {
				vel_x_y = test_1;
				best = comp;
			}
		}

		test_1 += 0.01;
	}

	//while (true) {
	//	test_1 = (max_x_y - min_x_y) / 2;
	//	test_1 += min_x_y;
	//	float test_2[2] = { (max_x_y - test_1) / 2, (test_1 - min_x_y) / 2 };
	//	test_2[0] += test_1;
	//	test_2[1] += min_x_y;
	//
	//	float val = test_velocity(record, test_1, (test_1 > 135.2) ? pVec1_len : pVec_len);
	//	if( val == -1 )
	//		return (record->m_anim_velocity.length_2d_sqr() != 0.f) ? record->m_anim_velocity.length_2d() : 0.f;
	//
	//	float comp = fabsf(val - w);
	//	if (comp <= 0.001f)
	//		return test_1;
	//
	//	val = test_velocity(record, test_2[0], (test_2[0] > 135.2) ? pVec1_len : pVec_len);
	//	if (val == -1)
	//		return (record->m_anim_velocity.length_2d_sqr() != 0.f) ? record->m_anim_velocity.length_2d() : 0.f;
	//
	//	if (fabsf(val - w) < comp) {
	//		min_x_y = test_1;
	//	}
	//	else {
	//		val = test_velocity(record, test_2[1], (test_2[1] > 135.2) ? pVec1_len : pVec_len);
	//		if (val == -1)
	//			return (record->m_anim_velocity.length_2d_sqr() != 0.f) ? record->m_anim_velocity.length_2d() : 0.f;
	//		if (fabsf(val - w) < comp)
	//		{
	//			max_x_y = test_1;
	//		}
	//		else {
	//			return test_1;
	//		}
	//	}
	//}

	if(best == -1)
		return (record->m_anim_velocity.length_2d_sqr() != 0.f) ? record->m_anim_velocity.length_2d() : 0.f;
	return vel_x_y;

	//while (max_x_y - min_x_y > 1) {
	//	float flSequenceGroundSpeed = 0.001f;
	//	float speed_est = ((max_x_y - min_x_y) / 2) + min_x_y;
	//	//float comp = fabsf((flMoveCycleRate / test(speed_est, pVec)) - speed_est);
	//
	//	float min_dif = fabsf((flMoveCycleRate / test(max_x_y, pVec)) - max_x_y);
	//	float max_dif = fabsf((flMoveCycleRate / test(min_x_y, pVec)) - min_x_y);
	//
	//	if (min_dif > max_dif) {
	//		min_x_y = speed_est;
	//		vel_x_y = speed_est;
	//	}
	//	else if (min_dif < max_dif) {
	//		vel_x_y = speed_est;
	//		max_x_y = speed_est;
	//	}
	//	else {
	//		float dif = fabsf((flMoveCycleRate / test(speed_est, pVec)) - speed_est);
	//		float new_speed = ((max_x_y - speed_est) / 2) + speed_est;
	//		float dif_2 = fabsf((flMoveCycleRate / test(new_speed, pVec)) - new_speed);
	//
	//		if (dif > dif_2) {
	//			min_x_y = speed_est; 
	//			vel_x_y = new_speed;
	//			continue;
	//		}
	//
	//		new_speed = ((speed_est - min_x_y) / 2) + min_x_y;
	//		dif_2 = fabsf((flMoveCycleRate / test(new_speed, pVec)) - max_x_y);
	//
	//		if (dif > dif_2) {
	//			max_x_y = speed_est;
	//			vel_x_y = new_speed;
	//			continue;
	//		}
	//		break;
	//	}
	//
	//
	//}
}

void ent_info_t::UpdateAnimations( std::shared_ptr<player_record_t> record ) {
	auto* state = m_ent->get_anim_state( );
	if ( !state )
		return;

	// player respawned.
	if ( m_ent->spawn_time( ) != m_spawn ) {
		m_ik.init( );
		// reset animation state.
		state->ResetAnimationState( );

		// note new spawn time.
		m_spawn = m_ent->spawn_time( );

		for ( auto& i : m_resolver_data.m_states ) {
			memcpy( &i, state, sizeof( anim_state ) );
		}
	}


	// backup stuff that we do not want to fuck with.
	animation_backup_t backup;

	backup.m_origin = m_ent->origin( );
	backup.m_abs_origin = m_ent->abs_origin( );
	backup.m_velocity = m_ent->velocity( );
	backup.m_abs_velocity = m_ent->abs_vel( );
	backup.m_flags = m_ent->flags( );
	backup.m_eflags = m_ent->iEFlags( );
	backup.m_duck = m_ent->duck_amount( );
	backup.m_body = m_ent->lower_body_yaw( );
	m_ent->GetAnimLayers( backup.m_layers );
	m_ent->GetPoseParameters( backup.m_poses );

	const auto bot = m_fake_player;

	// fix velocity.
	// https://github.com/VSES/SourceEngine2007/blob/master/se2007/game/client/c_baseplayer.cpp#L659
	if ( record->m_lag > 0 && record->m_lag < 16 && m_records.size( ) >= 2 ) {
		// get pointer to previous record.
		auto const &previous = m_records[ 1 ];
	
		if ( previous && !previous->m_dormant )
			record->m_velocity = ( record->m_origin - previous->m_origin ) * ( 1.f / g.ticks_to_time( record->m_lag ) );
	}
	// set this fucker, it will get overriden.
	record->m_anim_velocity = record->m_velocity;

	// fix various issues with the game eW91dHViZS5jb20vZHlsYW5ob29r
	// these issues can only occur when a player is choking data.
	//if ((record->m_flags & fl_onground) && !m_teamate) {
		g.m_interfaces->mdlcache()->begin_lock(); 
		vec3_t w;
		if (record->m_anim_velocity.length_2d_sqr() != 0)
			w = record->m_anim_velocity.normalized();
		else
			w = { 0.5,0.5, 0 };
		float speed = solve_velocity_len(record);
		if( speed >= 0.f )
			record->m_anim_velocity = vec3_t(w.x * speed, w.y * speed, record->m_anim_velocity.z);
		g.m_interfaces->mdlcache()->end_lock();
	//}
	if ( record->m_lag > 1 && !bot ) {
		auto speed = record->m_velocity.length( );		
		if (speed > 0.1f
			&& record->m_layers[ 6 ].m_weight == 0.0f
			&& record->m_layers[ 6 ].m_playback_rate < 0.0001f
			&& record->m_layers[ 12 ].m_weight > 0.0f
			&& speed < 10.f
			&& (record->m_flags & fl_onground))
			record->m_fake_walk = true;

		// detect players abusing micromovements or other trickery
		//record->m_fake_walk = record->m_layers[6].m_playback_rate == 0.f;
		if( record->m_fake_walk )
			record->m_anim_velocity = vec3_t(0,0,0);
		// we need atleast 2 updates/records
		// to fix these issues.
		if ( m_records.size( ) >= 2 && m_records[ 1 ] ) {
			if (speed < 20.f
				&& record->m_layers[6].m_weight != 1.0f
				&& record->m_layers[6].m_weight != 0.0f
				&& fabsf(record->m_layers[6].m_weight - m_records[1]->m_layers[6].m_weight) > 0.01f
				&& (record->m_flags & fl_onground))
				record->m_ukn_vel = true;
			if( record->m_ukn_vel )
				record->m_anim_velocity = vec3_t(0, 0, 0);
			// get pointer to previous record.
			auto const &previous = m_records[ 1 ];

			if ( previous && !previous->m_dormant ) {
				// strip the on ground flag.
				m_ent->flags( ) &= ~fl_onground;

				// been onground for 2 consecutive ticks? fuck yeah.
				if ( record->m_flags & fl_onground && previous->m_flags & fl_onground )
					m_ent->flags( ) |= fl_onground;

				// fix jump_fall.
				if ( record->m_layers[ 4 ].m_weight != 1.f && previous->m_layers[ 4 ].m_weight == 1.f && record->m_layers[ 5 ].m_weight != 0.f )
					m_ent->flags( ) |= fl_onground;

				if ( record->m_flags & fl_onground && !( previous->m_flags & fl_onground ) )
					m_ent->flags( ) &= ~fl_onground;

				// fix crouching players.
				// the duck amount we receive when people choke is of the last simulation.
				// if a player chokes packets the issue here is that we will always receive the last duckamount.
				// but we need the one that was animated.
				// therefore we need to compute what the duckamount was at animtime.

				// delta in duckamt and delta in time..
				const auto duck = record->m_duck - previous->m_duck;

				// fix crouching players.
				m_ent->duck_amount( ) = std::clamp<float>(previous->m_duck + std::copysignf(previous->m_duck_speed*g.m_interfaces->globals()->m_interval_per_tick, duck), 0.f, 1.f);
			}
		}
	}

	// set stuff before animating.
	m_ent->origin( ) = record->m_origin;
	m_ent->velocity( ) = m_ent->abs_vel( ) = record->m_anim_velocity;
	m_ent->lower_body_yaw( ) = record->m_body;

	// write potentially resolved angles.
	g_resolver.resolve( *this, record );

	//for ( auto i = 0; i < info->m_lag; i++ ) {
	//
	//	//console::log(("cycle " + std::to_string(record->m_layers[6].m_cycle) + "\n").c_str());
	//	//console::log(("m_playback_rate " + std::to_string(info->m_layers[6].m_playback_rate) + "\n").c_str());
	//	//console::log(("sequence " + std::to_string(info->m_layers[6].m_sequence) + "\n").c_str());
	//	//console::log(("m_weight " + std::to_string(info->m_layers[6].m_weight) + "\n").c_str());
	//	//console::log(("yaw " + std::to_string(record->m_eye_angles.y) + "\n").c_str());
	//
	//	// 'm_animating' returns true if being called from SetupVelocity, passes raw velocity to animstate.
	//
	m_ent->eye_angles( ) = record->m_eye_angles;
	m_ent->client_side_anim( ) = true;
	if (!m_teamate) {
		state->feetYawRate = 0.f;

		resolver_data::mode_data* mode_data = nullptr;
		bool lby_or_move = ( !record->m_body_reliable && record->m_mode == resolver::RESOLVE_BODY ) || record->m_mode == resolver::RESOLVE_WALK;
		if ( lby_or_move ) {
			mode_data = &m_resolver_data.m_mode_data[ resolver_data::LBY_MOVING ];
			float val = 35.f;
			if ( record->m_mode == resolver::RESOLVE_WALK )
				val = ( 30.f + 20.0f * state->m_flWalkToRunTransition ) * g.ticks_to_time( record->m_lag );

			record->m_resolver_data.m_dir_data.emplace_back( record->m_body );
			record->m_resolver_data.m_dir_data.emplace_back( record->m_body + val );
			record->m_resolver_data.m_dir_data.emplace_back( record->m_body - val );
		}
		else if ( record->m_mode == resolver::RESOLVE_STAND1 )
			mode_data = &m_resolver_data.m_mode_data[ resolver_data::STAND1 ];
		else if ( record->m_mode == resolver::RESOLVE_STAND2 )
			mode_data = &m_resolver_data.m_mode_data[ resolver_data::STAND2 ];

		anim_state backup_anim_state{};
		std::memcpy( &backup_anim_state, state, sizeof( anim_state ) );
		anim_state* index_state = nullptr;
		if ( mode_data && record->m_resolver_data.m_dir_data.size() == mode_data->m_dir_data.size( ) ) {
			auto new_index = 0;
			for ( auto i = 0; i < mode_data->m_dir_data.size(); i++ ) {
				if ( m_resolver_data.m_states[ i ].m_frame >= g.m_interfaces->globals( )->m_framecount )
					m_resolver_data.m_states[ i ].m_frame = g.m_interfaces->globals( )->m_framecount - 1;
				auto& record_dir_data = record->m_resolver_data.m_dir_data[ i ];
				std::memcpy( state, &m_resolver_data.m_states[ i ], sizeof( anim_state ) );
				state->feetYawRate = 0.f;
				m_ent->eye_angles( ).y = math::normalize_angle( record_dir_data.angles, 180 );
				{
					std::unique_lock<std::mutex> lock( g_thread_handler.queue_mutex );
					// backup curtime.
					const auto curtime = g.m_interfaces->globals( )->m_curtime;
					const auto frametime = g.m_interfaces->globals( )->m_frametime;

					g.m_interfaces->globals( )->m_curtime = record->m_anim_time;
					g.m_interfaces->globals( )->m_frametime = g.m_interfaces->globals( )->m_curtime;

					m_ent->update_client_side_animation( );

					g.m_interfaces->globals( )->m_curtime = curtime;
					g.m_interfaces->globals( )->m_frametime = frametime;
				}
				m_ent->GetPoseParameters( record_dir_data.poses );
				record_dir_data.m_abs_angles = m_ent->abs_angles( );
				std::memcpy( &m_resolver_data.m_states[ i ], state, sizeof( anim_state ) );
				if ( mode_data->m_index == i ) {
					record->m_abs_angles = m_ent->abs_angles( );
					index_state = &m_resolver_data.m_states[ i ];
					memcpy(record->m_poses, record_dir_data.poses, sizeof( float) * 24);
				}
				memcpy( state, &backup_anim_state, sizeof( anim_state ) );
				m_ent->SetPoseParameters( backup.m_poses );
				m_ent->SetAnimLayers( backup.m_layers );
			}
		}
		if ( index_state )
			std::memcpy( state, index_state, sizeof( anim_state ) );
		else {
			m_ent->eye_angles( ) = record->m_eye_angles;
			state->feetYawRate = 0.f;
			//m_ent->get_anim_state( )->m_goal_feet_yaw = m_ent->eye_angles( ).y;
			if ( m_ent->get_anim_state( )->m_frame >= g.m_interfaces->globals( )->m_framecount )
				m_ent->get_anim_state( )->m_frame = g.m_interfaces->globals( )->m_framecount - 1;
			{
				std::unique_lock<std::mutex> lock( g_thread_handler.queue_mutex );
				// backup curtime.
				const auto curtime = g.m_interfaces->globals( )->m_curtime;
				const auto frametime = g.m_interfaces->globals( )->m_frametime;

				g.m_interfaces->globals( )->m_curtime = record->m_anim_time;
				g.m_interfaces->globals( )->m_frametime = g.m_interfaces->globals( )->m_curtime;

				m_ent->update_client_side_animation( );

				g.m_interfaces->globals( )->m_curtime = curtime;
				g.m_interfaces->globals( )->m_frametime = frametime;
			}
			record->m_abs_angles = m_ent->abs_angles( );
			// store updated/animated poses and rotation in lagrecord.
			m_ent->GetPoseParameters( record->m_poses );
		}
	}
	else {
		m_ent->eye_angles( ) = record->m_eye_angles;
		state->feetYawRate = 0.f;
		//m_ent->get_anim_state( )->m_goal_feet_yaw = m_ent->eye_angles( ).y;
		if ( m_ent->get_anim_state( )->m_frame >= g.m_interfaces->globals( )->m_framecount )
			m_ent->get_anim_state( )->m_frame = g.m_interfaces->globals( )->m_framecount - 1;
		{
			std::unique_lock<std::mutex> lock( g_thread_handler.queue_mutex );
			// backup curtime.
			const auto curtime = g.m_interfaces->globals( )->m_curtime;
			const auto frametime = g.m_interfaces->globals( )->m_frametime;

			g.m_interfaces->globals( )->m_curtime = record->m_anim_time;
			g.m_interfaces->globals( )->m_frametime = g.m_interfaces->globals( )->m_curtime;

			m_ent->update_client_side_animation( );

			g.m_interfaces->globals( )->m_curtime = curtime;
			g.m_interfaces->globals( )->m_frametime = frametime;
		}
		record->m_abs_angles = m_ent->abs_angles( );
		// store updated/animated poses and rotation in lagrecord.
		m_ent->GetPoseParameters( record->m_poses );
	}
	m_ent->client_side_anim( ) = false;
	//}


	// restore backup data.
	m_ent->origin( ) = backup.m_origin;
	m_ent->velocity( ) = backup.m_velocity;
	m_ent->abs_vel( ) = backup.m_abs_velocity;
	m_ent->flags( ) = backup.m_flags;
	m_ent->iEFlags( ) = backup.m_eflags;
	m_ent->duck_amount( ) = backup.m_duck;
	m_ent->lower_body_yaw( ) = backup.m_body;
	m_ent->set_abs_origin( backup.m_abs_origin );
	m_ent->SetAnimLayers( backup.m_layers );
	m_ent->SetPoseParameters( backup.m_poses );
}

void ent_info_t::update( player_t *ent ) {
	m_ent = ent;
	m_teamate = ent->on_team( g.m_local );

	m_dormant = ent->dormant( );


	if ( m_dormant ) {
		m_walk_record.m_sim_time = 0.0f;
		auto insert = true;

		// we have any records already?
		if ( !m_records.empty( ) ) {

			auto const front = m_records.front( );

			// we already have a dormancy separator.
			if ( front && front->m_dormant )
				insert = false;
		}

		if ( insert ) {
			// add new record.
			m_records.emplace_front( std::make_shared< player_record_t >( this, 0 ) )->m_dormant = true;
		}
	}

	float last_sim = 0;
	auto update = m_records.empty( ) || !m_records[0];
	if ( !update ) {
		last_sim = m_records.front( )->m_sim_time;
		update = last_sim < ent->sim_time( );
	}

	if ( update ) {
		m_setup = false;

		//g_thread_handler.queue_mutex.lock( );
		m_records.emplace_front( std::make_shared< player_record_t >( this, last_sim ) );
		//g_thread_handler.queue_mutex.unlock( );

		auto &current = m_records.front( );
		current->m_dormant = false;
		UpdateAnimations( current );
		
		if ( !m_teamate ) {
			animation_layer_t backup_layers[ 15 ];
			memcpy( backup_layers, current->m_layers, sizeof( current->m_layers ) );

			if ( ((!current->m_body_reliable && current->m_mode == resolver::RESOLVE_BODY) || current->m_mode == resolver::RESOLVE_WALK) || ( current->m_mode == resolver::RESOLVE_STAND1 || current->m_mode == resolver::RESOLVE_STAND2 )) {
				m_setup = build_fake_bones( current );
			}
			else {
				current->m_ent->SetAnimLayers( current->m_layers );
				m_setup = g_bones.setup( m_ent, current->m_bones, current, &m_ik );
			}
			current->m_ent->SetAnimLayers( backup_layers );
		}
	}

	
	while ( m_records.size( ) > 256 )
		m_records.pop_back( );
}

class plThreadObject {
public:
	bool ret_state = false;
	bool read = false;
	ent_info_t* info;
	player_t* player;
	bool finished = false;
	void run( );
	plThreadObject( ent_info_t* info_, player_t* player_ ) {
		info = info_;
		player = player_;
	}
};

void plThreadObject::run ( ) {
	info->update( player );
	finished = true;
}

void update_player() {
	
}

void player_manager_t::update ( ) {
	if ( !g.m_local )
		return;

	g_resolver.update_shots( );
	m_animating = true;
	std::vector<std::shared_ptr<plThreadObject>> objects = {};
	g_thread_handler.start( );
	for ( auto i{ 1 }; i <= g.m_interfaces->globals(  )->m_max_clients; ++i ) {


		auto* const player = static_cast< player_t* >( g.m_interfaces->entity_list( )->get_client_entity( i ) );
		auto* data = &m_ents[ i - 1 ];
		data->m_valid = player && player != g.m_local && player->is_player( ) && player->alive( );
		if ( !data->m_valid ) {
			data->m_records.clear( );
			data->m_ent = nullptr;
			continue;
		}

		if ( data->m_ent != player ) {
			data->m_resolver_data.init( );
			data->m_records.clear( );
			//engine_player_info_t info{};
			//g.m_interfaces->engine( )->get_player_info( player->index( ), &info );
			//data->m_fake_player = info.fakeplayer;
		}
		{
			auto ptr = objects.emplace_back(std::make_shared<plThreadObject>( &m_ents[ i - 1 ], player ) );
			g_thread_handler.QueueJob( [ ptr ] {
				ptr->run( );
				});
		}
		
		//data->update( player );
	}
	while ( g_thread_handler.busy( ) );

	g_thread_handler.stop( );
	bool all_finished = true;
	while ( !all_finished ) {
		all_finished = true;
		for ( auto& i : objects )
			if ( !i->finished )
				all_finished = false;
	}
	//g_thread_handler.start( );
	//g_thread_handler.mutex_condition.notify_all( );
	//
	//while ( g_thread_handler.busy( ) );
	//
	//{
	//	g_thread_handler.queue_mutex2.lock( );
	//	g_thread_handler.objects.clear( );
	//	g_thread_handler.queue_mutex2.unlock( );
	//}
	//
	m_animating = false;
}

bool ent_info_t::build_fake_bones( std::shared_ptr<player_record_t> current ) {
	resolver_data::mode_data* mode_data = nullptr;

	if ( current->m_mode == resolver::RESOLVE_STAND1 )
		mode_data = &m_resolver_data.m_mode_data[ resolver_data::STAND1 ];
	else if ( current->m_mode == resolver::RESOLVE_STAND2 )
		mode_data = &m_resolver_data.m_mode_data[ resolver_data::STAND2 ];
	else if ( ( ( !current->m_body_reliable && current->m_mode == resolver::RESOLVE_BODY ) || current->m_mode == resolver::RESOLVE_WALK ) )
		mode_data = &m_resolver_data.m_mode_data[ resolver_data::LBY_MOVING ];

	if ( !mode_data )
		return false;

	const auto backup_ang = current->m_abs_angles;
	float backup_poses[ 24 ];
	memcpy( backup_poses, current->m_poses, sizeof( current->m_poses ) );

	bool all_setup = true;

	int bone_ix = 0;
	for ( auto i = 0; i < current->m_resolver_data.m_dir_data.size(); i++ ) {
		auto& record_dir_data = current->m_resolver_data.m_dir_data[ i ];
		memcpy( current->m_poses, record_dir_data.poses, sizeof( current->m_poses ) );
		//memcpy( current->m_layers, current->m_fake_layers[ i ], sizeof( animation_layer_t ) * 13 );
		//memcpy( current->m_layers, backup_layers, sizeof( animation_layer_t ) * 13 );
		current->m_abs_angles = record_dir_data.m_abs_angles;
		if ( i ==  mode_data->m_index ) {
			{
				if ( !g_bones.setup( current->m_ent, current->m_bones, current, &m_ik ) )
					all_setup = false;
			}
		}
		else {
			{
				if ( !g_bones.setup( current->m_ent, current->m_fake_bones[ bone_ix ], current, &m_ik ) )
					all_setup = false;
			}
			bone_ix++;
		}
		//memcpy( &m_ik, &backup_context, sizeof( CIKContext ) );
	}

	current->m_abs_angles = backup_ang;
	memcpy( current->m_poses, backup_poses, sizeof( current->m_poses ) );
	return all_setup;
}

void backup_record_t::store( player_t* player ) {
	// get bone cache ptr.
	bone_cache_t* cache = &player->bone_cache( );

	// store bone data.
	m_bones = cache->m_pCachedBones;
	m_bone_count = cache->m_CachedBoneCount;
	m_origin = player->origin( );
	m_mins = player->mins( );
	m_maxs = player->maxs( );
	m_abs_origin = player->abs_origin( );
	m_abs_ang = player->abs_angles( );
}

void backup_record_t::restore( player_t* player ) const {
	// get bone cache ptr.
	bone_cache_t* cache = &player->bone_cache( );

	cache->m_pCachedBones = m_bones;
	cache->m_CachedBoneCount = m_bone_count;

	player->origin( ) = m_origin;
	player->mins( ) = m_mins;
	player->maxs( ) = m_maxs;
	player->set_abs_angles( m_abs_ang );
	player->set_abs_origin( m_abs_origin );
}
