#include "aimbot.h"

#include "movement.h"
#include "bones.h"
#include "hooks.h"
#include "penetration.h"
#include "ray_tracer.h"
#include "resolver.h"
#include "thread_handler.h"

class hcThreadObject {
public:
	bool ret_state = false;
	bool read = false;
	weapon_t* weapon;
	ang_t view;
	int id = 0;
	bool finished = false;

	std::vector<RayTracer::Hitbox> hit_boxes = {};
	void run( );
	hcThreadObject( weapon_t* weapon_, ang_t view_, int id_, std::vector<RayTracer::Hitbox> hitboxes_ ) {
		weapon = weapon_;
		view = view_;
		id = id_;
		hit_boxes = hitboxes_;
	}
};

void hcThreadObject::run ( ) {
	size_t     total_hits{ };
	const auto needed_hits{ static_cast< size_t >( ( settings::rage::selection::hitchance * 255 / 100.f ) ) };

	const auto inaccuracy = g.m_weapon->inaccuracy( );
	const auto spread = g.m_weapon->get_spread( );
	const auto start{ g.m_shoot_pos };

	static auto surface_predicate = [ ]( const RayTracer::Hitbox& a, const RayTracer::Hitbox& b ) {
		const float area_1 = ( M_PI * powf( a.m_radius, 2 ) * a.m_len ) + ( 4.f / 3.f * M_PI * a.m_radius );
		const float area_2 = ( M_PI * powf( b.m_radius, 2 ) * b.m_len ) + ( 4.f / 3.f * M_PI * b.m_radius );

		return area_1 < area_2;
	};
	std::sort( hit_boxes.begin( ), hit_boxes.end( ), surface_predicate );


	vec3_t forward{}, right{}, up{};

	view.vectors( &forward, &right, &up );

	vec3_t origin;
	ang_t angles;
	for ( auto i = 0; i < 255; i++ ) {
		const auto weapon_spread = math::calculate_spread( g.m_weapon, i, inaccuracy, spread, false );

		auto dir = ( forward + ( right * weapon_spread.x ) + ( up * weapon_spread.y ) );
		dir /= dir.length( );
		const auto end = start + ( dir * g.m_weapon_info->m_range );

		math::custom_ray ray( g.m_shoot_pos, end );

		RayTracer::Ray ray_1( ray.m_start, ray.m_end );
		for ( auto& box : hit_boxes ) {
			RayTracer::Trace trace;
			float m1, m2;
			auto dist = math::distSegmentToSegment( ray.m_start, ray.m_end, box.m_mins, box.m_maxs, m1, m2 );
			//RayTracer::TraceHitbox( ray_1, box, trace, RayTracer::Flags_NONE );
			if ( dist <= box.m_radius ) {
				total_hits++; 
				finished = true;
				break;
			}
			//RayTracer::TraceHitbox( ray_1, box, trace, RayTracer::Flags_NONE );
			//if ( trace.m_hit ) {
			//	total_hits++;
			//	break;
			//}
		}
		//for ( auto &box : hit_boxes ) {
		//	trace_t trace;
		//	if ( !box.m_box ) {
		//		if ( box.m_radius >= math::dist_Segment_to_Segment( ray, box.m_min, box.m_max ) ) {
		//			total_hits++;
		//			break;
		//		}
		//	}
		//	else {
		//
		//		matrix_t rot_matrix;
		//		math::AngleMatrix( box.m_rot, &rot_matrix );
		//
		//		// apply the rotation to the entity input space (local).
		//		matrix_t matrix;
		//		math::ConcatTransforms( record->m_bones[ box.m_bone ], rot_matrix, &matrix );
		//		
		//		if ( math::IntersectRayWithOBB( ray.m_start, ray.m_ray_dir, matrix, box.m_min, box.m_max, 0.0f, &trace ) ) {
		//			total_hits++;
		//			break;
		//		}
		//	}
		//}
		if ( total_hits >= needed_hits ) {
			ret_state = true;
			finished = true;
			return;
		}
		if ( ( ( 255 - i ) + total_hits ) < needed_hits ) {
			ret_state = false;
			finished = true;
			return;
		}
	}
	ret_state = false;
	finished = true;
}

bool aimbot_t::valid(ent_info_t *ent) {
	return ent->m_ent && ent->m_valid && !ent->m_teamate && !ent->m_dormant && !ent->m_ent->gun_game_immunity( );
}
void aimbot_t::get_targets() {
	m_targets.clear( );
	for ( auto &ent : g_player_manager.m_ents ) {
		ent.m_damage *= 1 * valid(&ent); // reset damage for every target
		if (valid(&ent))
			m_targets.push_back( &ent );
	}
}
vec3_t get_hitbox_edge_in_dir( vec3_t start, vec3_t end, vec3_t mins, vec3_t maxs, float radius ) { // i think the logic here is right??
	vec3_t center = ( maxs + mins ) * 0.5f;
	vec3_t dir = ( start - end ).normalized( );

	vec3_t dir_to_maxs = maxs - end;
	float dir_to_maxs_dot = (dir).dot( dir_to_maxs );
	float dir_to_maxs_len = dir_to_maxs.normalize( );

	vec3_t dir_to_mins = mins - end;
	float dir_to_mins_dot = (dir).dot( dir_to_mins );
	float dir_to_mins_len = dir_to_mins.length( );


	vec3_t hitbox_end;
	float len_to_hitbox_end = 0.f;
	int perpendicular = 0;
	float greater_dot = 0.f;

	if ( dir_to_maxs_dot > dir_to_mins_dot ) { // select the hitbox end that has the dir coming towards it 
		len_to_hitbox_end = dir_to_maxs_len;
		greater_dot = dir_to_maxs_dot;
		hitbox_end = maxs;
		perpendicular = 1;
	}
	else if ( dir_to_mins_dot > dir_to_maxs_dot ) {
		len_to_hitbox_end = dir_to_mins_len;
		greater_dot = dir_to_mins_dot;
		hitbox_end = mins;
		perpendicular = 1;
	}
	else if( dir_to_maxs_len > dir_to_mins_len ) {
		len_to_hitbox_end = dir_to_mins_len;
		hitbox_end = mins;
		perpendicular = 2;
	}
	else if ( dir_to_maxs_len < dir_to_mins_len ) {
		len_to_hitbox_end = dir_to_maxs_len;
		hitbox_end = maxs;
		perpendicular = 2;
	}

	if ( perpendicular == 2 ) {
		vec3_t step = end + dir;
		float step_dist = ( hitbox_end - step ).length( );
		float step_difference = len_to_hitbox_end - step_dist;
		float needed_dist = len_to_hitbox_end - radius;
		return end + ( dir * needed_dist / step_difference );
	}
	else if ( perpendicular == 0 ) {
		vec3_t step = end - dir;
		float end_dist = math::minimum_distance( mins, maxs, end );
		float step_dist = math::minimum_distance( mins, maxs, step );
		float step_difference = end_dist - step_dist;
		float needed_dist = end_dist - radius;
		return end + ( dir * needed_dist / step_difference );

	}
	else {
		vec3_t step = end + (-dir * greater_dot);
		float step_dist = math::minimum_distance( mins, maxs, step );
		if ( step_dist < radius ) {
			step = end - dir;
			float end_dist = math::minimum_distance( mins, maxs, end );
			step_dist = math::minimum_distance( mins, maxs, step );
			float step_difference = end_dist - step_dist;
			float needed_dist = end_dist - radius;
			return end + ( dir * needed_dist / step_difference );
		}
		else {
			step = end - dir;
			step_dist = ( hitbox_end - step ).length( );
			float step_difference = len_to_hitbox_end - step_dist;
			float needed_dist = len_to_hitbox_end - radius;
			return end + ( dir * needed_dist / step_difference );
		}
	}

}
void aimbot_t::get_points( ent_info_t *info, studio_hdr_t *studio_model ) {
	info->m_hitboxes.clear( );
	
	for ( auto hitbox_num = hitboxes::hitbox_max - 1; hitbox_num >= 0; hitbox_num-- ) {
		auto* hitbox = studio_model->hitbox_set( info->m_ent->hitbox_set( ) )->hitbox( hitbox_num );
		if ( !hitbox )
			return;
		auto n = 5; // TODO: Add option on menu to adjust points per hitbox
		auto scale = settings::rage::selection::point_scale;
		int select = -1;
		switch ( hitbox_num ) {
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
			scale = settings::rage::selection::body_scale;
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
		if ( select == -1 || !( settings::rage::selection::hitboxes & ( 1 << select ) ) )
			continue;
		hitbox_helper_t point_list{ hitbox_num };

		auto center = ( hitbox->mins + hitbox->maxs ) / 2.f;

		auto mode = settings::rage::selection::body_aim_lethal ? lethal : 0;
		const auto r = hitbox->radius * ( scale / 100.f );
		constexpr const auto rotation = 0.70710678f;
		switch ( hitbox_num ) {
		case hitbox_head:

			// top/back 45 deg.
			// this is the best spot to shoot at.
			point_list.m_points.emplace_back( hitbox->maxs, 0, hitbox_num );

			point_list.m_points.emplace_back( hitbox->maxs, 0, hitbox_num );
			point_list.m_points.emplace_back( hitbox->mins, 0, hitbox_num );

			point_list.m_points.emplace_back( center, 0, hitbox_num );

			// back.
			point_list.m_points.emplace_back( vec3_t( hitbox->maxs.x, hitbox->maxs.y - r, hitbox->maxs.z ), 0, hitbox_num );

			break;
		case hitbox_body:
			// center. safe
			point_list.m_points.emplace_back( center, mode, hitbox_num );

			// back. safe
			point_list.m_points.emplace_back( vec3_t( center.x, center.y - r * 0.5f, center.z ), mode, hitbox_num );

			break;
		case hitbox_pelvis:
		case hitbox_upper_chest:
			// center. safe
			point_list.m_points.emplace_back( center, mode, hitbox_num );

			// back. safe
			point_list.m_points.emplace_back( vec3_t( center.x, center.y - r * 0.5f, center.z ), mode, hitbox_num );

			point_list.m_points.emplace_back( hitbox->mins, 0, hitbox_num );
			break;
		case hitbox_thorax:
		case hitbox_chest:
			point_list.m_points.emplace_back( center, mode, hitbox_num );
			point_list.m_points.emplace_back( hitbox->mins, 0, hitbox_num );
			// add center.

			if ( g.m_interfaces->globals( )->m_frametime < 1. / 60. )
				point_list.m_points.emplace_back( vec3_t{ center.x, center.y - r * 0.5f, center.z }, 0, hitbox_num );
			break;
		case hitbox_r_calf:
		case hitbox_l_calf:
			point_list.m_points.emplace_back( center, mode, hitbox_num );
			break;
		case hitbox_r_thigh:case hitbox_l_thigh:
			point_list.m_points.emplace_back( center, mode, hitbox_num );
			break;

			// arms get only one point.
		case hitbox_r_upper_arm:case hitbox_l_upper_arm:
			point_list.m_points.emplace_back( center, mode, hitbox_num );
			break;
		default:;
			break;
		}

		matrix_t bone_transform;
		memcpy( &bone_transform, &info->m_selected_record->m_bones[ hitbox->bone ], sizeof( matrix_t ) );
		if ( hitbox->angle != ang_t( ) ) {
			matrix_t temp;

			math::AngleMatrix( hitbox->angle, &temp );
			math::ConcatTransforms( bone_transform, temp, &bone_transform );
		}
		for ( int i = 0; i < point_list.m_points.size(); i++ ) {
			math::VectorTransform( point_list.m_points[i].m_point, bone_transform, point_list.m_points[i].m_point );
		}
		if ( hitbox_num == hitbox_head ) {
			point_list.m_points[ 0 ].m_point.z += r;

			const auto resolve_mode = info->m_selected_record->m_mode;
			if ( resolve_mode == resolver::RESOLVE_STAND1 || resolve_mode == resolver::RESOLVE_STAND2 || resolve_mode == resolver::RESOLVE_WALK || resolve_mode == resolver::RESOLVE_BODY )
				for ( auto i = 0; i < 2; i++ ) {
					int hitbox_count = 1;
					vec3_t temp_point = point_list.m_points[ hitbox_count ].m_point;
					vec3_t accumulated_point = temp_point;
					int acum_count = 0;
					resolver_data::mode_data* mode_data = nullptr;
					if ( resolve_mode == resolver::RESOLVE_STAND1 )
						mode_data = &info->m_resolver_data.m_mode_data[ resolver_data::STAND1 ];
					if ( resolve_mode == resolver::RESOLVE_STAND2 )
						mode_data = &info->m_resolver_data.m_mode_data[ resolver_data::STAND2 ];
					if ( ( !info->m_selected_record->m_body_reliable && resolve_mode == resolver::RESOLVE_BODY ) || resolve_mode == resolver::RESOLVE_WALK )
						mode_data = &info->m_resolver_data.m_mode_data[ resolver_data::LBY_MOVING ];
					if ( resolve_mode == resolver::RESOLVE_STAND1 ) {
						for ( auto i1 = 0; i1 < mode_data->m_dir_data.size(); i1++ ) {
							if ( i1 == mode_data->m_index )
								continue;
							auto& dir_data = mode_data->m_dir_data[ i1 ];
							if ( !dir_data.dir_enabled )
								continue;
							vec3_t new_avg_point = temp_point;
							math::VectorTransform( new_avg_point, bone_transform, new_avg_point );
							if ( hitbox_num == hitbox_head && hitbox_count == 1 )
								new_avg_point.z += r;
							accumulated_point += new_avg_point;
							acum_count++;
						}
					}

					math::VectorTransform( temp_point, bone_transform, temp_point );
					if ( hitbox_num == hitbox_head && hitbox_count == 1 )
						temp_point.z += r;

					accumulated_point /= acum_count;
					accumulated_point -= temp_point;
					float len = math::minimum_distance( hitbox->mins, hitbox->maxs, accumulated_point );
					if ( len > r )
						point_list.m_points[ hitbox_count ].m_point = get_hitbox_edge_in_dir( temp_point, accumulated_point, hitbox->mins, hitbox->maxs, r );
					else
						point_list.m_points[ hitbox_count ].m_point = temp_point + accumulated_point;
					hitbox_count++;
				}
		}
		



		point_list.hdr = studio_model;
		point_list.info = info;
		{
			info->m_hitboxes.push_back( point_list );
		}
	}
	static auto surface_predicate = [](const hitbox_helper_t& a, const hitbox_helper_t& b) {

		auto* hitbox = a.hdr->hitbox_set(a.info->m_ent->hitbox_set())->hitbox(a.m_index);
		auto* hitbox_1 = a.hdr->hitbox_set(a.info->m_ent->hitbox_set())->hitbox(b.m_index);
		float len = (hitbox->maxs - hitbox->mins).length_sqr();
		float len_1 = (hitbox_1->maxs - hitbox_1->mins).length_sqr();
		const float area_1 = (M_PI * powf(hitbox->radius, 2) * len) + (4.f / 3.f * M_PI * hitbox->radius);
		const float area_2 = (M_PI * powf(hitbox_1->radius, 2) * len_1) + (4.f / 3.f * M_PI * hitbox_1->radius);

		return area_1 < area_2;
	};
	{
		std::sort( info->m_hitboxes.begin( ), info->m_hitboxes.end( ), surface_predicate );
	}
}

bool aimbot_t::collides( math::custom_ray ray, ent_info_t *info, bone_array_t bones[128], float add ) {
	auto* studio_model = info->m_ent->model( );
	if ( !studio_model )
		return false;
	auto* hdr = g.m_interfaces->model_info( )->get_studio_model( studio_model );
	if ( !hdr )
		return false;
	auto* hitbox_set = hdr->hitbox_set( info->m_ent->hitbox_set( ) );
	if ( !hitbox_set )
		return false;
	vec3_t mins{}, maxs{};
	for ( auto i = 0; i < hitbox_set->hitbox_count; i++ ) {
		auto* hitbox = hitbox_set->hitbox( i );
		if ( !hitbox )
			return false;

		if ( hitbox && hitbox->radius > 0 ) {
			vec3_t vMin, vMax;
			math::VectorTransform( hitbox->mins, bones[ hitbox->bone ], vMin );
			math::VectorTransform( hitbox->maxs, bones[ hitbox->bone ], vMax );

			float m1, m2;
			const auto dist = math::distSegmentToSegment( ray.m_start, ray.m_end, vMin, vMax, m1, m2 );
			if ( dist <= hitbox->radius + add ) {
				return true;
			}
		}
	}
	return false;
}

bool aimbot_t::get_aim_matrix ( ang_t angle, bone_array_t *bones ) {
	auto *state = g.m_local->get_anim_state( );
	if ( !state )
		return false;
	float backup_pose = g.m_local->pose_parameters( )[ 12 ];
	
	angle.x = std::clamp( angle.x, -90.f, 90.f );
	
	g.m_local->pose_parameters( )[ 12 ] = ( angle.x + 90.f ) / 180.f;

	CIKContext backup_ik;
	memcpy( &backup_ik, &g.m_ipk, sizeof( CIKContext ) );

	g.m_interfaces->mdlcache()->begin_coarse_lock();
	g.m_interfaces->mdlcache()->begin_lock();
	// we have setup this record bones.
	const auto ret = g_bones.BuildBonesStripped( g.m_local, bone_used_by_hitbox, bones, &g.m_ipk );
	g.m_interfaces->mdlcache()->end_lock();
	g.m_interfaces->mdlcache()->end_coarse_lock();

	memcpy( &g.m_ipk, &backup_ik, sizeof( CIKContext ) );

	g.m_local->pose_parameters( )[ 12 ] = backup_pose;

	return ret;
}

bool aimbot_t::get_best_point( ent_info_t* info, bone_array_t* bones, vec3_t &eye ) {
	//g.m_interfaces->mem_alloc( )->free( bones );
	float max_damage = 0;
	vec3_t selected_eye;
	aim_point_t* selected_point = nullptr;
	ang_t view;
	int damage;
	int max_safe_points;
	hitbox_helper_t* box;
	ang_t new_look;
	bool found_match;
	float max_weight = 0.f;
	auto* const bone_cache = &g.m_local->bone_cache( );
	float weight;
	int safe_points;
	math::custom_ray ray;
	penetration::PenetrationInput_t in;
	g.m_interfaces->engine( )->get_view_angles( view );
	int bone_index;
	int current_index;
	ang_t look;
	int possible;
	for ( ent_info_t::helper_array::size_type i = 0; i < info->m_hitboxes.size( ); i++ ) {
		max_safe_points = 0;
		box = &info->m_hitboxes[ i ];
		for ( auto& point : box->m_points ) {
			new_look = eye.look( point.m_point );
			//if ( !found_match ) {
			//if ( get_aim_matrix( g.m_shoot_pos.look( point.m_point ) - g.m_local->aim_punch() * 2, bones ) ) {
			//	auto *const bone_cache = &g.m_local->bone_cache( );
			//	if ( bone_cache ) {
			//		bone_array_t *backup_cache = bone_cache->m_pCachedBones;
			//		bone_cache->m_pCachedBones = bones;
			//		g.m_local->get_eye_pos( &g.m_shoot_pos );
			//		new_look = g.m_shoot_pos.look( point.m_point );
			//		//m_list_eye_pos.emplace_back( new_look.x, g.m_shoot_pos );
			//		bone_cache->m_pCachedBones = backup_cache;
			//	}
			//}
			//}
			if ( settings::rage::selection::fov > 0 && fabsf( g.m_shoot_pos.look( point.m_point ).delta( view ) ) > settings::rage::selection::fov )
				continue;

			weight = 0.f;
			safe_points = 0;
			if ( ( !info->m_selected_record->m_body_reliable && info->m_selected_record->m_mode == resolver::RESOLVE_BODY ) || info->m_selected_record->m_mode == resolver::RESOLVE_WALK ) {
				ray.init( eye, point.m_point );

				//vec3_t mins = hitbox->mins; vec3_t maxs = hitbox->maxs;
				//math::VectorTransform( mins, info->m_selected_record->m_fake_bones_1[hitbox->bone], mins );
				//math::VectorTransform( maxs, info->m_selected_record->m_fake_bones_1[ hitbox->bone ], maxs );

				//if ( math::intersects_capsule( ray, mins, maxs, hitbox->radius ) ) {
				//	safe_points++;
				//} else if ( info->m_selected_record->m_body_update )
				//	continue;
				if ( !collides( ray, info, info->m_selected_record->m_fake_bones[ 0 ] ) ) {
					if ( info->m_selected_record->m_mode == resolver::RESOLVE_WALK )
						safe_points++;
					else continue;
				}

				//mins = hitbox->mins; maxs = hitbox->maxs;
				//math::VectorTransform( mins, info->m_selected_record->m_fake_bones_2[hitbox->bone], mins );
				//math::VectorTransform( maxs, info->m_selected_record->m_fake_bones_2[hitbox->bone], maxs );

				//if ( math::intersects_capsule( ray, mins, maxs, hitbox->radius ) ) {
				//	safe_points++;
				//} else if ( info->m_selected_record->m_body_update )
				//	continue;


				if ( !collides( ray, info, info->m_selected_record->m_fake_bones[ 1 ] ) ) {
					if ( info->m_selected_record->m_mode == resolver::RESOLVE_WALK )
						safe_points++;
					else continue;
				}
				weight += static_cast< float >( safe_points ) / 2.f;
			}


			in.m_from = g.m_local;
			in.m_start = eye;
			in.m_pos = point.m_point;
			in.m_target = info->m_ent;
			int damage = min( 100, info->m_ent->health( ) + settings::rage::selection::lethal_damage );
			in.m_damage = in.m_damage_pen = min( settings::rage::selection::min_damage, damage );
			if ( info->m_selected_record->m_mode == resolver::RESOLVE_STAND1 || info->m_selected_record->m_mode == resolver::RESOLVE_STAND2 ) {
				ray.init( eye, point.m_point );
				bone_index = 0;
				auto& mode_data = info->m_resolver_data.m_mode_data[ !( info->m_selected_record->m_mode == resolver::RESOLVE_STAND1 ) ];
				int possible = 0;
				for ( auto i1 = 0; i1 < mode_data.m_dir_data.size( ); i1++ ) {
					if ( mode_data.m_index == i1 )
						continue;
					auto& dir_data = mode_data.m_dir_data[ i1 ];
					if ( dir_data.dir_enabled ) {
						if ( collides( ray, info, info->m_selected_record->m_fake_bones[ bone_index ] ) )
							safe_points++;
						possible++;
					}
					bone_index++;
				}
				if( possible > 0 )
					weight += static_cast< float >( safe_points ) / static_cast< float >( possible );
			}

			penetration::PenetrationOutput_t out;

			info->m_selected_record->cache( -1 );
			if ( run( &in, &out ) ) {
				if ( out.m_target != in.m_target || in.m_damage > out.m_damage )
					continue;
				//if ( point.m_mode & prefer ) {
				//	selected_eye = eye;
				//	selected_point = &point;
				//	max_damage = out.m_damage;
				//	break;
				//}
				//if ( ( point.m_mode & lethal ) && ( static_cast<float>( info->m_ent->health( ) ) <= info->m_damage ) ) {
				//	selected_eye = eye;
				//	selected_point = &point;
				//	max_damage = out.m_damage;
				//	break;
				//}

				if ( selected_point )
					continue;

				penetration::PenetrationOutput_t sim_out;

				run( &in, &sim_out );
				weight += out.m_damage / info->m_ent->health();

				look = eye.look( point.m_point );

				if ( safe_points > max_safe_points )
					max_safe_points = safe_points;

				if ( weight < max_weight )
					continue;

				max_weight = weight;

				selected_eye = eye;
				selected_point = &point;
				max_damage = out.m_damage;
			}
			//else if ( out.m_damage <= -50.f ) {
			//	skip_player = true;
			//	break;
			//}
		}
		//if ( skip_player < -100.f ) {
		//	skip_player = true;
		//	break;
		//}
	}
	info->m_aim_point = selected_point;
	if ( selected_point ) {
		info->m_damage = max_damage;
		selected_point->m_shoot_pos = selected_eye;
		return true;
	}
	return false;
}

bool aimbot_t::best_target( ent_info_t *&info ) const {
	if ( !m_best_target )
		return true;

	if ( ( info->m_ent->origin() - g.m_shoot_pos ).length_sqr( ) < ( m_best_target->m_ent->origin( ) - g.m_shoot_pos ).length_sqr( ) )
		return true;
	
	return false;
}

void aimbot_t::apply( ) const {
	if ( m_best_target == nullptr || m_best_target->m_aim_point == nullptr )
		return;
	if ( m_best_target != m_last_target )
		m_best_target->m_shot_wanted = g.m_interfaces->globals( )->m_curtime;
	g.m_shoot_pos = m_best_target->m_aim_point->m_shoot_pos;
	auto look = g.m_shoot_pos.look( m_best_target->m_aim_point->m_point );
	//if ( m_best_target->m_shot_wanted + settings::rage::general::delay_shot > g.m_interfaces->globals( )->m_curtime )
	//	return;
	
	//if ( !check_hitchance( m_best_target, look, m_best_target->m_selected_record ) ) {
	//	return;
	//}
	draw_hitboxes( );
	g.m_cmd->m_tick_count = g.time_to_ticks(m_best_target->m_selected_record->m_pred_time + g.m_lerp);
	g.m_cmd->m_viewangles = look - g.m_local->aim_punch( ) * 2;
	g.m_cmd->m_buttons |= IN_ATTACK;
	g.m_shot = true;
	*g.m_packet = false;

	g_resolver.add_shot( m_best_target, m_best_target->m_damage, g.m_weapon_info->m_bullets, m_best_target->m_selected_record );
}

void aimbot_t::draw_hitboxes( player_t *player, bone_array_t * bones ) const {
	if ( !player )
		player = m_best_target->m_ent;
	auto *hdr = g.m_interfaces->model_info( )->get_studio_model( player->model( ) );
	auto *set = hdr->hitbox_set( player->hitbox_set( ) );

	if ( bones == nullptr )
		bones = m_best_target->m_selected_record->m_bones;
	
	vec3_t origin;
	ang_t angles;
	
	for ( auto i = 0; i < set->hitbox_count; i++ ) {
		auto *hitbox = set->hitbox( i );

		if ( !hitbox )
			continue;
		if ( hitbox->radius > 0 ) {
			matrix_t temp;

			math::AngleMatrix( hitbox->angle, &temp );
			matrix_t bone_transform;
			memcpy( &bone_transform, &bones[hitbox->bone], sizeof( matrix_t ) );
			if ( hitbox->angle != ang_t( ) )
				math::ConcatTransforms( bone_transform, temp, &bone_transform );

			vec3_t vMin, vMax;
			math::VectorTransform( hitbox->mins, bone_transform, vMin );
			math::VectorTransform( hitbox->maxs, bone_transform, vMax );

			g.m_interfaces->debug_overlay( )->AddCapsuleOverlay( vMin, vMax, hitbox->radius, 255, 0, 0, 255, 4 );
		} else {

			// convert rotation angle to a matrix.
			matrix_t rot_matrix;
			math::AngleMatrix( hitbox->angle, &rot_matrix );

			// apply the rotation to the entity input space (local).
			matrix_t matrix;
			math::ConcatTransforms( bones[ hitbox->bone ], rot_matrix, &matrix );

			// extract the compound rotation as an angle.
			ang_t bbox_angle;
			math::MatrixAngles( matrix, bbox_angle );

			// extract hitbox origin.
			auto origin1 = matrix.get_origin( );
			g.m_interfaces->debug_overlay( )->AddBoxOverlay( origin1, hitbox->mins, hitbox->maxs, bbox_angle, 255, 0, 0, 255, 4 );
		}
	}
}

bool aimbot_t::check_hitchance( ent_info_t *info, ang_t &view, std::shared_ptr<player_record_t> record, aim_point_t &point) {
	size_t     total_hits{ };
	const auto needed_hits{ static_cast< size_t >((settings::rage::selection::hitchance * 255 / 100.f )) };

	const auto inaccuracy = g.m_weapon->inaccuracy( );
	const auto spread = g.m_weapon->get_spread( );
	const auto start{ g.m_shoot_pos };
	vec3_t forward{}, right{}, up{};
	
	view.vectors( &forward, &right, &up );


	auto *studio_model = g.m_interfaces->model_info( )->get_studio_model( ( info )->m_ent->model( ) );
	if ( !studio_model )
		return false;

	auto *hitbox_set = studio_model->hitbox_set( info->m_ent->hitbox_set( ) );
	std::vector<RayTracer::Hitbox> hit_boxes;
	vec3_t mins{}, maxs{};
	
	for ( auto i = 0; i < hitbox_set->hitbox_count; i++ ) {
		if (i != point.m_hitbox)
			continue;
		auto *hitbox = hitbox_set->hitbox( i );

		if ( hitbox && hitbox->radius > 0 ) {
			matrix_t bone_transform;
			memcpy( &bone_transform, &record->m_bones[ hitbox->bone ], sizeof( matrix_t ) );
			if ( hitbox->angle != ang_t( ) ) {
				matrix_t temp;

				math::AngleMatrix( hitbox->angle, &temp );
				math::ConcatTransforms( bone_transform, temp, &bone_transform );
			}

			vec3_t vMin, vMax;
			math::VectorTransform( hitbox->mins, bone_transform, vMin );
			math::VectorTransform( hitbox->maxs, bone_transform, vMax );
			hit_boxes.emplace_back( vMin, vMax, hitbox->radius );
		}
	}
	if ( hit_boxes.empty( ) )
		return false;
	static auto surface_predicate = [ ]( const RayTracer::Hitbox &a, const RayTracer::Hitbox &b ) {
		const float area_1 = ( M_PI * powf( a.m_radius, 2 ) * a.m_len ) + ( 4.f / 3.f * M_PI * a.m_radius );
		const float area_2 = ( M_PI * powf( b.m_radius, 2 ) * b.m_len ) + ( 4.f / 3.f * M_PI * b.m_radius );

		return area_1 < area_2;
	};
	std::sort( hit_boxes.begin( ), hit_boxes.end( ), surface_predicate );
	//std::deque<hitbox_t> hit_boxes;
	//vec3_t mins{}, maxs{};
	//for ( auto i = 0; i < hitbox_set->hitbox_count; i++ ) {
	//	auto *hitbox = hitbox_set->hitbox( i );
	//
	//	if ( hitbox->radius > 0 ) {
	//		matrix_t temp;
	//
	//		math::AngleMatrix( hitbox->angle, &temp );
	//		matrix_t bone_transform;
	//		memcpy( &bone_transform, &record->m_bones[ hitbox->bone ], sizeof( matrix_t ) );
	//		if ( hitbox->angle != ang_t( ) )
	//			math::ConcatTransforms( bone_transform, temp, &bone_transform );
	//
	//		vec3_t vMin, vMax;
	//		math::VectorTransform( hitbox->mins, bone_transform, vMin );
	//		math::VectorTransform( hitbox->maxs, bone_transform, vMax );
	//		hit_boxes.emplace_front( vMin, vMax, hitbox->radius );
	//	} else {
	//		auto center = hitbox->maxs - hitbox->mins;
	//		mins = hitbox->mins; maxs = hitbox->maxs;
	//		math::VectorTransform( center, record->m_bones[ hitbox->bone ], center );
	//		hit_boxes.emplace_front( hitbox->bone, math::get_bone_position( record->m_bones[ hitbox->bone ] ), mins, maxs, hitbox->angle );
	//	}
	//}


	vec3_t origin;
	ang_t angles;
	for ( auto i = 0; i < 255; i++ ) {
		const auto weapon_spread = math::calculate_spread( g.m_weapon, i, inaccuracy, spread, false );

		auto dir = (forward + (right * weapon_spread.x) + (up * weapon_spread.y));
		dir /= dir.length( );
		const auto end = start + (dir * g.m_weapon_info->m_range);
		
		math::custom_ray ray( g.m_shoot_pos, end );

		RayTracer::Ray ray_1( ray.m_start, ray.m_end );
		for ( auto &box : hit_boxes ) {
			RayTracer::Trace trace;
			float m1, m2;
			auto dist = math::distSegmentToSegment( ray.m_start, ray.m_end, box.m_mins, box.m_maxs, m1, m2 );
			//RayTracer::TraceHitbox( ray_1, box, trace, RayTracer::Flags_NONE );
			if ( dist <= box.m_radius ) {
				total_hits++;
				break;
			}
			//RayTracer::TraceHitbox( ray_1, box, trace, RayTracer::Flags_NONE );
			//if ( trace.m_hit ) {
			//	total_hits++;
			//	break;
			//}
		}
		//for ( auto &box : hit_boxes ) {
		//	trace_t trace;
		//	if ( !box.m_box ) {
		//		if ( box.m_radius >= math::dist_Segment_to_Segment( ray, box.m_min, box.m_max ) ) {
		//			total_hits++;
		//			break;
		//		}
		//	}
		//	else {
		//
		//		matrix_t rot_matrix;
		//		math::AngleMatrix( box.m_rot, &rot_matrix );
		//
		//		// apply the rotation to the entity input space (local).
		//		matrix_t matrix;
		//		math::ConcatTransforms( record->m_bones[ box.m_bone ], rot_matrix, &matrix );
		//		
		//		if ( math::IntersectRayWithOBB( ray.m_start, ray.m_ray_dir, matrix, box.m_min, box.m_max, 0.0f, &trace ) ) {
		//			total_hits++;
		//			break;
		//		}
		//	}
		//}
		if ( total_hits >= needed_hits )
			return true;

		if ( ( (255 - i) + total_hits ) < needed_hits )
			return false;
	}
	return false;
}

std::shared_ptr<player_record_t> aimbot_t::last_record( ent_info_t *info ) {
	std::shared_ptr<player_record_t> best = nullptr;
	for ( auto &i : info->m_records ) {
		if ( !i || !i->m_setup || !i->valid( ) )
			continue;
		if ( i->m_dormant )
			break;
		best = i;
	}
	return best;
}

player_record_t *aimbot_t::best_record( ent_info_t *info ) {
	player_record_t *best = nullptr;
	for ( auto &i : info->m_records ) {
		if ( !i->m_setup || !i->valid( ))
			continue;
		if ( i->m_dormant )
			break;
		if ( i->m_shot || i->m_mode == resolver::RESOLVE_BODY || i->m_mode == resolver::RESOLVE_WALK)
			return i.get( );
		if( !best )
			best = i.get(  );
	}
	return best;
}

void aimbot_t::backup_players( const bool restore ) {
	for ( auto i{ 1 }; i <= g.m_interfaces->globals( )->m_max_clients; ++i ) {
		const auto& player_data = g_player_manager.m_ents[ i - 1 ];

		if ( !player_data.m_valid )
			continue;
		if ( !player_data.m_ent )
			continue;

		if ( restore )
			m_backup[ i - 1 ].restore( player_data.m_ent );
		else
			m_backup[ i - 1 ].store( player_data.m_ent );
	}
}

bool add_to_threads( std::vector<std::shared_ptr<hcThreadObject>> &objects, ent_info_t* info, int id ) {
	if ( !info->m_aim_point )
		return false;
	auto look = g.m_shoot_pos.look( info->m_aim_point->m_point );
	auto* studio_model = g.m_interfaces->model_info( )->get_studio_model( ( info )->m_ent->model( ) );
	if ( !studio_model )
		return false;
	auto* hitbox_set = studio_model->hitbox_set( info->m_ent->hitbox_set( ) );
	std::vector<RayTracer::Hitbox> hit_boxes;
	vec3_t mins{}, maxs{};

	for ( auto i = 0; i < hitbox_set->hitbox_count; i++ ) {
		if ( i != info->m_aim_point->m_hitbox )
			continue;
		auto* hitbox = hitbox_set->hitbox( i );

		if ( hitbox && hitbox->radius > 0 ) {
			matrix_t bone_transform;
			memcpy( &bone_transform, &info->m_selected_record->m_bones[ hitbox->bone ], sizeof( matrix_t ) );
			if ( hitbox->angle != ang_t( ) ) {
				matrix_t temp;

				math::AngleMatrix( hitbox->angle, &temp );
				math::ConcatTransforms( bone_transform, temp, &bone_transform );
			}

			vec3_t vMin, vMax;
			math::VectorTransform( hitbox->mins, bone_transform, vMin );
			math::VectorTransform( hitbox->maxs, bone_transform, vMax );
			hit_boxes.emplace_back( vMin, vMax, hitbox->radius );
		}
	}
	if ( hit_boxes.empty( ) )
		return false;
	//g_thread_handler.wait( ); // wait until all threads are finished
	objects.emplace_back( std::make_shared<hcThreadObject>(g.m_weapon, look, id, hit_boxes) );
	return true;
}

class mpThreadObject {
public:
	bool ret_state = false;
	bool finished = false;
	bool read = false;
	ent_info_t* info = nullptr;
	vec3_t eye;
	void run( );
	mpThreadObject( ent_info_t* info_, vec3_t eye ) : eye( eye ) {
		info = info_;
	}
};

void mpThreadObject::run ( ) {
	auto* studio_model = g.m_interfaces->model_info( )->get_studio_model( info->m_ent->model( ) );
	if ( !studio_model ) {
		ret_state = false;
		finished = true;
		return;
	}
	info->m_selected_record = g_resolver.FindIdealRecord( info );
	if ( info->m_selected_record ) {
		aimbot_t::get_points( info, studio_model );
		if ( /*best_target( info ) && */g_aimbot.get_best_point( info, nullptr, eye ) ) {
			//m_best_target = info;
			std::unique_lock<std::mutex> lock(g_thread_handler.queue_mutex2);
			ret_state = true;
			finished = true;
			return;
			//valid_targets.push_back( info );
			//if ( info->m_damage >= info->m_ent->health( ) )
			//	break;
			//continue;
		}
	}
	info->m_selected_record = aimbot_t::last_record( info );
	if ( info->m_selected_record ) {
		aimbot_t::get_points( info, studio_model );
		if ( /*best_target( info ) && */g_aimbot.get_best_point( info, nullptr, eye ) ) {
			//m_best_target = info;
			std::unique_lock<std::mutex> lock( g_thread_handler.queue_mutex2 );
			ret_state = true;
			finished = true;
			return;
			//valid_targets.push_back( info );
			//if ( info->m_damage >= info->m_ent->health( ) )
			//	break;
		}
	}
	std::unique_lock<std::mutex> lock( g_thread_handler.queue_mutex2 );
	ret_state = false;
	finished = true;
}

std::vector<ent_info_t*> aimbot_t::mp_threading( ) const {
	std::vector<ent_info_t*> targets = {};
	std::vector< std::shared_ptr< mpThreadObject > > objects = {};

	g_thread_handler.start( );
	ang_t look;
	auto bones = static_cast< bone_array_t* >( g.m_interfaces->mem_alloc( )->alloc( sizeof( bone_array_t ) * 128 ) );
	vec3_t eye;
	bone_array_t* backup_cache;
	for ( auto& info : m_targets ) {
		look = g.m_shoot_pos.look( info->m_ent->world_space_center( ) );
		if ( get_aim_matrix( look - g.m_local->aim_punch( ) * 2, bones ) ) {
			auto bone_cache = &g.m_local->bone_cache( );
			if ( bone_cache ) {
				backup_cache = bone_cache->m_pCachedBones;
				int backup_bone_count = bone_cache->m_CachedBoneCount;
				bone_cache->m_pCachedBones = bones;
				g.m_local->get_eye_pos( &eye );
				bone_cache->m_pCachedBones = backup_cache;
				bone_cache->m_CachedBoneCount = backup_bone_count;
			}
		}
		auto ptr = objects.emplace_back( std::make_shared<mpThreadObject>( info, eye ) );
		g_thread_handler.QueueJob( [ ptr ] { 
			ptr->run( );
			} );
	}
	g.m_interfaces->mem_alloc( )->free( bones );

	//while ( g_thread_handler.busy( ) )
	//	continue;
	while ( g_thread_handler.busy( ) );

	g_thread_handler.stop( );

	//for ( auto& i : g_thread_handler.objects )
	//	i->run( );

	{
		bool all_finished = true;
		while ( !all_finished ) {
			all_finished = true;
			for ( auto &i : objects )
				if ( !i->finished )
					all_finished = false;
		}
		for ( const auto& object : objects ) {
			if ( object->ret_state )
				targets.push_back( object->info );
		}
	}
	return targets;
}


void aimbot_t::on_tick ( ) {
	m_list_eye_pos.clear();
	g.m_cmd->m_viewangles -= g.m_local->aim_punch( ) * 2;
	if ( !settings::rage::general::enabled )
		return;


	if ( !g.m_weapon_info )
		return;

	if ( g.m_lag == 0 )
		return;

	if ( !g.m_can_fire )
		return;



	if ( !settings::rage::general::key )
		return;

	if ( !settings::rage::general::auto_shoot && !( g.m_cmd->m_buttons & IN_ATTACK ) )
		return;



	backup_players( false ); // backup player vars before we change them
	
	m_last_target = m_best_target; 
	m_best_target = nullptr;
	
	get_targets( ); // get all possible targets



	std::vector<ent_info_t*> valid_targets = mp_threading( );

	if ( valid_targets.size() > 0 )
		g_movement.set_should_stop( true );

	g_thread_handler.start( );

	std::vector<std::shared_ptr<hcThreadObject>> objects = {};
	for ( auto i = 0; i < valid_targets.size( ); i++ ) {
		if ( add_to_threads( objects, valid_targets[ i ], i ) ) // this is a very jank thread hander, will update soon
		{
			auto ptr = objects.back( );
			g_thread_handler.QueueJob( [ ptr ] {
				ptr->run( ); 
				} );
		}
	}
	
	while ( g_thread_handler.busy( ) );

	g_thread_handler.stop( );

	//for ( auto &i : g_thread_handler.objects )
	//	i->run( );

	{
		bool all_finished = true;
		while ( !all_finished ) {
			all_finished = true;
			for ( auto &i : objects )
				if ( !i->finished )
					all_finished = false;
		}

		for ( const auto& object : objects ) {
			if ( object->ret_state ) // did the player pass?
				if ( best_target( valid_targets[ object->id ] ) ) // only set if the player is the best overall target
					m_best_target = valid_targets[ object->id ];
		}

	}

	apply( ); // look and shoot at the best point

	backup_players( true ); //restore
}
