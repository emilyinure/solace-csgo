#pragma once
#include "netvars.h"
#include "datamap.h"

class offsets_t {
public:
	void load_buffer( char* offset_buffer ) { }

	explicit offsets_t( char* offset_buffer = nullptr ) {
	#ifdef _DEBUG || 1
		// temporarily getting offsets dynamically
		// still use this when debugging -- e
	#else
		if ( offset_buffer )
			load_buffer( offset_buffer );
	#endif
	}

	struct {

	} m_sigs;


	struct {

	} m_entity;
	struct {
        uint32_t m_last_bone_setup_time = *(uint32_t*)(util::find("client.dll", "80 3D ? ? ? ? ? 74 16 A1 ? ? ? ? 48 C7 81") + 0x11);
		NETVAR( "DT_CSPlayer", "m_flSimulationTime", m_simulation_time )
		NETVAR( "DT_BasePlayer", "m_vecViewOffset[0]", m_view_offset )
		NETVAR( "DT_CSPlayer", "m_iTeamNum", m_team )
		NETVAR( "DT_BaseEntity", "m_bSpotted", m_spotted )
		NETVAR( "DT_BasePlayer", "m_nSurvivalTeam", m_survival_team )
		NETVAR( "DT_BasePlayer", "m_flHealthShotBoostExpirationTime", m_health_boost_time )
		NETVAR( "DT_CSPlayer", "m_vecOrigin", m_origin )
		NETVAR( "DT_BasePlayer", "m_hViewModel[0]", m_view_model )
		NETVAR( "DT_CSPlayer", "m_bHasDefuser", m_defuser )
		NETVAR( "DT_CSPlayer", "m_bGunGameImmunity", m_gun_game_immunity )
		NETVAR( "DT_CSPlayer", "m_iShotsFired", m_shots_fired )
		NETVAR( "DT_CSPlayer", "m_angEyeAngles", m_eye_angles )
		NETVAR( "DT_CSPlayer", "m_ArmorValue", m_armor )
		NETVAR( "DT_CSPlayer", "m_bHasHelmet", m_has_helmet )
		NETVAR( "DT_CSPlayer", "m_bIsScoped", m_is_scoped )
		NETVAR( "DT_CSPlayer", "m_bIsDefusing", m_defusing )
		NETVAR( "DT_CSPlayer", "m_iAccount", m_money )
		NETVAR( "DT_CSPlayer", "m_flLowerBodyYawTarget", m_lower_body_yaw )
		NETVAR( "DT_CSPlayer", "m_flVelocityModifier", velocity_modifier )
		NETVAR( "DT_CSPlayer", "m_flNextAttack", m_next_attack )
		NETVAR( "DT_CSPlayer", "m_flFlashDuration", m_flash_duration )
		NETVAR( "DT_CSPlayer", "m_flFlashMaxAlpha", m_flash_alpha )
		NETVAR( "DT_CSPlayer", "m_bHasNightVision", m_has_night_vision )
		NETVAR( "DT_CSPlayer", "m_bNightVisionOn", m_night_vision_enabled )
		NETVAR( "DT_CSPlayer", "m_iHealth", m_health )
		NETVAR( "C_BasePlayer", "m_lifeState", m_life_state )
		NETVAR( "DT_CSPlayer", "m_fFlags", m_flags )
		NETVAR( "DT_BasePlayer", "m_viewPunchAngle", m_punch_angle )

		NETVAR( "DT_BaseEntity", "m_vecNetworkOrigin", m_network_origin )

		NETVAR( "DT_BaseEntity", "m_vecMins", m_mins )
		NETVAR( "DT_BaseEntity", "m_vecMaxs", m_maxs )
		NETVAR( "DT_BasePlayer", "m_aimPunchAngle", m_aim_punch_angle )
		NETVAR( "DT_BasePlayer", "m_vecBaseVelocity", m_vecBaseVelocity )
		NETVAR( "DT_BasePlayer", "m_aimPunchAngleVel", m_aim_punch_angle_vel )
		NETVAR( "DT_CSPlayer", "m_flFallVelocity", m_fall_velocity )
		NETVAR( "DT_CSPlayer", "m_flStepSize", m_step_size )
		NETVAR( "DT_CSPlayer", "m_nDuckTimeMsecs", m_nDuckTimeMsecs )
		NETVAR( "DT_CSPlayer", "m_nDuckJumpTimeMsecs", m_nDuckJumpTimeMsecs )
		NETVAR( "DT_CSPlayer", "m_nJumpTimeMsecs", m_nJumpTimeMsecs )
		NETVAR( "DT_BasePlayer", "m_vecVelocity[0]", m_velocity )
		NETVAR( "C_BasePlayer", "m_flMaxspeed", m_max_speed )
		NETVAR( "DT_CSPlayer", "m_flStamina ", m_stamina )
		NETVAR( "DT_BaseEntity", "m_flShadowCastDistance", m_fov_time )
		NETVAR( "DT_BasePlayer", "m_hObserverTarget", m_observer_target )
		NETVAR( "DT_BasePlayer", "m_nHitboxSet", m_hitbox_set )
		NETVAR( "C_BasePlayer", "m_hGroundEntity", m_ground_ent )
		NETVAR("DT_CSPlayer", "m_flDuckAmount", m_duck_amount)
		NETVAR("DT_CSPlayer", "m_flDuckSpeed", m_duck_speed)
		NETVAR( "DT_CSPlayer", "m_bDucking", m_ducking )
		NETVAR( "DT_CSPlayer", "m_bHasHeavyArmor", m_heavy_armor )
		NETVAR( "DT_SmokeGrenadeProjectile", "m_nSmokeEffectTickBegin", m_smoke_grenade_tick_begin )
		NETVAR( "C_BasePlayer", "m_nTickBase", m_tick_base )
		NETVAR( "DT_BaseEntity", "m_hOwnerEntity", m_hOwnerEntity )
		NETVAR( "DT_CSPlayer", "m_flPoseParameter", m_pose_parameters )
		NETVAR( "DT_CSPlayer", "m_flEncodedController", m_encoded_controller )
		NETVAR( "DT_CSPlayer", "m_bKilledByTaser", m_killed_by_taser )
		NETVAR( "DT_CSPlayer", "m_hActiveWeapon", m_active_weapon )
		NETVAR("DT_Inferno", "m_bFireIsBurning", m_fire_burning)
		NETVAR("DT_Inferno", "m_fireXDelta", m_fire_x)
		NETVAR("DT_Inferno", "m_fireYDelta", m_fire_y)
		NETVAR("DT_Inferno", "m_fireZDelta", m_fire_z)
		NETVAR("DT_Inferno", "m_fireCount", m_fire_count)

		NETVAR( "DT_BaseAnimating", "m_bClientSideAnimation", m_client_side_animation )
		NETVAR( "DT_CSPlayer", "m_nSequence", m_nSequence )
		NETVAR( "DT_CSPlayer", "m_flCycle", m_flCycle )
		NETVAR( "DT_BasePlayer", "m_MoveType", m_move_type );
		NETVAR( "C_BaseEntity", "m_angAbsRotation", m_angAbsRotation );
		NETVAR( "C_BaseEntity", "m_angRotation", m_angRotation );
		NETVAR( "C_BaseEntity", "m_angNetworkAngles", m_angNetworkAngles );
		NETVAR( "C_BasePlayer", "m_surfaceFriction", m_surfaceFriction );
	} m_player;
	struct {
		NETVAR( "DT_BaseCombatWeapon", "m_flNextPrimaryAttack", next_primary_attack )
		NETVAR( "DT_BaseCombatWeapon", "m_flNextSecondaryAttack", next_secondary_attack )
		NETVAR( "DT_BaseCSGrenade", "m_flThrowStrength", m_throw_strength )
		NETVAR( "DT_BaseAttributableItem", "m_iItemDefinitionIndex", item_definition_index )
		NETVAR( "DT_BaseCombatWeapon", "m_iClip1", clip1_count )
		NETVAR( "DT_BaseCombatWeapon", "m_iClip2", clip2_count )
		NETVAR( "DT_BaseCombatWeapon", "m_iPrimaryReserveAmmoCount", primary_reserve_ammo_acount )
		NETVAR( "DT_WeaponCSBase", "m_flRecoilIndex", recoil_index )
		NETVAR( "DT_WeaponCSBase", "m_fLastShotTime", last_shot_time )
		NETVAR( "DT_SmokeGrenadeProjectile", "m_nSmokeEffectTickBegin", smoke_effect_begin_tick )
		NETVAR( "DT_WeaponCSBase", "m_flPostponeFireReadyTime", m_flPostponeFireReadyTime )
		NETVAR( "DT_WeaponCSBaseGun", "m_zoomLevel", zoom_level )
		NETVAR( "DT_BaseCombatWeapon", "m_iEntityQuality", entity_quality )
		NETVAR( "DT_BaseCombatWeapon", "m_hWeaponWorldModel", weapon_model )
		NETVAR( "DT_BaseCSGrenade", "m_bPinPulled", m_pin_pulled )
		NETVAR( "DT_BaseCSGrenade", "m_fThrowTime", m_throw_time )
	} m_weapon;
};
