#include "player_manager.h"

#include "bones.h"
#include "prediction.h"
#include "resolver.h"

player_record_t::~player_record_t()
{
    // free heap allocated game mem.
    g.m_interfaces->mem_alloc()->free(m_bones);
    for (auto pObj = m_fake_bones.begin(); pObj != m_fake_bones.end(); ++pObj)
        g.m_interfaces->mem_alloc()->free(*pObj);
    m_fake_bones.clear();
    m_resolver_data.m_dir_data.clear();
}

void player_record_t::cache(int index) const
{
    // get bone cache ptr.
    auto* const cache = &m_ent->bone_cache();

    if (index == -1)
        cache->m_pCachedBones = m_bones;
    else
        cache->m_pCachedBones = m_fake_bones[index];
    cache->m_CachedBoneCount = 128;

    m_ent->last_bone_setup_time() = FLT_MAX;
    m_ent->origin() = m_pred_origin;
    m_ent->mins() = m_mins;
    m_ent->maxs() = m_maxs;

    m_ent->set_abs_angles(m_abs_angles);
    m_ent->set_abs_origin(m_pred_origin);
}

bool player_record_t::valid()
{
    if (!g.m_interfaces->client_state()->m_NetChannel)
        return false;
    // use prediction curtime for this.
    float curtime = g.ticks_to_time(g.m_local->tick_base());

    // correct is the amount of time we have to correct game time,
    float correct = g.m_lerp;

    correct += g.m_interfaces->client_state()->m_NetChannel->GetLatency(1);

    if (floorf(curtime - 1.f) > m_pred_time)
        return false;
    // check bounds [ 0, sv_maxunlag ]
    correct = std::clamp<float>(correct, 0.f, 1.f);

    // calculate difference between tick sent by player and our latency based
    // tick. ensure this record isn't too old.
    return std::abs(correct - (curtime - m_pred_time)) < 0.19f;
}

// bool player_record_t::valid() const {
//	if (!m_setup)
//		return false;
//	// use prediction curtime for this.
//	static auto* sv_maxunlag =
// g.m_interfaces->console()->get_convar("sv_maxunlag"); 	auto* net =
// g.m_interfaces->engine()->get_net_channel_info();
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
//	//// calculate difference between tick sent by player and our latency
// based tick.
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
//	const auto arrival_tick = g.m_local->tick_base() +
// g.time_to_ticks(avg_latency) + tick_delay;
//
//	const auto correct = std::clamp( g.m_lerp + net->GetLatency( 0 ), 0.f,
// sv_maxunlag->GetFloat( ) ) - g.ticks_to_time( arrival_tick + g.time_to_ticks(
// g.m_lerp ) - tickcount );
//
//	return std::abs( correct ) < 0.2f - g.m_interfaces->globals(
//)->m_interval_per_tick;
//}

player_record_t::player_record_t(ent_info_t* info, float last_sim)
{
    m_info = info;
    m_ent = info->m_ent;

    m_pred_time = m_sim_time = info->m_ent->sim_time();
    m_old_sim_time = info->m_ent->old_sim_time();
    m_anim_time = m_old_sim_time + g.m_interfaces->globals()->m_interval_per_tick;
    m_lag = g.time_to_ticks(m_sim_time - m_old_sim_time);
    m_pred_origin = m_origin = info->m_ent->origin();
    m_abs_origin = info->m_ent->abs_origin();
    m_eye_angles = info->m_ent->eye_angles();
    m_maxs = info->m_ent->maxs();
    m_mins = info->m_ent->mins();
    m_duck = info->m_ent->duck_amount();
    m_duck_speed = info->m_ent->duck_speed();
    m_pred_flags = m_flags = info->m_ent->flags();
    m_setup = false;
    m_pred_velocity = m_velocity = info->m_ent->velocity();
    m_body = info->m_ent->lower_body_yaw();

    m_tick = g.m_interfaces->client_state()->m_ClockDriftMgr.m_nServerTick;
    m_ent->GetAnimLayers(m_layers);
    g.m_interfaces->mdlcache()->begin_lock();
    feet_cycle = m_layers[6].m_cycle;
    feet_yaw_rate = m_layers[6].m_weight;
    {
        const auto hdr = m_ent->GetModelPtr();
        for (auto& m_layer : m_layers) {
            m_layer.m_owner = m_ent;
            m_layer.m_pDispatchedStudioHdr = hdr;
        }
    }
    g.m_interfaces->mdlcache()->end_lock();
    m_ent->GetPoseParameters(m_poses);
    m_cycle = m_ent->cycle();
    m_sequence = m_ent->sequence();

    m_bones = static_cast<bone_array_t*>(g.m_interfaces->mem_alloc()->alloc(sizeof(bone_array_t) * 128));
    // if (m_lag <= 0)
    //     g.m_interfaces->console()->console_printf("Exploit?\n");
}

template <class T> T Lerp(float flPercent, T const& A, T const& B)
{
    return A + ((B - A) * flPercent);
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
float InvSqrt(float x)
{
    float xhalf = 0.5f * x;
    int i = *(int*)&x;              // store floating-point bits in integer
    i = 0x5f3759df - (i >> 1);      // initial guess for Newton's method
    x = *(float*)&i;                // convert new bits into float
    x = x * (1.5f - xhalf * x * x); // One round of Newton's method
    return x;
}
float ent_info_t::test_velocity(std::shared_ptr<player_record_t> record, float speed, float pVec)
{
    auto weapon = (weapon_t*)g.m_interfaces->entity_list()->get_client_entity_handle(record->m_ent->active_weapon());
    float max_speed = record->m_ent->max_speed();
    auto info = weapon ? g.m_interfaces->weapon_system()->get_weapon_data(weapon->item_definition_index()) : nullptr;
    if (info)
    {
        max_speed = info->m_max_player_speed;
    }

    float flMoveCycleRate = 1.f / speed;
    float flSequenceGroundSpeed = 0.001f;

    float ukn = 1.0 / (float)(1.f / flMoveCycleRate);
    if ((pVec * (float)(1.f / (float)(1.0f / ukn)), 2) > 0.001f)
    {
        flSequenceGroundSpeed = powf(pVec, 2.f) * ukn;
    }
    flSequenceGroundSpeed = fmaxf(flSequenceGroundSpeed, 0.001f);
    return (speed / flSequenceGroundSpeed) * flMoveCycleRate;
}
inline float SimpleSpline(float value)
{
    float valueSquared = value * value;

    // Nice little ease-in, ease-out spline-like curve
    return (3 * valueSquared - 2 * valueSquared * value);
}
// remaps a value in [startInterval, startInterval+rangeInterval] from linear to
// spline using SimpleSpline
inline float SimpleSplineRemapValClamped(float val, float A, float B, float C, float D)
{
    if (A == B)
        return val >= B ? D : C;
    float cVal = (val - A) / (B - A);
    cVal = std::clamp<float>(cVal, 0.0f, 1.0f);
    return C + (D - C) * SimpleSpline(cVal);
}
float ent_info_t::solve_velocity_len(std::shared_ptr<player_record_t> record)
{
    auto weapon = (weapon_t*)g.m_interfaces->entity_list()->get_client_entity_handle(record->m_ent->active_weapon());
    float max_speed = 260.0f;
    auto info = weapon ? g.m_interfaces->weapon_system()->get_weapon_data(weapon->item_definition_index()) : nullptr;
    if (info)
    {
        max_speed = fmaxf(0.001f, info->m_max_player_speed);
    }

    float comp_value = record->m_layers[ANIMATION_LAYER_ALIVELOOP].m_weight;
    if (comp_value == 0.f)
        return -1;
    float best = -1;
    float closest = -1;

    for (float i = 0.f; i < max_speed; i += 0.5f)
    {
        float val = SimpleSplineRemapValClamped(i / max_speed, 0.55f, 0.9f, 1.0f, 0.f);
        if (closest == -1 || fabsf(comp_value - val) < best)
        {
            best = fabsf(comp_value - val);
            closest = i;
        }
    }

    return closest;

    auto state = record->m_ent->get_anim_state();
    if (!state)
        return record->m_anim_velocity.length();
    float cycle_dif = record->m_layers[6].m_playback_rate;
    if (cycle_dif == 0.f)
    {
        if (record->m_layers[6].m_cycle >= 0.999f)
        {
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
    // while (fabsf(walk_to_run_max - walk_to_run_min) > 0.00001) {
    //	float test = ((walk_to_run_max - walk_to_run_min) / 2) +
    // walk_to_run_min;
    //
    //	float min_dif = fabsf(Lerp(walk_to_run_min, 1.f, 0.14999998f *
    //*(float*)(state + 284)) - flMoveCycleRate); 	float max_dif =
    // fabsf(Lerp(walk_to_run_max, 1.f, 0.14999998f * *(float*)(state + 284)) -
    // flMoveCycleRate);
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
    //		float dif = fabsf(Lerp(test, 1.f, 0.14999998f * *(float*)(state +
    // 284)) - flMoveCycleRate); 		float test_2 = ((walk_to_run_max - test) / 2) +
    // walk_to_run_min;
    //
    //		float dif_2 = fabsf(Lerp(test_2, 1.f, 0.14999998f * *(float*)(state
    //+ 284)) - flMoveCycleRate);
    //
    //		if (dif > dif_2) {
    //			walk_to_run = test_2;
    //			walk_to_run_min = test;
    //			continue;
    //		}
    //
    //		test_2 = ((test - walk_to_run_min) / 2) + test;
    //		float idk = flMoveCycleRate / Lerp(test_2, 1.f, 0.14999998f *
    //*(float*)(state + 284)); 		dif_2 = fabsf(Lerp(test_2, 1.f, 0.14999998f *
    //*(float*)(state + 284) ) - flMoveCycleRate);
    //
    //		if (dif > dif_2) {
    //			walk_to_run = test_2;
    //			walk_to_run_max = test;
    //			continue;
    //		}
    //		break;
    //	}
    //}
    // float best_2 = -1;
    // float test_2 = walk_to_run_min;
    // while (test_2 <= walk_to_run_max)
    //{
    //	float val = fabsf(Lerp(test_2, 1.f, 0.14999998f * *(float*)(state +
    // 284)) - flMoveCycleRate); 	if (best_2 == -1 || val < best_2) { 		walk_to_run =
    // test_2; 		best_2 = val;
    //	}
    //	test_2 += 0.000005f;
    //}

    // auto state = record->m_ent->get_anim_state();

    // float m_flAnimDuckAmount = std::clamp(Approach(std::clamp(record->m_duck +
    // *(float*)(state + 0xA8), 0.f, 1.f), *(float*)(state + 164),
    // g.ticks_to_time(record->m_lag) * 6.0f), 0.f, 1.f);

    // record->m_poses[(int)PoseParam_t::MOVE_BLEND_WALK] = (1.0f - walk_to_run) *
    // (1.0f - m_flAnimDuckAmount);
    // record->m_poses[(int)PoseParam_t::MOVE_BLEND_RUN] = (walk_to_run) * (1.0f -
    // m_flAnimDuckAmount); record->m_poses[(int)PoseParam_t::MOVE_BLEND_CROUCH] =
    // m_flAnimDuckAmount;

    auto hdr = record->m_ent->GetModelPtr();
    if (!hdr)
    {
        return (record->m_anim_velocity.length_2d_sqr() != 0.f) ? record->m_anim_velocity.length_2d() : 0.f;
    }
    // if (flMoveCycleRate == 0)
    //	return 0;
    // return 1 / flMoveCycleRate;

    int nWeaponMoveSeq = record->m_layers[6].m_sequence;

    float m_flWalkToRunTransition = *(float*)(record->m_ent->get_anim_state() + 284);

    bool set = (*(BYTE*)(record->m_ent->get_anim_state() + 308)) == 1;
    if (m_flWalkToRunTransition > 0 && m_flWalkToRunTransition < 1)
    {
        // currently transitioning between walk and run
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

    float m_flAnimDuckAmount =
        std::clamp(Approach(std::clamp(record->m_duck + *(float*)(record->m_ent->get_anim_state() + 168), 0.f, 1.f),
                            *(float*)(record->m_ent->get_anim_state() + 164), g.ticks_to_time(record->m_lag) * 6.0f),
                   0.f, 1.f);

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
        // currently transitioning between walk and run
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

    record->m_poses[(int)PoseParam_t::MOVE_BLEND_WALK] =
        (1.0f - m_flWalkToRunTransition_1) * (1.0f - m_flAnimDuckAmount);
    record->m_poses[(int)PoseParam_t::MOVE_BLEND_RUN] = (m_flWalkToRunTransition_1) * (1.0f - m_flAnimDuckAmount);

    vec3_t pVec2;
    pVec2 = record->m_ent->GetSequenceLinearMotion(hdr, nWeaponMoveSeq, record->m_poses);
    for (auto i = 0; i < 3; i++)
        if (isnan(fabsf(pVec1[i])))
            return (record->m_anim_velocity.length_2d_sqr() != 0.f) ? record->m_anim_velocity.length_2d() : 0.f;

    float pVec_len = pVec1.length();
    float pVec1_len = pVec2.length();

    float vel_x_y = 0.f;
    // float min_x_y = 0.1f - FLT_MIN;
    float max_x_y = 300.f;
    double test_1 = 0.1 - 0.01;
    while (test_1 <= max_x_y)
    {
        float w =
            flMoveCycleRate /
            (float)(1.0 - (float)((test_1 > 135.2) ? m_flWalkToRunTransition : m_flWalkToRunTransition_1) * 0.14999998);
        float val = test_velocity(record, test_1, (test_1 > 135.2) ? pVec1_len : pVec_len);
        if (val != -1)
        {
            const auto comp = fabsf(val - w);

            if (best == -1 || comp < best)
            {
                vel_x_y = test_1;
                best = comp;
            }
        }

        test_1 += 0.01;
    }

    // while (true) {
    //	test_1 = (max_x_y - min_x_y) / 2;
    //	test_1 += min_x_y;
    //	float test_2[2] = { (max_x_y - test_1) / 2, (test_1 - min_x_y) / 2 };
    //	test_2[0] += test_1;
    //	test_2[1] += min_x_y;
    //
    //	float val = test_velocity(record, test_1, (test_1 > 135.2) ? pVec1_len :
    // pVec_len); 	if( val == -1 ) 		return (record->m_anim_velocity.length_2d_sqr()
    //!= 0.f) ? record->m_anim_velocity.length_2d() : 0.f;
    //
    //	float comp = fabsf(val - w);
    //	if (comp <= 0.001f)
    //		return test_1;
    //
    //	val = test_velocity(record, test_2[0], (test_2[0] > 135.2) ? pVec1_len :
    // pVec_len); 	if (val == -1) 		return (record->m_anim_velocity.length_2d_sqr() !=
    // 0.f) ? record->m_anim_velocity.length_2d() : 0.f;
    //
    //	if (fabsf(val - w) < comp) {
    //		min_x_y = test_1;
    //	}
    //	else {
    //		val = test_velocity(record, test_2[1], (test_2[1] > 135.2) ?
    // pVec1_len : pVec_len); 		if (val == -1) 			return
    //(record->m_anim_velocity.length_2d_sqr() != 0.f) ?
    // record->m_anim_velocity.length_2d() : 0.f; 		if (fabsf(val - w) < comp)
    //		{
    //			max_x_y = test_1;
    //		}
    //		else {
    //			return test_1;
    //		}
    //	}
    //}

    if (best == -1)
        return (record->m_anim_velocity.length_2d_sqr() != 0.f) ? record->m_anim_velocity.length_2d() : 0.f;
    return vel_x_y;

    // while (max_x_y - min_x_y > 1) {
    //	float flSequenceGroundSpeed = 0.001f;
    //	float speed_est = ((max_x_y - min_x_y) / 2) + min_x_y;
    //	//float comp = fabsf((flMoveCycleRate / test(speed_est, pVec)) -
    // speed_est);
    //
    //	float min_dif = fabsf((flMoveCycleRate / test(max_x_y, pVec)) -
    // max_x_y); 	float max_dif = fabsf((flMoveCycleRate / test(min_x_y, pVec)) -
    // min_x_y);
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
    //		float dif = fabsf((flMoveCycleRate / test(speed_est, pVec)) -
    // speed_est); 		float new_speed = ((max_x_y - speed_est) / 2) + speed_est; 		float
    // dif_2 = fabsf((flMoveCycleRate / test(new_speed, pVec)) - new_speed);
    //
    //		if (dif > dif_2) {
    //			min_x_y = speed_est;
    //			vel_x_y = new_speed;
    //			continue;
    //		}
    //
    //		new_speed = ((speed_est - min_x_y) / 2) + min_x_y;
    //		dif_2 = fabsf((flMoveCycleRate / test(new_speed, pVec)) -
    // max_x_y);
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

void ent_info_t::UpdateAnimations(std::shared_ptr<player_record_t> record)
{
    auto* state = m_ent->get_anim_state();
    if (!state)
        return;

    // player respawned.
    if (m_ent->spawn_time() != m_spawn)
    {
        memset(&m_ik, 0, sizeof(CIKContext));
        m_ik.init();
        // reset animation state.
        state->ResetAnimationState();

        // note new spawn time.
        m_spawn = m_ent->spawn_time();

        for (auto& i : m_resolver_data.m_states)
        {
            i = *state;
        }
    }

    // backup stuff that we do not want to fuck with.
    animation_backup_t backup;

    backup.m_origin = m_ent->origin();
    backup.m_abs_origin = m_ent->abs_origin();
    backup.m_velocity = m_ent->velocity();
    backup.m_abs_velocity = m_ent->abs_vel();
    backup.m_flags = m_ent->flags();
    backup.m_eflags = m_ent->iEFlags();
    backup.m_duck = m_ent->duck_amount();
    backup.m_body = m_ent->lower_body_yaw();
    m_ent->GetAnimLayers(backup.m_layers);
    m_ent->GetPoseParameters(backup.m_poses);

    const auto bot = m_fake_player;

    // fix velocity.
    // https://github.com/VSES/SourceEngine2007/blob/master/se2007/game/client/c_baseplayer.cpp#L659
    if (m_records.size() >= 2)
    {
        // get pointer to previous record.
        auto const& previous = m_records[1];

        if (previous && !previous->m_dormant)
        {
            record->m_velocity = (record->m_origin - previous->m_origin) * (1.f / g.ticks_to_time(record->m_lag));
        }
    }
    // set this fucker, it will get overriden.
    record->m_anim_velocity = record->m_velocity;

    // fix various issues with the game eW91dHViZS5jb20vZHlsYW5ob29r
    // these issues can only occur when a player is choking data.
    // if ((record->m_flags & fl_onground) && !m_teamate) {
    // g.m_interfaces->mdlcache()->begin_lock();
    // vec3_t w;
    // if (record->m_anim_velocity.length_2d_sqr() != 0)
    //	w = record->m_anim_velocity.normalized();
    // else
    //	w = { 0.5,0.5, 0 };
    // float speed = solve_velocity_len(record);
    // if( speed >= 0.f )
    //	record->m_anim_velocity = vec3_t(w.x * speed, w.y * speed,
    // record->m_anim_velocity.z); g.m_interfaces->mdlcache()->end_lock();
    //}
    cmd_t cmd;
    if (record->m_lag > 1 && !bot)
    {
        auto speed = record->m_velocity.length();
        //if (speed > 0.1f && record->m_layers[6].m_cycle <= 0.01f && (record->m_flags & fl_onground))
        //    record->m_fake_walk = true;
        //
        //// detect players abusing micromovements or other trickery
        //// record->m_fake_walk = record->m_layers[6].m_playback_rate == 0.f;
        //if (record->m_fake_walk || record->m_layers[6].m_cycle == 0.f && (record->m_flags & fl_onground))
        //    record->m_anim_velocity = vec3_t(0, 0, 0);
        // we need atleast 2 updates/records
        // to fix these issues.
        if (m_records.size() >= 2 && m_records[1])
        {
            //if (speed < 20.f && record->m_layers[6].m_weight != 1.0f && record->m_layers[6].m_weight != 0.0f &&
            //    record->m_layers[6].m_weight != m_records[1]->m_layers[6].m_weight && (record->m_flags & fl_onground))
            //    record->m_ukn_vel = true;

            // auto weapon = ( weapon_t* )g.m_interfaces->entity_list(
            // )->get_client_entity_handle( record->m_ent->active_weapon( ) ); auto
            // max_speed = 260.f; if ( weapon ) { 	auto data =
            // g.m_interfaces->weapon_system( )->get_weapon_data(
            // weapon->item_definition_index( ) ); 	if ( data )
            // max_speed = fmaxf( data->m_max_player_speed, 0.001f );
            //}
            // float temp = ( max_speed * 0.52f );
            // if ( temp > 0.f ) {
            //	float walk_speed = 1.1f / temp;
            //	float max_dist = ( 1.f - ( 2.f * g.ticks_to_time( record->m_lag
            //) ) ) - walk_speed; 	if ( speed < 40.f
            //		&& record->m_layers[ 6 ].m_weight != 1.0f
            //		&& record->m_layers[ 6 ].m_weight != 0.0f
            //		&& fabsf( record->m_layers[ 6 ].m_weight - walk_speed )
            //> max_dist
            //		&& ( record->m_flags & fl_onground ) )
            //		record->m_ukn_vel = true;
            //}
            //if (record->m_ukn_vel)
            //    record->m_anim_velocity = vec3_t(0, 0, 0);
            // get pointer to previous record.

            if (auto const& previous = m_records[1]; previous && !previous->m_dormant)
            {
                // strip the on ground flag.
                m_ent->flags() &= ~fl_onground;

                // float flLandTime = 0.0f;
                // bool bLandedOnServer = false;
                // if (record->m_layers[4].m_cycle != previous->m_layers[4].m_cycle &&
                //     record->m_layers[5].m_cycle != previous->m_layers[5].m_cycle && record->m_layers[5].m_cycle != 0)
                //{
                //     m_ent->flags() |= fl_onground;
                // }
                // else if (record->m_layers[4].m_cycle != previous->m_layers[4].m_cycle &&
                //          record->m_layers[5].m_cycle == previous->m_layers[5].m_cycle)
                //{
                //     m_ent->flags() &= ~fl_onground;
                // }

                if (record->m_lag > 2)
                {
                    float flLandTime = 0.0f;
                    bool bJumped = false;
                    bool bLandedOnServer = false;
                    if (record->m_layers[4].m_cycle < 0.5f &&
                        (!(record->m_flags & fl_onground) || !(previous->m_flags & fl_onground)))
                    {
                        // note - VIO (violations btw);
                        // well i guess when llama wrote v3, he was drunk or sum cuz this is incorrect. -> cuz he
                        // changed this in v4. and alpha didn't realize this but i did, so its fine. improper way to do
                        // this -> flLandTime = record->m_flSimulationTime - float( record->m_serverAnimOverlays[ 4
                        // ].m_flPlaybackRate * record->m_serverAnimOverlays[ 4 ].m_flCycle ); we need to divide instead
                        // of multiplication.
                        if (record->m_layers[4].m_cycle > 0.f)
                            flLandTime =
                                record->m_anim_time - record->m_layers[4].m_cycle / record->m_layers[4].m_playback_rate;
                        bLandedOnServer = flLandTime >= previous->m_anim_time;
                    }

                    bool bOnGround = record->m_flags & fl_onground;
                    // jump_fall fix
                    if (bLandedOnServer && !bJumped)
                    {
                        if (flLandTime <= record->m_anim_time)
                        {
                            bJumped = true;
                            bOnGround = true;
                        }
                        else
                        {
                            bOnGround = previous->m_flags & fl_onground;
                        }
                    }
                    if (bOnGround)
                        record->m_flags |= fl_onground;
                }

                // fix crouching players.
                // the duck amount we receive when people choke is of the last
                // simulation. if a player chokes packets the issue here is that we will
                // always receive the last duckamount. but we need the one that was
                // animated. therefore we need to compute what the duckamount was at
                // animtime.

                // delta in duckamt and delta in time..
                const auto duck = record->m_duck - previous->m_duck;
                float time = record->m_sim_time - previous->m_sim_time;

                float change = (duck / time) * g.m_interfaces->globals()->m_interval_per_tick;
                if (change > 0)
                    cmd.m_buttons |= IN_DUCK;
                m_ent->duck_amount() = previous->m_duck + change;
                
                // fix the velocity till the moment of animation.
                vec3_t velo = record->m_velocity - previous->m_velocity;

                // accel per tick.
                vec3_t accel = (velo / time) * g.m_interfaces->globals()->m_interval_per_tick;

                // set the anim velocity to the previous velocity.
                // and predict one tick ahead.
                record->m_anim_velocity = previous->m_velocity + accel;

                vec3_t temp_vel = previous->m_velocity;
                block_bot_t::friction(1.f, &temp_vel);

                velo = record->m_velocity - temp_vel;
                accel = (velo / time) * g.m_interfaces->globals()->m_interval_per_tick;

                cmd.m_forwardmove = accel.length_2d();
                if (accel.length_2d_sqr() > 0.1f)
                    cmd.m_viewangles = vec3_t().look(accel);
            }
        }

        auto weapon =
            static_cast<weapon_t*>(g.m_interfaces->entity_list()->get_client_entity_handle(m_ent->active_weapon()));

        float weapon_speed = 260.f;
        if (weapon)
        {
            weapon_speed = max(0.001f, weapon->get_max_speed());
        }

        float flInterval = record->m_anim_time - state->m_flUnknownVelocityLean;
        if (record->m_flags & fl_onground)
        {
            if (record->m_layers[6].m_weight <= 0.f)
            {
                record->m_anim_velocity = vec3_t(0, 0, 0);
            }
            else if (record->m_layers[6].m_playback_rate > 0.f)
            {
                auto origin_delta_norm = record->m_anim_velocity.normalized();
                origin_delta_norm.z = 0.f;

                auto delta_norm_len = origin_delta_norm.length_2d();
                float MoveWeightAirSmooth =
                    record->m_layers[6].m_weight / max(1.f - record->m_layers[5].m_weight, 0.55f);
                float flTargetMoveWeight_to_speed =
                    Lerp(m_ent->duck_amount(), weapon_speed * 0.52f, weapon_speed * 0.34f) * MoveWeightAirSmooth;

                float speed_as_portion_of_run_top_speed = 0.35f * (1.f - record->m_layers[11].m_weight);

                if (record->m_layers[11].m_weight > 0.f && record->m_layers[11].m_weight < 1.f) {
                    record->m_anim_velocity = origin_delta_norm * (weapon_speed * (speed_as_portion_of_run_top_speed));
                }
                else if (flTargetMoveWeight_to_speed < 0.95f || flTargetMoveWeight_to_speed > delta_norm_len)
                {
                    record->m_anim_velocity = origin_delta_norm * flTargetMoveWeight_to_speed;
                }
                else {
                    auto adjusted_speed = weapon_speed;
                    if (record->m_flags & fl_ducking)
                        adjusted_speed *= 0.34f;
                    if (delta_norm_len > adjusted_speed)
                        record->m_anim_velocity = origin_delta_norm * adjusted_speed;
                }
            }
        }
    }
    else if (record->m_lag > 0 && m_records.size() >= 2 && m_records[1])
    {
        auto const& previous = m_records[1];
        float time = record->m_sim_time - previous->m_sim_time;
        vec3_t temp_vel = previous->m_velocity;
        block_bot_t::friction(1.f, &temp_vel);
        vec3_t velo = record->m_velocity - temp_vel;

        // accel per tick.
        vec3_t accel = (velo / time) * g.m_interfaces->globals()->m_interval_per_tick;
        cmd.m_forwardmove = accel.length_2d();
        if (accel.length_2d_sqr() > 0.1f)
            cmd.m_viewangles = vec3_t().look(accel);
    }

    //if (fabsf(cmd.m_forwardmove) < 0.001f && record->m_anim_velocity.length_2d_sqr() > 0.1f)
    //{
    //    cmd.m_forwardmove = record->m_anim_velocity.length_2d();
    //    cmd.m_viewangles = vec3_t().look(record->m_anim_velocity);
    //}
    // set stuff before animating.
    m_ent->origin() = record->m_origin;
    m_ent->velocity() = m_ent->abs_vel() = record->m_anim_velocity;
    m_ent->lower_body_yaw() = record->m_body;

   // {
   //     player_move_data data;
   //     float old_cur_time = g.m_interfaces->globals()->m_curtime;
   //     float old_frame_time = g.m_interfaces->globals()->m_frametime;
   // 
   //     g.m_interfaces->globals()->m_curtime = record->m_pred_time += g.m_interfaces->globals()->m_interval_per_tick;
   //     g.m_interfaces->globals()->m_frametime = g.m_interfaces->globals()->m_interval_per_tick;
   // 
   //     g.m_interfaces->prediction()->setup_move(m_ent, &cmd, g.m_interfaces->move_helper(), &data);
   //     data.max_speed = 260.f;
   //     g.m_interfaces->game_movement()->process_movement(m_ent, &data);
   //     if (record->m_lag <= 0)
   //     {
   //         if (auto nci = g.m_interfaces->engine()->get_net_channel_info())
   //         {
   //             float add_time = m_ent->sim_time() - g.ticks_to_time(g.m_local->tick_base());
   //             while (add_time < nci->GetLatency(0) + nci->GetAvgLatency(1))
   //             {
   //                 record->m_pred_time += g.m_interfaces->globals()->m_interval_per_tick;
   //                 g.m_interfaces->globals()->m_curtime = record->m_pred_time;
   //                 g.m_interfaces->game_movement()->process_movement(m_ent, &data);
   // 
   //                 add_time += g.m_interfaces->globals()->m_interval_per_tick;
   //             }
   //         }
   //     }
   //     record->m_pred_origin = data.abs_origin;
   //     g.m_interfaces->prediction()->finish_move(m_ent, &cmd, &data);
   // 
   //     g.m_interfaces->globals()->m_curtime = old_cur_time;
   //     g.m_interfaces->globals()->m_frametime = old_frame_time;
   // }

    // write potentially resolved angles.

    // for ( auto i = 0; i < info->m_lag; i++ ) {
    //
    //	//console::log(("cycle " + std::to_string(record->m_layers[6].m_cycle) +
    //"\n").c_str());
    //	//console::log(("m_playback_rate " +
    // std::to_string(info->m_layers[6].m_playback_rate) + "\n").c_str());
    //	//console::log(("sequence " +
    // std::to_string(info->m_layers[6].m_sequence) + "\n").c_str());
    //	//console::log(("m_weight " + std::to_string(info->m_layers[6].m_weight)
    //+ "\n").c_str());
    //	//console::log(("yaw " + std::to_string(record->m_eye_angles.y) +
    //"\n").c_str());
    //
    //	// 'm_animating' returns true if being called from SetupVelocity, passes
    // raw velocity to animstate.
    //
    // auto backup_overlay_count = m_ent->anim_overlay_vec().Count();
    // m_ent->anim_overlay_vec().m_Size = (0);

    m_ent->eye_angles() = record->m_eye_angles;
    m_ent->client_side_anim() = true;
    {
        // backup curtime.
        const auto curtime = g.m_interfaces->globals()->m_curtime;
        const auto frametime = g.m_interfaces->globals()->m_frametime;

        g.m_interfaces->globals()->m_curtime = record->m_anim_time;
        g.m_interfaces->globals()->m_frametime = g.m_interfaces->globals()->m_interval_per_tick;

        if (!m_teamate && !bot)
        {
            g_resolver.resolve(*this, record);
            // state->feetYawRate = 0.f;

            resolver_data::mode_data* mode_data = nullptr;
            if ((!record->m_body_reliable && record->m_mode == resolver::RESOLVE_BODY) ||
                record->m_mode == resolver::RESOLVE_WALK)
            {
                mode_data = &m_resolver_data.m_mode_data[resolver_data::LBY_MOVING];
                float val = 30.f;
                if (record->m_mode == resolver::RESOLVE_WALK)
                    val += 20.f;

                record->m_resolver_data.m_dir_data.emplace_back(record->m_body);
                record->m_resolver_data.m_dir_data.emplace_back(record->m_body + val);
                record->m_resolver_data.m_dir_data.emplace_back(record->m_body - val);
            }
            else if (record->m_mode == resolver::RESOLVE_STAND1)
                mode_data = &m_resolver_data.m_mode_data[resolver_data::STAND1];
            else if (record->m_mode == resolver::RESOLVE_STAND2)
                mode_data = &m_resolver_data.m_mode_data[resolver_data::STAND2];

            anim_state backup_anim_state{};
            backup_anim_state = *state;
            anim_state index_state{};
            bool index_found = false;
            if (mode_data && record->m_resolver_data.m_dir_data.size() == mode_data->m_dir_data.size())
            {
                for (uint32_t i = 0; i < mode_data->m_dir_data.size(); i++)
                {
                    auto& record_dir_data = record->m_resolver_data.m_dir_data[i];
                    *state = m_resolver_data.m_states[i];

                    m_ent->eye_angles().y = math::normalize_angle(record_dir_data.angles, 180);

                    if (state->m_frame >= g.m_interfaces->globals()->m_framecount)
                        state->m_frame = g.m_interfaces->globals()->m_framecount - 1;
                    //if (m_records.size() >= 2 && m_records[1])
                    //{
                    //    state->feetYawRate = m_records[1]->feet_yaw_rate;
                    //    state->feetCycle = m_records[1]->feet_cycle;
                    //
                    //    // m_ent->SetAnimLayers(m_records[1]->m_layers);
                    //}

                    float lby_delta = record->m_body - m_ent->eye_angles().y;
                    lby_delta = std::remainderf(lby_delta, 360.f);
                    lby_delta = std::clamp(lby_delta, -60.f, 60.f);

                    float feet_yaw = std::remainderf(m_ent->eye_angles().y + lby_delta, 360.f);
                    if (feet_yaw < 0.f)
                        feet_yaw += 360.f;

                    state->m_goal_feet_yaw = state->m_cur_feet_yaw = feet_yaw;
                    // else
                    //{
                    //     state->feetYawRate = record->feet_yaw_rate;
                    //     state->feetCycle = record->feet_cycle;
                    //
                    //     m_ent->SetAnimLayers(record->m_layers);
                    // }
                    m_ent->iEFlags() &= ~(0x1000 | (1 << 11) | (1 << 12));
                    m_ent->update_client_side_animation();

                    m_ent->GetPoseParameters(record_dir_data.poses);
                    record_dir_data.m_abs_angles = m_ent->abs_angles();
                    
                    m_resolver_data.m_states[i] = *state;

                    if (mode_data->m_index == i)
                    {
                        record->m_abs_angles = m_ent->abs_angles();
                        index_found = true;
                        index_state = *state;
                        m_ent->GetPoseParameters(record->m_poses);
                    }

                    *state = backup_anim_state;
                    m_ent->SetPoseParameters(backup.m_poses);
                    m_ent->SetAnimLayers(backup.m_layers);
                }
            }
            if (index_found)
            {
                *state = index_state;
            }
            else
            {
                m_ent->eye_angles() = record->m_eye_angles;
                if (state->m_frame >= g.m_interfaces->globals()->m_framecount)
                    state->m_frame = g.m_interfaces->globals()->m_framecount - 1;
                //if (m_records.size() >= 2 && m_records[1])
                //{
                //    state->feetYawRate = m_records[1]->feet_yaw_rate;
                //    state->feetCycle = m_records[1]->feet_cycle;
                //
                //    // m_ent->SetAnimLayers(m_records[1]->m_layers);
                //}
                // else
                //{
                //     state->feetYawRate = record->feet_yaw_rate;
                //     state->feetCycle = record->feet_cycle;
                //
                //     m_ent->SetAnimLayers(record->m_layers);
                // }

                // m_ent->iEFlags() &= ~0x1000;

                float lby_delta = record->m_body - m_ent->eye_angles().y;
                lby_delta = std::remainderf(lby_delta, 360.f);
                lby_delta = std::clamp(lby_delta, -60.f, 60.f);

                float feet_yaw = std::remainderf(m_ent->eye_angles().y + lby_delta, 360.f);
                if (feet_yaw < 0.f)
                    feet_yaw += 360.f;

                state->m_goal_feet_yaw = state->m_cur_feet_yaw = feet_yaw;

                m_ent->iEFlags() &= ~(0x1000 | (1 << 11) | (1 << 12));
                m_ent->update_client_side_animation();
                record->m_abs_angles = m_ent->abs_angles();
                // store updated/animated poses and rotation in lagrecord.
                m_ent->GetPoseParameters(record->m_poses);
                m_ent->SetAnimLayers(backup.m_layers);
            }
        }
        else
        {
            m_ent->eye_angles() = record->m_eye_angles;
            if (state->m_frame >= g.m_interfaces->globals()->m_framecount)
                state->m_frame = g.m_interfaces->globals()->m_framecount - 1;

            //if (m_records.size() >= 2 && m_records[1])
            //{
            //    state->feetYawRate = m_records[1]->feet_yaw_rate;
            //    state->feetCycle = m_records[1]->feet_cycle;
            //
            //    // m_ent->SetAnimLayers(m_records[1]->m_layers);
            //}
            // else
            //{
            //     state->feetYawRate = record->feet_yaw_rate;
            //     state->feetCycle = record->feet_cycle;
            //
            //     m_ent->SetAnimLayers(record->m_layers);
            // }

            // m_ent->iEFlags() &= ~0x1000;

            float lby_delta = record->m_body - m_ent->eye_angles().y;
            lby_delta = std::remainderf(lby_delta, 360.f);
            lby_delta = std::clamp(lby_delta, -60.f, 60.f);

            float feet_yaw = std::remainderf(m_ent->eye_angles().y + lby_delta, 360.f);
            if (feet_yaw < 0.f)
                feet_yaw += 360.f;

            state->m_goal_feet_yaw = state->m_cur_feet_yaw = feet_yaw;

            m_ent->iEFlags() &= ~(0x1000 | (1 << 11) | (1 << 12));
            m_ent->update_client_side_animation();
            record->m_abs_angles = m_ent->abs_angles();
            // store updated/animated poses and rotation in lagrecord.
            m_ent->GetPoseParameters(record->m_poses);
            m_ent->SetAnimLayers(backup.m_layers);
        }

        g.m_interfaces->globals()->m_curtime = curtime;
        g.m_interfaces->globals()->m_frametime = frametime;
    }
    // m_ent->anim_overlay_vec().m_Size = (backup_overlay_count);
    m_ent->client_side_anim() = false;
    //}

    // restore backup data.
    m_ent->origin() = backup.m_origin;
    m_ent->velocity() = backup.m_velocity;
    m_ent->abs_vel() = backup.m_abs_velocity;
    m_ent->flags() = backup.m_flags;
    m_ent->iEFlags() = backup.m_eflags;
    m_ent->duck_amount() = backup.m_duck;
    m_ent->lower_body_yaw() = backup.m_body;
    m_ent->set_abs_origin(backup.m_abs_origin);
    m_ent->SetAnimLayers(backup.m_layers);
    m_ent->SetPoseParameters(backup.m_poses);
}

float player_record_t::set_lerped_time(player_record_t* prev, player_record_t* next)
{
    int temp_tick = g.time_to_ticks(prev->m_sim_time + g.m_lerp);
    while (g.ticks_to_time(temp_tick) - g.m_lerp > prev->m_sim_time)
        temp_tick--;
    m_backtrack_tick = temp_tick;
    float target_time = g.ticks_to_time(temp_tick) - g.m_lerp;
    float delta = (target_time - m_sim_time);
    if (delta > 0.f)
    {
        if (prev->m_sim_time <= m_sim_time)
            return 0.f;
        if (target_time >= prev->m_sim_time)
            return 0.f;

        // calc fraction between both records
        float frac = (target_time - m_sim_time) / (prev->m_sim_time - m_sim_time);

        if (frac <= 0.f || frac >= 1.f)
            return 0.f;

        m_eye_angles = Lerp(frac, m_abs_angles, prev->m_abs_angles);
        m_pred_origin = Lerp(frac, m_origin, prev->m_origin);
        m_mins = Lerp(frac, m_mins, prev->m_mins);
        m_maxs = Lerp(frac, m_maxs, prev->m_maxs);
    }
    else if (delta < 0)
    {
        if (prev->m_sim_time >= m_sim_time)
            return 0.f;
        if (target_time <= prev->m_sim_time)
            return 0.f;
        // calc fraction between both records
        const float frac = (m_sim_time - target_time) / (m_sim_time - next->m_sim_time);

        if (frac <= 0.f || frac >= 1.f)
            return 0.f;

        m_eye_angles = Lerp(frac, next->m_abs_angles, m_abs_angles);
        m_pred_origin = Lerp(frac, next->m_origin, m_origin);
        m_mins = Lerp(frac, next->m_mins, m_mins);
        m_maxs = Lerp(frac, next->m_maxs, m_maxs);
    }
    return delta;
}
void ent_info_t::update(player_t* ent)
{
    m_ent = ent;
    m_teamate = ent->on_team(g.m_local);

    m_dormant = ent->dormant();

    if (m_dormant)
    {
        m_walk_record.m_sim_time = 0.0f;
        auto insert = true;

        // we have any records already?
        if (!m_records.empty())
        {
            // we already have a dormancy separator.
            if (auto const front = m_records.front(); front && front->m_dormant)
                insert = false;
        }

        if (insert)
        {
            // add new record.
            const auto rec = std::make_shared<player_record_t>(this, 0);
            rec->m_dormant = true;
            m_records.push_front(rec);
        }
    }

    float last_sim;
    if (const auto update = m_records.empty() || !m_records[0]; !update)
    {
        last_sim = m_records.front()->m_sim_time;
        if (fabsf(last_sim - ent->sim_time()) < g.m_interfaces->globals()->m_interval_per_tick)
            return;
    }
    else
    {
        last_sim = ent->sim_time();
    }

    // g_thread_handler.queue_mutex.lock( );
    m_records.push_front(std::make_shared<player_record_t>(this, last_sim));

    while (m_records.size() > 256)
        m_records.pop_back();

    // g_thread_handler.queue_mutex.unlock( );

    const auto& current = m_records.front();

    current->m_setup = false;

    current->m_dormant = false;
    UpdateAnimations(current);

    if (m_teamate)
        return;

    if (auto* weapon =
            static_cast<weapon_t*>(g.m_interfaces->entity_list()->get_client_entity_handle(m_ent->active_weapon())))
    {
        if (const auto weapon_world_model = static_cast<weapon_world_model_t*>(
                g.m_interfaces->entity_list()->get_client_entity_handle(weapon->weapon_model())))
        {
            if (const auto weapon_studio_hdr = weapon_world_model->GetModelPtr())
            {
                for (auto& layer : current->m_layers)
                {
                    if (layer.m_sequence <= 1 || layer.m_cycle <= 0.f)
                    {
                        m_ent->UpdateDispatchLayer(&layer, weapon_studio_hdr, layer.m_sequence);
                    }
                }
            }
        }
    }

    if (current->m_mode == resolver::RESOLVE_BODY || current->m_mode == resolver::RESOLVE_WALK)
    {
        current->m_fake_bones_setup = build_fake_bones(current);
    }
    else
    {
        m_ent->iEFlags() &= ~(1 << 11);
        current->m_setup = g_bones.setup(m_ent, current->m_bones, current, &m_ik);
    }
}

class plThreadObject
{
public:
    bool ret_state = false;
    bool read = false;
    ent_info_t* info;
    player_t* player;
    bool finished = false;
    void run();
    plThreadObject(ent_info_t* info_, player_t* player_)
    {
        info = info_;
        player = player_;
    }
};

void plThreadObject::run()
{
    info->update(player);
    finished = true;
}

void update_player()
{
}

void player_manager_t::update()
{
    if (!g.m_local)
        return;

    g_resolver.update_shots();
    m_animating = true;
    std::vector<std::shared_ptr<plThreadObject>> objects = {};
    // g_thread_handler.start();
    for (auto i{1}; i <= g.m_interfaces->globals()->m_max_clients; ++i)
    {
        auto* const player = static_cast<player_t*>(g.m_interfaces->entity_list()->get_client_entity(i));
        auto* data = &m_ents[i - 1];
        data->m_valid = player && player != g.m_local && player->is_player() && player->alive();
        if (!data->m_valid)
        {
            data->m_records.clear();
            data->m_ent = nullptr;
            continue;
        }

        if (data->m_ent != player)
        {
            data->m_resolver_data.init();
            data->m_records.clear();
            engine_player_info_t info{};
            g.m_interfaces->engine()->get_player_info(player->index(), &info);
            data->m_fake_player = info.fakeplayer;
        }
        if (data->m_spawn != player->spawn_time())
        {
            data->m_resolver_data.init();
            data->m_records.clear();
        }
        {
            data->update(player);
            // objects.push_back(std::make_shared<plThreadObject>(&m_ents[i - 1], player));
            // std::weak_ptr<plThreadObject> ptr = objects.back();
            // g_thread_handler.QueueJob([ptr] { ptr.lock()->run(); });
        }

        // data->update( player );
    }
    // while (g_thread_handler.busy())
    //     continue;
    //
    // g_thread_handler.stop();
    // bool all_finished = true;
    // while (!all_finished)
    //{
    //     all_finished = true;
    //     for (auto& i : objects)
    //         if (!i->finished)
    //             all_finished = false;
    // }
    //  g_thread_handler.start( );
    //  g_thread_handler.mutex_condition.notify_all( );
    //
    //  while ( g_thread_handler.busy( ) );
    //
    //{
    //	g_thread_handler.queue_mutex2.lock( );
    //	g_thread_handler.objects.clear( );
    //	g_thread_handler.queue_mutex2.unlock( );
    // }
    //
    m_animating = false;
}

bool ent_info_t::build_fake_bones(std::shared_ptr<player_record_t> current)
{
    resolver_data::mode_data* mode_data = nullptr;

    if (current->m_mode == resolver::RESOLVE_STAND1)
        mode_data = &m_resolver_data.m_mode_data[resolver_data::STAND1];
    else if (current->m_mode == resolver::RESOLVE_STAND2)
        mode_data = &m_resolver_data.m_mode_data[resolver_data::STAND2];
    else if (((!current->m_body_reliable && current->m_mode == resolver::RESOLVE_BODY) ||
              current->m_mode == resolver::RESOLVE_WALK))
        mode_data = &m_resolver_data.m_mode_data[resolver_data::LBY_MOVING];

    if (!mode_data)
        return false;

    const auto backup_ang = current->m_abs_angles;
    float backup_poses[24];
    memcpy(backup_poses, current->m_poses, sizeof(float) * 24);

    bool all_setup = true;

    int bone_ix = 0;
    for (uint32_t i = 0; i < current->m_resolver_data.m_dir_data.size(); i++)
    {
        auto& record_dir_data = current->m_resolver_data.m_dir_data[i];
        // memcpy( current->m_layers, current->m_fake_layers[ i ], sizeof(
        // animation_layer_t ) * 13 ); memcpy( current->m_layers, backup_layers,
        // sizeof( animation_layer_t ) * 13 );
        current->m_abs_angles = record_dir_data.m_abs_angles;
        m_ent->iEFlags() &= ~(1 << 11);
        if (i == mode_data->m_index && !current->m_setup)
        {
            {
                current->m_setup = g_bones.setup(current->m_ent, current->m_bones, current, &m_ik);
            }
        }
        else
        {
            memcpy(current->m_poses, record_dir_data.poses, sizeof(float) * 24);
            {
                current->m_fake_bones.push_back(
                    static_cast<bone_array_t*>(g.m_interfaces->mem_alloc()->alloc(sizeof(bone_array_t) * 128)));
                if (!g_bones.setup(current->m_ent, current->m_fake_bones.back(), current, &m_ik))
                    all_setup = false;
            }
            bone_ix++;
        }
        memcpy(current->m_poses, backup_poses, sizeof(float) * 24);
    }

    current->m_abs_angles = backup_ang;
    return all_setup;
}

void backup_record_t::store(player_t* player)
{
    // get bone cache ptr.
    bone_cache_t* cache = &player->bone_cache();

    // store bone data.
    m_last_bone_setup = player->last_bone_setup_time();
    m_bones = cache->m_pCachedBones;
    m_bone_count = cache->m_CachedBoneCount;
    m_origin = player->origin();
    m_mins = player->mins();
    m_maxs = player->maxs();
    m_abs_origin = player->abs_origin();
    m_abs_ang = player->abs_angles();
}

void backup_record_t::restore(player_t* player) const
{
    // get bone cache ptr.
    bone_cache_t* cache = &player->bone_cache();

    cache->m_pCachedBones = m_bones;
    cache->m_CachedBoneCount = m_bone_count;
    player->last_bone_setup_time() = m_last_bone_setup;
    player->origin() = m_origin;
    player->mins() = m_mins;
    player->maxs() = m_maxs;
    player->set_abs_angles(m_abs_ang);
    player->set_abs_origin(m_abs_origin);
}
