#include "aimbot.h"

#include "threading.h"
#include "shared_mutex.h"
#include <execution>
#include "movement.h"
#include "bones.h"
#include "hooks.h"
#include "penetration.h"
#include "resolver.h"

// class hcThreadObject {
// public:
//	bool ret_state = false;
//	bool read = false;
//	weapon_t* weapon;
//	ang_t view;
//	int id = 0;
//	bool finished = false;
//
//	std::vector<RayTracer::Hitbox> hit_boxes = {};
//	void run( );
//	hcThreadObject( weapon_t* weapon_, ang_t view_, int id_, std::vector<RayTracer::Hitbox> hitboxes_ ) {
//		weapon = weapon_;
//		view = view_;
//		id = id_;
//		hit_boxes = hitboxes_;
//	}
// };
//
// void hcThreadObject::run ( ) {
//	size_t     total_hits{ };
//	const auto needed_hits{ static_cast< size_t >( ( settings::rage::selection::hitchance * 255 / 100.f ) ) };
//
//	const auto inaccuracy = g.m_weapon->inaccuracy( );
//	const auto spread = g.m_weapon->get_spread( );
//	const auto start{ g.m_shoot_pos };
//
//	static auto surface_predicate = [ ]( const RayTracer::Hitbox& a, const RayTracer::Hitbox& b ) {
//		const float area_1 = ( M_PI * powf( a.m_radius, 2 ) * a.m_len ) + ( 4.f / 3.f * M_PI * a.m_radius );
//		const float area_2 = ( M_PI * powf( b.m_radius, 2 ) * b.m_len ) + ( 4.f / 3.f * M_PI * b.m_radius );
//
//		return area_1 < area_2;
//	};
//	std::sort( hit_boxes.begin( ), hit_boxes.end( ), surface_predicate );
//
//
//	vec3_t forward{}, right{}, up{};
//
//	view.vectors( &forward, &right, &up );
//
//	vec3_t origin;
//	ang_t angles;
//	for ( auto i = 0; i < 255; i++ ) {
//		const auto weapon_spread = math::calculate_spread( g.m_weapon, i, inaccuracy, spread, false );
//
//		auto dir = ( forward + ( right * weapon_spread.x ) + ( up * weapon_spread.y ) );
//		dir /= dir.length( );
//		const auto end = start + ( dir * g.m_weapon_info->m_range );
//
//		math::custom_ray_t ray( g.m_shoot_pos, end );
//
//		RayTracer::Ray ray_1( ray.m_start, ray.m_end );
//		for ( auto& box : hit_boxes ) {
//			RayTracer::Trace trace;
//			float m1, m2;
//			auto dist = math::distSegmentToSegmentSqr( ray.m_start, ray.m_end, box.m_mins, box.m_maxs, m1, m2 );
//			//RayTracer::TraceHitbox( ray_1, box, trace, RayTracer::Flags_NONE );
//			if ( dist <= box.m_radius * box.m_radius ) {
//				total_hits++;
//				finished = true;
//				break;
//			}
//			//RayTracer::TraceHitbox( ray_1, box, trace, RayTracer::Flags_NONE );
//			//if ( trace.m_hit ) {
//			//	total_hits++;
//			//	break;
//			//}
//		}
//		//for ( auto &box : hit_boxes ) {
//		//	trace_t trace;
//		//	if ( !box.m_box ) {
//		//		if ( box.m_radius >= math::dist_Segment_to_Segment( ray, box.m_min, box.m_max ) ) {
//		//			total_hits++;
//		//			break;
//		//		}
//		//	}
//		//	else {
//		//
//		//		matrix_t rot_matrix;
//		//		math::AngleMatrix( box.m_rot, &rot_matrix );
//		//
//		//		// apply the rotation to the entity input space (local).
//		//		matrix_t matrix;
//		//		math::ConcatTransforms( record->m_bones[ box.m_bone ], rot_matrix, &matrix );
//		//
//		//		if ( math::IntersectRayWithOBB( ray.m_start, ray.m_ray_dir, matrix, box.m_min, box.m_max, 0.0f, &trace ) )
//{
//		//			total_hits++;
//		//			break;
//		//		}
//		//	}
//		//}
//		if ( total_hits >= needed_hits ) {
//			ret_state = true;
//			finished = true;
//			return;
//		}
//		if ( ( ( 255 - i ) + total_hits ) < needed_hits ) {
//			ret_state = false;
//			finished = true;
//			return;
//		}
//	}
//	ret_state = false;
//	finished = true;
// }

bool aimbot_t::valid(ent_info_t* ent)
{
    return ent->m_ent && ent->m_valid && !ent->m_teamate && !ent->m_dormant && !ent->m_ent->gun_game_immunity();
}
void aimbot_t::get_targets()
{
    m_targets.clear();
    for (auto& ent : g_player_manager.m_ents)
    {
        ent.m_damage *= 1 * valid(&ent); // reset damage for every target
        if (valid(&ent))
            m_targets.push_back(&ent);
    }
}

enum aim_conditions
{
    LETHAL_BAIM = 0,
    IN_AIR_BAIM,
    FULL_SCAN
};

struct andrew_is_black {
    int color = 0;
};
struct andrew_is_gay {
    int how_much;
};

void aimbot_t::get_points(ent_info_t* info, studio_hdr_t* studio_model)
{
    info->m_hitboxes.clear();
    vec3_t center;
    int mode;
    float r;
    for (auto hitbox_num = hitboxes::hitbox_max - 1; hitbox_num >= 0; hitbox_num--)
    {
        auto* hitbox = studio_model->hitbox_set(info->m_ent->hitbox_set())->hitbox(hitbox_num);
        if (!hitbox)
            return;
        auto n = 5; // TODO: Add option on menu to adjust points per hitbox
        auto scale = settings::rage::hitbox::point_scale;
        int select = -1;
        switch (hitbox_num)
        {
            case hitbox_head:
                select = 0;
                break;

            case hitbox_chest:
            case hitbox_thorax:
            case hitbox_upper_chest:
                select = 1;
                break;

            case hitbox_pelvis:
            case hitbox_body:
                select = 2;
                scale = settings::rage::hitbox::body_scale;
                break;
            case hitbox_l_upper_arm:
            case hitbox_r_upper_arm:
                select = 3;
                break;
            case hitbox_l_calf:
            case hitbox_r_calf:
            case hitbox_l_thigh:
            case hitbox_r_thigh:
                select = 4;
                break;

            default:
                continue;
        }

        if (!(settings::rage::hitbox::hitboxes & (1 << select)))
            continue;

        if (settings::rage::hitbox::baim_conditions & (1 << IN_AIR_BAIM) && select != 2 &&
            info->m_selected_record->m_mode == resolver::RESOLVE_AIR) // force baim in air
            continue;

        hitbox_helper_t point_list{hitbox_num};

        center = (hitbox->mins + hitbox->maxs) / 2.f;

        mode = settings::rage::hitbox::baim_conditions;
        r = hitbox->radius * (scale / 100.f);

        matrix_t bone_transform;
        memcpy(&bone_transform, &info->m_selected_record->m_bones[hitbox->bone], sizeof(matrix_t));
        if (hitbox->angle != ang_t())
        {
            matrix_t temp;

            math::AngleMatrix(hitbox->angle, &temp);
            math::ConcatTransforms(bone_transform, temp, &bone_transform);
        }

        math::VectorTransform(center, bone_transform, center);

        ang_t view = g.m_shoot_pos.look(center);

        center = (hitbox->mins + hitbox->maxs) / 2.f;

        vec3_t right;
        view.vectors(nullptr, &right, nullptr);
        vec3_t point;
        switch (hitbox_num)
        {
            case hitbox_head:

                // top/back 45 deg.
                // this is the best spot to shoot at.
                math::VectorTransform(hitbox->maxs, bone_transform, point);
                point.z += r;
                point_list.m_points.emplace_back(point, 0, hitbox_num);

                math::VectorTransform(hitbox->maxs, bone_transform, point);
                point_list.m_points.emplace_back(point, 0, hitbox_num);

                math::VectorTransform(hitbox->maxs, bone_transform, point);
                point += right * r;
                point_list.m_points.emplace_back(point, 0, hitbox_num);

                math::VectorTransform(hitbox->maxs, bone_transform, point);
                point += right * -r;
                point_list.m_points.emplace_back(point, 0, hitbox_num);

                math::VectorTransform(hitbox->mins, bone_transform, point);
                point += right * r * 0.5f;
                point_list.m_points.emplace_back(point, 1 << FULL_SCAN, hitbox_num);

                math::VectorTransform(hitbox->mins, bone_transform, point);
                point += right * -r * 0.5f;
                point_list.m_points.emplace_back(point, 1 << FULL_SCAN, hitbox_num);

                math::VectorTransform(hitbox->mins, bone_transform, point);
                point += right * -r * 0.5f;
                point_list.m_points.emplace_back(point, 1 << FULL_SCAN, hitbox_num);

                math::VectorTransform(center, bone_transform, point);
                point_list.m_points.emplace_back(point, 1 << FULL_SCAN, hitbox_num);
                break;
            case hitbox_pelvis:
            case hitbox_body:

                math::VectorTransform(center, bone_transform, point);
                point_list.m_points.emplace_back(point, mode, hitbox_num);

                math::VectorTransform(center, bone_transform, point);
                point += right * r;
                point_list.m_points.emplace_back(point, mode, hitbox_num);

                math::VectorTransform(center, bone_transform, point);
                point += right * -r;
                point_list.m_points.emplace_back(point, mode, hitbox_num);

                math::VectorTransform(center, bone_transform, point);
                point_list.m_points.emplace_back(point, 1 << FULL_SCAN, hitbox_num);
                break;
            case hitbox_thorax:
            case hitbox_upper_chest:
                math::VectorTransform(center, bone_transform, point);
                point_list.m_points.emplace_back(point, 0, hitbox_num);

                math::VectorTransform(center, bone_transform, point);
                point += right * r;
                point_list.m_points.emplace_back(point, 0, hitbox_num);

                math::VectorTransform(center, bone_transform, point);
                point += right * -r;
                point_list.m_points.emplace_back(point, 0, hitbox_num);

                math::VectorTransform(center, bone_transform, point);
                point_list.m_points.emplace_back(point, 1 << FULL_SCAN, hitbox_num);
                break;
            case hitbox_chest:
                point = center;
                math::VectorTransform(center, bone_transform, point);
                point_list.m_points.emplace_back(point, mode, hitbox_num);

                math::VectorTransform(center, bone_transform, point);
                point += right * r;
                point_list.m_points.emplace_back(point, 0, hitbox_num);

                math::VectorTransform(center, bone_transform, point);
                point += right * -r;
                point_list.m_points.emplace_back(point, 0, hitbox_num);

                math::VectorTransform(center, bone_transform, point);
                point_list.m_points.emplace_back(point, 1 << FULL_SCAN, hitbox_num);
                break;
            case hitbox_r_calf:
            case hitbox_l_calf:
            case hitbox_r_thigh:
            case hitbox_l_thigh:
            case hitbox_r_upper_arm:
            case hitbox_l_upper_arm:
                math::VectorTransform(center, bone_transform, point);
                point_list.m_points.emplace_back(point, 0, hitbox_num);
                break;
        }

        point_list.hdr = studio_model;
        point_list.info = info;
        {
            info->m_hitboxes.push_back(point_list);
        }
    }
}

bool aimbot_t::collides(math::custom_ray_t ray, ent_info_t* info, bone_array_t bones[128], float mult)
{
    const auto* studio_model = info->m_ent->model();
    if (!studio_model)
        return false;
    auto* hdr = g.m_interfaces->model_info()->get_studio_model(studio_model);
    if (!hdr)
        return false;
    auto* hitbox_set = hdr->hitbox_set(info->m_ent->hitbox_set());
    if (!hitbox_set)
        return false;
    vec3_t mins{}, maxs{};
    for (auto i = 0; i < hitbox_set->hitbox_count; i++)
    {
        auto* hitbox = hitbox_set->hitbox(i);
        if (!hitbox)
            return false;

        if (hitbox && hitbox->radius > 0)
        {
            vec3_t vMin, vMax;
            math::VectorTransform(hitbox->mins, bones[hitbox->bone], vMin);
            math::VectorTransform(hitbox->maxs, bones[hitbox->bone], vMax);

            float m1, m2;
            const auto dist = math::distSegmentToSegmentSqr(ray.m_start, ray.m_end, vMin, vMax, m1, m2);
            if (dist <= (hitbox->radius * hitbox->radius) * mult)
            {
                return true;
            }
        }
    }
    return false;
}

bool aimbot_t::get_aim_matrix(ang_t angle, bone_array_t* bones)
{
    auto* state = g.m_local->get_anim_state();
    if (!state)
        return false;

    float backup_pose[24];
    animation_layer_t backup_layers[15];

    g.m_local->GetAnimLayers(backup_layers);
    g.m_local->GetPoseParameters(backup_pose);

    g.m_local->SetAnimLayers(g.m_layers);
    g.m_local->SetPoseParameters(g.m_poses);

    vec3_t mins = g.m_local->mins();
    vec3_t maxs = g.m_local->maxs();
    if (g.m_bones_setup)
    {
        g.m_local->maxs() = g.bones_maxs;
        g.m_local->mins() = g.bones_mins;
    }
    float flPitch = (angle.x + 90.f) / 180.f;

    g.m_local->pose_parameters()[12] = flPitch;

    g.m_local->set_abs_angles(ang_t(0.f, g.m_abs_yaw, 0.f));

    CIKContext backup_ik;
    memcpy(&backup_ik, &g.m_ipk, sizeof(CIKContext));

    g.m_interfaces->mdlcache()->begin_coarse_lock();
    g.m_interfaces->mdlcache()->begin_lock();
    // we have setup this record bones.
    const auto ret =
        g_bones.BuildBonesStripped(g.m_local, bone_used_by_anything & ~bone_used_by_bone_merge, bones, &g.m_ipk);
    g.m_interfaces->mdlcache()->end_lock();
    g.m_interfaces->mdlcache()->end_coarse_lock();

    memcpy(&g.m_ipk, &backup_ik, sizeof(CIKContext));

    g.m_local->SetAnimLayers(backup_layers);
    g.m_local->SetPoseParameters(backup_pose);

    g.m_local->maxs() = maxs;
    g.m_local->mins() = mins;
    return ret;
}
class autowall_job_data
{
public:
    autowall_job_data(hitbox_helper_t* hitbox, ent_info_t* info)
        : hitbox(hitbox), info(info)
    {
    }
    hitbox_helper_t* hitbox;
    ent_info_t* info;
    aim_point_t* selected_point = nullptr;
    bool out = false;
    float out_damage = 0;
    float out_weight = 0;
};
static Semaphore dispatchSem;
static SharedMutex smtx;
static Mutex smtx2;
void autowall_job_func(void* data)
{
    dispatchSem.Post();
    smtx.rlock();
    smtx.runlock();

    const auto job_data = static_cast<autowall_job_data*>(data);
    bool should_full_scan = false;
    math::custom_ray_t ray;
    for ( uint32_t i = 0; i < job_data->hitbox->m_points.size(); ++i )
    {
        aim_point_t* point = &job_data->hitbox->m_points[i];
        if (!should_full_scan && point->m_mode & (1 << FULL_SCAN))
            continue;
        

        ang_t new_look = g.m_shoot_pos.look(point->m_point);
        if (settings::rage::selection::fov > 0 &&
            fabsf(g.m_shoot_pos.look(point->m_point).delta(new_look)) > settings::rage::selection::fov)
            continue;

        float weight = 0.f;
        int safe_points = 0;
        if (job_data->info->m_selected_record->m_fake_bones_setup && ((!job_data->info->m_selected_record->m_body_reliable &&
             job_data->info->m_selected_record->m_mode == resolver::RESOLVE_BODY) ||
            job_data->info->m_selected_record->m_mode == resolver::RESOLVE_WALK))
        { // check these safepoints on walk/lby flick
            ray.init(g.m_shoot_pos, point->m_point);

            bool should_continue = false;
            for (auto& i : job_data->info->m_selected_record->m_fake_bones)
                if (aimbot_t::collides(ray, job_data->info, i))
                { // if this shot collides with the fake hitboxes add on to our safepoints
                    if (job_data->info->m_selected_record->m_mode == resolver::RESOLVE_WALK)
                        safe_points++;
                    else
                    {
                        should_continue = true;
                        break;
                    }
                }

            if (should_continue)
                continue;

            weight += static_cast<float>(safe_points) / static_cast<float>(job_data->info->m_selected_record->m_fake_bones.size());
        }
        penetration::PenetrationInput_t in;
        in.m_from = g.m_local;
        in.m_start = g.m_shoot_pos;
        in.m_pos = point->m_point;
        in.m_target = job_data->info->m_ent;
        float damage = std::min<float>(100.f, static_cast<float>(job_data->info->m_ent->health()) + settings::rage::selection::lethal_damage);
        in.m_damage = in.m_damage_pen = std::min<float>(settings::rage::selection::min_damage, damage);

        penetration::PenetrationOutput_t out;

        job_data->info->m_selected_record->cache(-1);
        if (run(&in, &out))
        { // check wall penetration
            if (out.m_target != in.m_target || in.m_damage > out.m_damage)
                continue;
            should_full_scan = true;

            weight += out.m_damage / static_cast<float>(job_data->info->m_ent->health());

            if (weight < job_data->out_weight)
                continue;

            job_data->out_weight = weight;

            job_data->selected_point = point;
            job_data->out_damage = out.m_damage;
        }
    }
    if (job_data->selected_point)
        job_data->out = true;
}

bool aimbot_t::get_best_point(ent_info_t* info, vec3_t& eye)
{
    if (info->m_hitboxes.empty())
        return false;
    // g.m_interfaces->mem_alloc( )->free( bones );
    // if ( info->m_shot_wanted > info->m_selected_record->m_sim_time )
    //	return false;
    float max_damage = 0;
    vec3_t selected_eye;
    aim_point_t* selected_point = nullptr;
    ang_t view;
    int max_safe_points;
    hitbox_helper_t* box;
    ang_t new_look;
    float max_weight = 0.f;
    float weight;
    int safe_points;
    math::custom_ray_t ray;
    penetration::PenetrationInput_t in;
    g.m_interfaces->engine()->get_view_angles(view);
    ang_t look;
    std::vector<autowall_job_data*> job_data = {};
    smtx.wlock();
    for (ent_info_t::helper_array::size_type i = 0; i < info->m_hitboxes.size(); i++)
    {
        job_data.emplace_back(new autowall_job_data(&info->m_hitboxes[i], info));
        Threading::QueueJobRef(autowall_job_func, job_data.back());
    }
    for (size_t i = 0; i < min(job_data.size(), Threading::numThreads); i++)
        dispatchSem.Wait();
    smtx.wunlock();
    Threading::FinishQueue();
    bool has_lethal_body_aim = false;
    for (auto i : job_data)
    {
        if (!i->out)
            continue;
        if (has_lethal_body_aim &&
            !(i->selected_point->m_mode & (1 << LETHAL_BAIM)))
            continue;
        if ((!selected_point || !(selected_point->m_mode & (1 << LETHAL_BAIM))) &&
            i->selected_point->m_mode & (1 << LETHAL_BAIM))
        {
            if (i->out_damage >= info->m_ent->health() + settings::rage::selection::lethal_damage)
            {
                if (!has_lethal_body_aim)
                    max_weight = 0;
                has_lethal_body_aim = true;
            }
            else if (has_lethal_body_aim)
                continue;
        }
        if (i->out_weight < max_weight)
             continue;
        max_weight = i->out_weight;

        selected_point = i->selected_point;
        max_damage = i->out_damage;
    }

    for (const auto i : job_data)
    {
        delete i;
    }
    job_data.clear();
    if (selected_point)
    {
        info->m_aim_point = selected_point;
        info->m_damage = max_damage;
        selected_point->m_shoot_pos = selected_eye;
        return true;
    }
    return false;
}

bool aimbot_t::best_target(ent_info_t*& info) const
{
    if (!m_best_target)
        return true;

    if ((info->m_ent->origin() - g.m_shoot_pos).length_sqr() <
        (m_best_target->m_ent->origin() - g.m_shoot_pos).length_sqr())
        return true;

    return false;
}

void aimbot_t::apply(bone_array_t *bones)
{
    if (m_best_target == nullptr ||
        m_best_target->m_aim_point == nullptr) // was there no good targets to shoot? m_aim_point should always be non
                                               // null at this point but for safety sake
        return;
    if (m_best_target != m_last_target)
        m_best_target->m_shot_wanted = m_best_target->m_selected_record->m_sim_time +
                                       settings::rage::general::delay_shot; // Unused for now, used for

    // don't auto stop if local is fakewalking.
    if (!settings::hvh::antiaim::fakewalk)
        g_movement.set_should_stop(true);

    autoscope();

    if (const auto bone_cache = &g.m_local->bone_cache())
    {
        bone_array_t* backup_cache = bone_cache->m_pCachedBones;
        const int backup_bone_count = bone_cache->m_CachedBoneCount;
        bone_cache->m_CachedBoneCount = 128;
        bone_cache->m_pCachedBones = bones;
        vec3_t eye = g.m_local->view_offset() + g.m_local->abs_origin();
        for (auto f = 0; f < 10; f++)
        {
             auto look = g.m_shoot_pos.look(m_best_target->m_aim_point->m_point);
             if (get_aim_matrix(look - g.m_local->aim_punch() * 2, bones))
             {
                //g.m_local->get_eye_pos(&eye);
                c_g::ModifyEyePosition(g.m_local->get_anim_state(), bones, &eye);
                if (fabsf(g.m_shoot_pos.z - eye.z) <= 0.01f)
                    break;
             }
        }
        g.m_shoot_pos = eye;
        bone_cache->m_pCachedBones = backup_cache;
        bone_cache->m_CachedBoneCount = backup_bone_count;
    }
    auto look = g.m_shoot_pos.look(m_best_target->m_aim_point->m_point);
    if (!check_hitchance(m_best_target, look, m_best_target->m_selected_record, m_best_target->m_aim_point))
        return;

    g_movement.set_should_unpeek(true);

    draw_hitboxes();
    g.m_cmd->m_tick_count = g.time_to_ticks(m_best_target->m_selected_record->m_pred_time + g.m_lerp);
    g.m_cmd->m_viewangles = look - g.m_local->aim_punch() * 2;
    g.m_cmd->m_buttons |= IN_ATTACK;
    g.m_shot = true;
    *g.m_packet = false;

    g_resolver.add_shot(m_best_target, m_best_target->m_damage, g.m_weapon_info->m_bullets,
                        m_best_target->m_selected_record, m_best_target->m_aim_point->m_point);
}

void aimbot_t::draw_hitboxes(player_t* player, bone_array_t* bones) const
{
    if (!player)
        player = m_best_target->m_ent;
    auto* hdr = g.m_interfaces->model_info()->get_studio_model(player->model());
    auto* set = hdr->hitbox_set(player->hitbox_set());

    if (bones == nullptr)
        bones = m_best_target->m_selected_record->m_bones;

    vec3_t origin;
    ang_t angles;

    for (auto i = 0; i < set->hitbox_count; i++)
    {
        auto* hitbox = set->hitbox(i);

        if (!hitbox)
            continue;
        if (hitbox->radius > 0)
        {
            matrix_t temp;

            math::AngleMatrix(hitbox->angle, &temp);
            matrix_t bone_transform;
            memcpy(&bone_transform, &bones[hitbox->bone], sizeof(matrix_t));
            if (hitbox->angle != ang_t())
                math::ConcatTransforms(bone_transform, temp, &bone_transform);

            vec3_t vMin, vMax;
            math::VectorTransform(hitbox->mins, bone_transform, vMin);
            math::VectorTransform(hitbox->maxs, bone_transform, vMax);

            g.m_interfaces->debug_overlay()->AddCapsuleOverlay(vMin, vMax, hitbox->radius, 255, 0, 0, 255, 4);
        }
        else
        {

            // convert rotation angle to a matrix.
            matrix_t rot_matrix;
            math::AngleMatrix(hitbox->angle, &rot_matrix);

            // apply the rotation to the entity input space (local).
            matrix_t matrix;
            math::ConcatTransforms(bones[hitbox->bone], rot_matrix, &matrix);

            // extract the compound rotation as an angle.
            ang_t bbox_angle;
            math::MatrixAngles(matrix, bbox_angle);

            // extract hitbox origin.
            auto origin1 = matrix.get_origin();
            g.m_interfaces->debug_overlay()->AddBoxOverlay(origin1, hitbox->mins, hitbox->maxs, bbox_angle, 255, 0, 0,
                                                           255, 4);
        }
    }
}

bool aimbot_t::check_hitchance(ent_info_t* info, ang_t& view, std::shared_ptr<player_record_t> record,
                               aim_point_t* point)
{
    size_t total_hits{};
    const auto needed_hits{static_cast<size_t>((settings::rage::selection::hitchance * 255 / 100.f))};

    const auto inaccuracy = g.m_weapon->inaccuracy();
    const auto spread = g.m_weapon->get_spread();
    const auto start{g.m_shoot_pos};

    auto* studio_model = g.m_interfaces->model_info()->get_studio_model((info)->m_ent->model());
    if (!studio_model)
        return false;

    auto* hitbox_set = studio_model->hitbox_set(info->m_ent->hitbox_set());
    vec3_t mins{}, maxs{};

    if (point->m_hitbox < 0 || point->m_hitbox >= hitbox_set->hitbox_count)
        return false;
    auto* hitbox = hitbox_set->hitbox(point->m_hitbox);

    if (!(hitbox && hitbox->radius > 0))
        return false;

    matrix_t bone_transform;
    memcpy(&bone_transform, &record->m_bones[hitbox->bone], sizeof(matrix_t));
    if (hitbox->angle != ang_t())
    {
        matrix_t temp;

        math::AngleMatrix(hitbox->angle, &temp);
        math::ConcatTransforms(bone_transform, temp, &bone_transform);
    }

    vec3_t vMin, vMax;
    math::VectorTransform(hitbox->mins, bone_transform, vMin);
    math::VectorTransform(hitbox->maxs, bone_transform, vMax);

    vec3_t forward{}, right{}, up{};

    view.vectors(&forward, &right, &up);

    vec3_t origin;
    ang_t angles;
    for (auto i = 0; i < 255; i++)
    {
        const auto weapon_spread = math::calculate_spread(g.m_weapon, i, inaccuracy, spread, false);

        vec3_t dir = (forward + (right * weapon_spread.x) + (up * weapon_spread.y)).normalized();
        
        const auto end = start + (dir * g.m_weapon_info->m_range);

        float m1, m2;
        float dist = math::distSegmentToSegmentSqr(g.m_shoot_pos, end, vMin, vMax, m1, m2);
        // RayTracer::TraceHitbox( ray_1, box, trace, RayTracer::Flags_NONE );
        if (dist <= hitbox->radius * hitbox->radius)
        {
            total_hits++;
            if (total_hits >= needed_hits)
                return true;
        }
        if (((255 - i) + total_hits) < needed_hits)
            return false;
    }
    return false;
}

bool aimbot_t::autoscope()
{
    if (!g.m_cmd)
        return false;

    if (g.m_weapon_type != WEAPONTYPE_SNIPER_RIFLE)
        return false;

    if (g.m_weapon->zoom_level() >= 1)
        return false;

    g.m_cmd->m_buttons &= ~IN_ATTACK;
    g.m_cmd->m_buttons |= IN_ATTACK2;

    return true;
}

std::shared_ptr<player_record_t> aimbot_t::last_record(ent_info_t* info)
{
    std::shared_ptr<player_record_t> best = nullptr;
    for (auto& i : info->m_records)
    {
        if (!i || !i->m_setup || !i->valid())
            continue;
        if (i->m_dormant)
            break;
        best = i;
    }
    return best;
}

player_record_t* aimbot_t::best_record(ent_info_t* info)
{
    player_record_t* best = nullptr;
    for (auto& i : info->m_records)
    {
        if (!i->m_setup || !i->valid())
            continue;
        if (i->m_dormant)
            break;
        if (i->m_shot)
            continue;
        if (i->m_mode == resolver::RESOLVE_BODY || i->m_mode == resolver::RESOLVE_WALK)
            return i.get();
        if (!best)
            best = i.get();
    }
    return best;
}

void aimbot_t::backup_players(const bool restore)
{
    for (auto i{1}; i <= g.m_interfaces->globals()->m_max_clients; ++i)
    {
        const auto& player_data = g_player_manager.m_ents[i - 1];

        if (!player_data.m_valid)
            continue;
        if (!player_data.m_ent)
            continue;

        if (restore)
            m_backup[i - 1].restore(player_data.m_ent);
        else
            m_backup[i - 1].store(player_data.m_ent);
    }
}

//			Scrapping this since it ended up causing more lag than it was worth
//			Just check best target that we can find

// bool add_to_threads( std::vector<std::shared_ptr<hcThreadObject>> &objects, ent_info_t* info, int id ) {
//	if ( !info->m_aim_point )
//		return false;
//	auto look = g.m_shoot_pos.look( info->m_aim_point->m_point );
//	auto* studio_model = g.m_interfaces->model_info( )->get_studio_model( ( info )->m_ent->model( ) );
//	if ( !studio_model )
//		return false;
//	auto* hitbox_set = studio_model->hitbox_set( info->m_ent->hitbox_set( ) );
//	std::vector<math::hitbox_t> hit_boxes;
//	vec3_t mins{}, maxs{};
//
//	for ( auto i = 0; i < hitbox_set->hitbox_count; i++ ) {
//		if ( i != info->m_aim_point->m_hitbox )
//			continue;
//		auto* hitbox = hitbox_set->hitbox( i );
//
//		if ( hitbox && hitbox->radius > 0 ) {
//			matrix_t bone_transform;
//			memcpy( &bone_transform, &info->m_selected_record->m_bones[ hitbox->bone ], sizeof( matrix_t ) );
//			if ( hitbox->angle != ang_t( ) ) {
//				matrix_t temp;
//
//				math::AngleMatrix( hitbox->angle, &temp );
//				math::ConcatTransforms( bone_transform, temp, &bone_transform );
//			}
//
//			vec3_t vMin, vMax;
//			math::VectorTransform( hitbox->mins, bone_transform, vMin );
//			math::VectorTransform( hitbox->maxs, bone_transform, vMax );
//			hit_boxes.emplace_back( vMin, vMax, hitbox->radius );
//		}
//	}
//	if ( hit_boxes.empty( ) )
//		return false;
//	//g_thread_handler.wait( ); // wait until all threads are finished
//	objects.emplace_back( std::make_shared<hcThreadObject>(g.m_weapon, look, id, hit_boxes) );
//	return true;
// }

class mpThreadObject
{
public:
    bool ret_state = false;
    bool finished = false;
    bool read = false;
    ent_info_t* info = nullptr;
    void run();
    mpThreadObject(ent_info_t* info_)
    {
        info = info_;
    }
};

void mpThreadObject::run()
{
    //auto* studio_model = g.m_interfaces->model_info()->get_studio_model(info->m_ent->model());
    //if (!studio_model)
    //{
    //    ret_state = false;
    //    finished = true;
    //    return;
    //}
    //info->m_selected_record = g_resolver.FindIdealRecord(info);
    //if (info->m_selected_record)
    //{
    //    aimbot_t::get_points(info, studio_model);
    //    if (g_aimbot.best_target(info) && g_aimbot.get_best_point(info, eye))
    //    {
    //        g_aimbot.m_best_target = info;
    //        //std::unique_lock<std::mutex> lock(g_thread_handler.queue_mutex2);
    //        //ret_state = true;
    //        //finished = true;
    //        return;
    //        // valid_targets.push_back( info );
    //        // if ( info->m_damage >= info->m_ent->health( ) )
    //        //	break;
    //        // continue;
    //    }
    //}
    //info->m_selected_record = g_resolver.FindIdealBackRecord(info);
    //if (info->m_selected_record)
    //{
    //    aimbot_t::get_points(info, studio_model);
    //    if (g_aimbot.best_target(info) && g_aimbot.get_best_point(info, eye))
    //    {
    //        g_aimbot.m_best_target = info;
    //        // std::unique_lock<std::mutex> lock(g_thread_handler.queue_mutex2);
    //        // ret_state = true;
    //        // finished = true;
    //        return;
    //        // valid_targets.push_back( info );
    //        // if ( info->m_damage >= info->m_ent->health( ) )
    //        //	break;
    //    }
    //}
    //info->m_selected_record = aimbot_t::last_record(info);
    //if (info->m_selected_record)
    //{
    //    if (g_aimbot.best_target(info) && g_aimbot.get_best_point(info, eye))
    //    {
    //        g_aimbot.m_best_target = info;
    //        // std::unique_lock<std::mutex> lock(g_thread_handler.queue_mutex2);
    //        // ret_state = true;
    //        // finished = true;
    //        return;
    //        // valid_targets.push_back( info );
    //        // if ( info->m_damage >= info->m_ent->health( ) )
    //        //	break;
    //    }
    //}
    //std::unique_lock<std::mutex> lock(g_thread_handler.queue_mutex2);
    //ret_state = false;
    //finished = true;
}

std::vector<ent_info_t*> aimbot_t::mp_threading()
{
    //std::vector<ent_info_t*> targets = {};

    vec3_t eye;
    ang_t look;
    bone_array_t* backup_cache;
    for (auto& info : m_targets)
    {
        auto* studio_model = g.m_interfaces->model_info()->get_studio_model(info->m_ent->model());
        if (!studio_model)
        {
            continue;
        }
        info->m_selected_record = resolver::FindIdealRecord(info);
        if (info->m_selected_record)
        {
            aimbot_t::get_points(info, studio_model);
            if (g_aimbot.best_target(info) && g_aimbot.get_best_point(info, eye))
            {
                g_aimbot.m_best_target = info;
                // std::unique_lock<std::mutex> lock(g_thread_handler.queue_mutex2);
                // ret_state = true;
                // finished = true;
                continue;
                // valid_targets.push_back( info );
                // if ( info->m_damage >= info->m_ent->health( ) )
                //	break;
                // continue;
            }
        }
        //info->m_selected_record = g_resolver.FindIdealBackRecord(info);
        //if (info->m_selected_record)
        //{
        //    aimbot_t::get_points(info, studio_model);
        //    if (g_aimbot.best_target(info) && g_aimbot.get_best_point(info, eye))
        //    {
        //        g_aimbot.m_best_target = info;
        //        // std::unique_lock<std::mutex> lock(g_thread_handler.queue_mutex2);
        //        // ret_state = true;
        //        // finished = true;
        //        continue;
        //        // valid_targets.push_back( info );
        //        // if ( info->m_damage >= info->m_ent->health( ) )
        //        //	break;
        //    }
        //}
        info->m_selected_record = aimbot_t::last_record(info);
        if (info->m_selected_record)
        {
            if (g_aimbot.best_target(info) && g_aimbot.get_best_point(info, eye))
            {
                g_aimbot.m_best_target = info;
                // std::unique_lock<std::mutex> lock(g_thread_handler.queue_mutex2);
                // ret_state = true;
                // finished = true;
                continue;
                // valid_targets.push_back( info );
                // if ( info->m_damage >= info->m_ent->health( ) )
                //	break;
            }
        }
        //objects.push_back(std::make_shared<mpThreadObject>(info, eye));
    }

    //std::for_each(std::begin(objects), std::end(objects),
    //              [&](std::shared_ptr<mpThreadObject> obj)
    //              {
    //                  obj->run();
    //              });

    //for (const auto& object : objects)
    //{
    //    if (object->ret_state)
    //        targets.push_back(object->info);
    //}
    //return targets;
    return {};
}

void aimbot_t::on_tick()
{
    m_list_eye_pos.clear();
    // g.m_cmd->m_viewangles -= g.m_local->aim_punch( ) * 2;
    if (!settings::rage::general::enabled)
        return;

    if (!g.m_weapon_info)
        return;

    if (g.m_lag == 0)
        return;

    if (!g.m_can_fire)
    {
        return;
    }

    if (!settings::rage::general::key)
        return;

    if (!settings::rage::general::auto_shoot && !(g.m_cmd->m_buttons & IN_ATTACK))
        return;

    backup_players(false); // backup player vars before we change them

    m_last_target = m_best_target;
    m_best_target = nullptr;

    get_targets(); // get all possible targets

    const auto bones = static_cast<bone_array_t*>(g.m_interfaces->mem_alloc()->alloc(sizeof(bone_array_t) * 128));

    if (const auto bone_cache = &g.m_local->bone_cache())
    {
        auto look = ang_t(0,0,0);
        bone_array_t* backup_cache = bone_cache->m_pCachedBones;
        const int backup_bone_count = bone_cache->m_CachedBoneCount;
        bone_cache->m_CachedBoneCount = 128;
        bone_cache->m_pCachedBones = bones;
        vec3_t eye = g.m_local->view_offset() + g.m_local->abs_origin();
        if (get_aim_matrix(look - g.m_local->aim_punch() * 2, bones))
        {
            // g.m_local->get_eye_pos(&eye);
            c_g::ModifyEyePosition(g.m_local->get_anim_state(), bones, &eye);
            g.m_shoot_pos = eye;
        }
        bone_cache->m_pCachedBones = backup_cache;
        bone_cache->m_CachedBoneCount = backup_bone_count;
    }

    std::vector<ent_info_t*> valid_targets = mp_threading();

    apply(bones); // look and shoot at the best point
    g.m_interfaces->mem_alloc()->free(bones);

    backup_players(true); // restore
}
