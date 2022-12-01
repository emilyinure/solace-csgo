#pragma once
class anim_state;
class c_client_class;
#include "weapon_info.h"


class networkable_t {
public:
	VFUNC( client_class( ), 2, c_client_class *( __thiscall * )( void * ) );
	VFUNC(dormant(), 9, bool( __thiscall * )( void * ) );
};


enum player_flags {
	fl_onground = ( 1 << 0 ), 	
	fl_ducking = ( 1 << 1 ),	
	fl_waterjump = ( 1 << 2 ),	
	fl_ontrain = ( 1 << 3 ),	
	fl_inrain = ( 1 << 4 ),		
	fl_frozen = ( 1 << 5 ),		
	fl_atcontrols = ( 1 << 6 ),	
	fl_client = ( 1 << 7 ),		
	fl_fakeclient = ( 1 << 8 ),	
	fl_inwater = ( 1 << 10 ),	
};
enum item_definition_indexes {
	WEAPON_NONE = 0,
	WEAPON_DEAGLE,
	WEAPON_ELITE,
	WEAPON_FIVESEVEN,
	WEAPON_GLOCK,
	WEAPON_AK47 = 7,
	WEAPON_AUG,
	WEAPON_AWP,
	WEAPON_FAMAS,
	WEAPON_G3SG1,
	WEAPON_GALILAR = 13,
	WEAPON_M249,
	WEAPON_M4A1 = 16,
	WEAPON_MAC10,
	WEAPON_P90 = 19,
	WEAPON_MP5SD = 23,
	WEAPON_UMP45,
	WEAPON_XM1014,
	WEAPON_BIZON,
	WEAPON_MAG7,
	WEAPON_NEGEV,
	WEAPON_SAWEDOFF,
	WEAPON_TEC9,
	WEAPON_TASER,
	WEAPON_HKP2000,
	WEAPON_MP7,
	WEAPON_MP9,
	WEAPON_NOVA,
	WEAPON_P250,
	WEAPON_SHIELD,
	WEAPON_SCAR20,
	WEAPON_SG556,
	WEAPON_SSG08,
	WEAPON_KNIFEGG,
	WEAPON_KNIFE,
	WEAPON_FLASHBANG,
	WEAPON_HEGRENADE,
	WEAPON_SMOKEGRENADE,
	WEAPON_MOLOTOV,
	WEAPON_DECOY,
	WEAPON_INCGRENADE,
	WEAPON_C4,
	WEAPON_HEALTHSHOT = 57,
	WEAPON_KNIFE_T = 59,
	WEAPON_M4A1_SILENCER,
	WEAPON_USP_SILENCER,
	WEAPON_CZ75A = 63,
	WEAPON_REVOLVER,
	WEAPON_TAGRENADE = 68,
	WEAPON_FISTS,
	WEAPON_BREACHCHARGE,
	WEAPON_TABLET = 72,
	WEAPON_MELEE = 74,
	WEAPON_AXE,
	WEAPON_HAMMER,
	WEAPON_SPANNER = 78,
	WEAPON_KNIFE_GHOST = 80,
	WEAPON_FIREBOMB,
	WEAPON_DIVERSION,
	WEAPON_FRAG_GRENADE,
	WEAPON_SNOWBALL,
	WEAPON_BUMPMINE,
	WEAPON_BAYONET = 500,
	WEAPON_KNIFE_CSS = 503,
	WEAPON_KNIFE_FLIP = 505,
	WEAPON_KNIFE_GUT,
	WEAPON_KNIFE_KARAMBIT,
	WEAPON_KNIFE_M9_BAYONET,
	WEAPON_KNIFE_TACTICAL,
	WEAPON_KNIFE_FALCHION = 512,
	WEAPON_KNIFE_SURVIVAL_BOWIE = 514,
	WEAPON_KNIFE_BUTTERFLY,
	WEAPON_KNIFE_PUSH,
	WEAPON_KNIFE_URSUS = 519,
	WEAPON_KNIFE_GYPSY_JACKKNIFE,
	WEAPON_KNIFE_STILETTO = 522,
	WEAPON_KNIFE_WIDOWMAKER,
	GLOVE_STUDDED_BLOODHOUND = 5027,
	GLOVE_T_SIDE = 5028,
	GLOVE_CT_SIDE = 5029,
	GLOVE_SPORTY = 5030,
	GLOVE_SLICK = 5031,
	GLOVE_LEATHER_WRAP = 5032,
	GLOVE_MOTORCYCLE = 5033,
	GLOVE_SPECIALIST = 5034,
	GLOVE_HYDRA = 5035
};

class CStudioHdr;
class entity_t;
class animation_layer_t {
public:
	float   m_anim_time;			// 0x0000
	float   m_fade_out_time;		// 0x0004
	CStudioHdr *m_pDispatchedStudioHdr;				// 0x0008
	int     m_nDispatchedSrc;				// 0x000C
	int     m_nDispatchedDst;				// 0x0010
	int     m_order;				// 0x0014
	int     m_sequence;				// 0x0018
	float   m_prev_cycle;			// 0x001C
	float   m_weight;				// 0x0020
	float   m_weight_delta_rate;	// 0x0024
	float   m_playback_rate;		// 0x0028
	float   m_cycle;				// 0x002C
	entity_t *m_owner;				// 0x0030
	int     m_bits;					// 0x0034
}; // size: 0x0038

class bone_accessor_t {
public:
	void *m_pAnimating;
	bone_array_t *m_pBones;
	int        m_ReadableBones;
	int        m_WritableBones;
};

class bone_cache_t {
public:
	bone_array_t *m_pCachedBones;
	char		u4[ 0x8 ];
	int        m_CachedBoneCount;
};

class CBoneCacheHandler {
public:
	float			m_timeValid;
	int				m_boneMask;
	unsigned int	m_size;
	unsigned short	m_cachedBoneCount;
	unsigned short	m_matrixOffset;
	unsigned short	m_cachedToStudioOffset;
	unsigned short	m_boneOutOffset;
};

struct model_t;

class animating_t {
public:
	VFUNC( setup_bones( matrix_t *out, int max_bones, int mask, float time ), 13, bool( __thiscall * )( void *, matrix_t *, int, int, float ), out, max_bones, mask, time )
};

struct RenderableInstance_t {
	uint8_t m_alpha;
	__forceinline RenderableInstance_t( ) : m_alpha{ 255ui8 } { }
};
enum class PoseParam_t : int {
	STRAFE_YAW = 0,
	STAND,
	LEAN_YAW,
	SPEED,
	LADDER_YAW,
	LADDER_SPEED,
	JUMP_FALL,
	MOVE_YAW,
	MOVE_BLEND_CROUCH,
	MOVE_BLEND_WALK,
	MOVE_BLEND_RUN,
	BODY_YAW,
	BODY_PITCH,
	AIM_BLEND_STAND_IDLE,
	AIM_BLEND_STAND_WALK,
	AIM_BLEND_STAND_RUN,
	AIM_BLEND_COURCH_IDLE,
	AIM_BLEND_CROUCH_WALK,
	DEATH_YAW
}; 
enum animstate_layer_t {
	ANIMATION_LAYER_AIMMATRIX = 0,
	ANIMATION_LAYER_WEAPON_ACTION,
	ANIMATION_LAYER_WEAPON_ACTION_RECROUCH,
	ANIMATION_LAYER_ADJUST,
	ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL,
	ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB,
	ANIMATION_LAYER_MOVEMENT_MOVE,
	ANIMATION_LAYER_MOVEMENT_STRAFECHANGE,
	ANIMATION_LAYER_WHOLE_BODY,
	ANIMATION_LAYER_FLASHED,
	ANIMATION_LAYER_FLINCH,
	ANIMATION_LAYER_ALIVELOOP,
	ANIMATION_LAYER_LEAN,
	ANIMATION_LAYER_COUNT,
};
enum MoveType_t {
	MOVETYPE_NONE = 0,	// never moves
	MOVETYPE_ISOMETRIC,			// For players -- in TF2 commander view, etc.
	MOVETYPE_WALK,				// Player only - moving on the ground
	MOVETYPE_STEP,				// gravity, special edge handling -- monsters use this
	MOVETYPE_FLY,				// No gravity, but still collides with stuff
	MOVETYPE_FLYGRAVITY,		// flies through the air + is affected by gravity
	MOVETYPE_VPHYSICS,			// uses VPHYSICS for simulation
	MOVETYPE_PUSH,				// no clip to world, push and crush
	MOVETYPE_NOCLIP,			// No gravity, no collisions, still do velocity/avelocity
	MOVETYPE_LADDER,			// Used by players only when going onto a ladder
	MOVETYPE_OBSERVER,			// Observer movement, depends on player's observer mode
	MOVETYPE_CUSTOM,			// Allows the entity to describe its own physics

	// should always be defined as the last item in the list
	MOVETYPE_LAST = MOVETYPE_CUSTOM,

	MOVETYPE_MAX_BITS = 4
};
class entity_t {
public:
	template< typename t >
	__forceinline t &get( size_t offset ) {
		return *reinterpret_cast< t * >(reinterpret_cast< uintptr_t >(this) + offset);
	}
	animating_t *animating( ) {
		return reinterpret_cast< animating_t * >( uintptr_t( this ) + 0x4 );
	}
	networkable_t *networkable( ) {
		return reinterpret_cast< networkable_t * >( uintptr_t( this ) + 0x8 );
	}

	int index( ) {
		using original_fn = int( __thiscall * )( void * );
		return ( *( original_fn ** )networkable( ) )[ 10 ]( networkable( ) );
	}
	
	bone_cache_t &bone_cache( ) {
		return get< bone_cache_t>( 0x2900 );
	}

	vec3_t &abs_origin( ) {
		using original_fn = vec3_t & ( __thiscall * )( void * );
		return ( *( original_fn ** )this )[ 10 ]( this );;
	}
	ang_t &abs_angles( ) {
		using original_fn = ang_t & ( __thiscall * )( void * );
		return ( *( original_fn ** )this )[ 11 ]( this );;
	}

	void *bone_merge_cache() {
		return reinterpret_cast< void * >( reinterpret_cast< uintptr_t >( this ) + 0x28FC );
	}

	model_t *model( ) {
		using original_fn = model_t * ( __thiscall * )( void * );
		return ( *( original_fn ** )animating( ) )[ 8 ]( animating( ) );
	}
	void set_abs_angles( ang_t ang ) {
		static auto func = util::find( "client.dll", "55 8B EC 83 E4 F8 83 EC 64 53 56 57 8B F1 E8" );
		using SetAbsAngles_t = void( __thiscall * )( decltype( this ), const ang_t & );
		reinterpret_cast< SetAbsAngles_t >(func)( this, ang );
	}

	void set_abs_origin( vec3_t org ) {
		static auto func = util::find( "client.dll", "55 8B EC 83 E4 F8 51 53 56 57 8B F1" );
		using SetAbsAngles_t = void( __thiscall * )( decltype( this ), const vec3_t & );
		reinterpret_cast< SetAbsAngles_t >(func)( this, org );
	}
	
	void set_abs_velocity( vec3_t org ) {
		static auto func = util::find( "client.dll", "55 8B EC 83 E4 F8 83 EC 0C 53 56 57 8B 7D 08 8B F1" );
		using SetAbsAngles_t = void( __thiscall * )( decltype( this ), const vec3_t & );
		reinterpret_cast< SetAbsAngles_t >( func )( this, org );
	}


	void PreThink( ) {
		using think_t = void( __thiscall * )( void * );
		return util::get_virtual_function< think_t >( this, 307 )( this );
	}
	void Think( ) {
		using think_t = void( __thiscall * )( void * );
		return util::get_virtual_function< think_t >( this, 137 )( this );
	}
	void PostThink( ) {
		using think_t = void( __thiscall * )( void * );
		return util::get_virtual_function< think_t >( this, 308 )( this );
	}

	void StandardBlendingRules( void *hdr, vec3_t *pos, quaternion_t *q, float time, int mask ) {
		using StandardBlendingRules_t = void( __thiscall * )( void *, void *, vec3_t *, quaternion_t *, float, int );
		return util::get_virtual_function< StandardBlendingRules_t >( this, 205 )( this, hdr, pos, q, time, mask );
	}
	void BuildTransformations( void *hdr, vec3_t pos[], quaternion_t q[], const matrix_t &transform, int mask, uint8_t *computed ) {
		using BuildTransformations_t = void( __thiscall * )( void *, void *, vec3_t *, quaternion_t *, matrix_t const &, int, uint8_t * );
		return util::get_virtual_function< BuildTransformations_t >( this, 184 )( this, hdr, pos, q, transform, mask, computed );
	}

	void DoExtraBoneProcessing( void *hdr, vec3_t pos[], quaternion_t q[], matrix_t *bones, uint8_t *computed, CIKContext *ik ) {
		using BuildTransformations_t = void( __thiscall * )( void *, void *, vec3_t *, quaternion_t *, matrix_t *, uint8_t *, CIKContext * );
		return util::get_virtual_function< BuildTransformations_t >( this, 197 )( this, hdr, pos, q, bones, computed, ik );
	}

	void UpdateIKLocks( float time ) {
		using BuildTransformations_t = void( __thiscall * )( void *, float );
		return util::get_virtual_function< BuildTransformations_t >( this, 186 )( this, time );
	}

	void CalculateIKLocks( float time ) {
		using BuildTransformations_t = void( __thiscall * )( void *, float );
		return util::get_virtual_function< BuildTransformations_t >( this, 187 )( this, time );
	}

	inline void Wrap_UpdateTargets( uintptr_t ik, vec3_t *pos, quaternion_t *q, matrix_t *bones, uint8_t *computed ) const {
		using CreateAnimState_t = void( __thiscall * )( uintptr_t, vec3_t *, quaternion_t *, matrix_t *, uint8_t * );

		static auto func = util::find( "client.dll", "55 8B EC 83 E4 ? 81 EC ? ? ? ? 33 D2" );
		if ( !func )
			return;

		( ( CreateAnimState_t )func )( ik, pos, q, bones, computed );
	}

	inline void Wrap_SolveDependencies( uintptr_t ik, vec3_t *pos, quaternion_t *q, matrix_t *bones, uint8_t *computed ) const {
		using CreateAnimState_t = void( __thiscall * )( uintptr_t, vec3_t *, quaternion_t *, matrix_t *, uint8_t * );

		static auto func = util::find( "client.dll", "55 8B EC 83 E4 ? 81 EC ? ? ? ? 8B 81 ? ? ? ? 56" );
		if ( !func )
			return;

		( ( CreateAnimState_t )func )( ik, pos, q, bones, computed );
	}

	OFFSET( bool, client_side_anim, g.m_offsets->m_player.m_client_side_animation);
	
	void update_client_side_animation( ) {
		using update_client_side_animation_t = void( __thiscall * )( decltype( this ) );
		return util::get_virtual_function< update_client_side_animation_t >( this, 218 )( this );
	}

	enum InvalidatePhysicsBits_t : int {
		POSITION_CHANGED = 0x1,
		ANGLES_CHANGED = 0x2,
		VELOCITY_CHANGED = 0x4,
		ANIMATION_CHANGED = 0x8,
		BOUNDS_CHANGED = 0x10,
		SEQUENCE_CHANGED = 0x20
	};

	__forceinline void InvalidatePhysicsRecursive( InvalidatePhysicsBits_t bits ) {
		using InvalidatePhysicsRecursive_t = void( __thiscall* )( decltype( this ), InvalidatePhysicsBits_t );
		static auto addr = util::find( "client.dll", "55 8B EC 83 E4 F8 83 EC 0C 53 8B 5D 08 8B C3 56" );
		(( InvalidatePhysicsRecursive_t ) addr )( this, bits );
	}

	float spawn_time( ) {
		static auto SpawnTime = *( uintptr_t * )( util::find( "client.dll", "F3 0F 5C 88 ? ? ? ? 0F" ) + 4 );
		return *(float*)( this + SpawnTime );
	}
	__forceinline animation_layer_t *&anim_overlay(  ) {
		static auto anim_overlay = *( uintptr_t * )( util::find( "client.dll", "8B 80 ? ? ? ? 03 C1 74 ?") + 2 );
		return *reinterpret_cast< animation_layer_t ** >(this + anim_overlay);
	}
	__forceinline void GetAnimLayers( animation_layer_t *layers ) {
		std::memcpy( layers, anim_overlay( ), sizeof( animation_layer_t ) * 15 );
	}

	OFFSETPTR( float, pose_parameters, g.m_offsets->m_player.m_pose_parameters );
	__forceinline void GetPoseParameters( float *poses ) {
		std::memcpy( poses, pose_parameters( ), sizeof( float ) * 24 );
	}
	__forceinline void SetPoseParameters( float *poses ) {
		std::memcpy( pose_parameters( ), poses, sizeof( float ) * 24 );
	}
	__forceinline void SetAnimLayers( animation_layer_t *layers ) {
		std::memcpy( anim_overlay( ), layers, sizeof( animation_layer_t ) * 15 );
	}
	void handle_taser( ) {
		using handle_taser_t = void( __thiscall * )( void * );
		static auto handle_taser = reinterpret_cast< handle_taser_t >(util::find( "client.dll",
		                                                                          "55 8B EC 83 EC ? 56 8B F1 80 BE ? ? ? ? ? 0F 84 ? ? ? ? 80 BE ? ? ? ? ? 0F 84 ? ? ? ? A1 ? ? ? ?" ) );
		if ( killed_by_taser( ) )
			handle_taser( this );
	}
	bool on_team( entity_t *target ) {
		return target->team( ) == team( );
	}
	OFFSET( int, team, g.m_offsets->m_player.m_team );
	OFFSET( bool, killed_by_taser, g.m_offsets->m_player.m_killed_by_taser );
	OFFSET( int, iEFlags, 0xE8 );
	OFFSETPTR( float, encoder_controller, g.m_offsets->m_player.m_encoded_controller )
	OFFSET( int, move_type, g.m_offsets->m_player.m_move_type )
	OFFSET( float, surface_friction, g.m_offsets->m_player.m_surfaceFriction )
	OFFSET( int, sequence, g.m_offsets->m_player.m_nSequence )
		OFFSET( float, cycle, g.m_offsets->m_player.m_flCycle )
		OFFSET( vec3_t, mins, g.m_offsets->m_player.m_mins )
		OFFSET( vec3_t, maxs, g.m_offsets->m_player.m_maxs )
	__forceinline ang_t &m_angAbsRotation( ) {
		return get< ang_t >( g.m_offsets->m_player.m_angAbsRotation );
	}

	
	__forceinline ang_t &m_angRotation( ) {
		return get< ang_t >( g.m_offsets->m_player.m_angRotation );
	}

	__forceinline ang_t &m_angNetworkAngles( ) {
		return get< ang_t >( g.m_offsets->m_player.m_angNetworkAngles );
	}

	bool  UpdateDispatchLayer( animation_layer_t *pLayer, CStudioHdr *studio_hdr, int sequence ) {
		///static auto func = util::find( "client.dll", "55 8B EC 81 EC ? ? ? ? 56 57 8B 7D ? 8B D1" );
		using SetAbsAngles_t = bool( __thiscall * )( void *, animation_layer_t *, CStudioHdr *, int );
		///return reinterpret_cast< SetAbsAngles_t >(func)( animating(), pLayer, studio_hdr, sequence );

		return util::get_virtual_function< SetAbsAngles_t>( this, 241 )( this, pLayer, studio_hdr, sequence );
		if ( !studio_hdr || !pLayer ) {
			if ( pLayer )
				pLayer->m_nDispatchedDst = -1;
			return false;
		}
		auto v4 = *( DWORD * )( studio_hdr + 4 );
		auto v5 = ( v4 != 0) ? *( int * )( v4 + 20 ) : *( int * )( *( int * )studio_hdr + 0xBC );
		if ( pLayer->m_pDispatchedStudioHdr != studio_hdr || pLayer->m_nDispatchedSrc != sequence ||
			 pLayer->m_nDispatchedDst >= v5 ) {
			pLayer->m_pDispatchedStudioHdr = studio_hdr;
			pLayer->m_nDispatchedSrc = sequence;
			using GetSequenceName_t = const char *( __thiscall * )( void *, int );
			static auto GetSequenceName = (GetSequenceName_t)( util::find( "client.dll",
																				 "55 8B EC 83 7D ? ? 56 8B F1 75 ? B8 ? ? ? ?" ) );

			auto pszSeqName = GetSequenceName( this, sequence );

			using LookupSequence_t = int( __thiscall * )( void *, const char * );
			static auto LookupSequence = reinterpret_cast< LookupSequence_t >( util::find( "client.dll",
																			   "55 8B EC 83 EC ? 53 8B 5D ? 56 57 8B F9 85 DB" ) );

			pLayer->m_nDispatchedDst = LookupSequence( studio_hdr, pszSeqName );
		}
		return pLayer->m_nDispatchedDst != -1;
	}

	vec3_t GetSequenceLinearMotion( CStudioHdr* pstudiohdr, int iSequence, const float *poseParameter) {
		static auto studioHdr = (util::find("client.dll", "55 8B EC 83 EC ? 56 8B F1 57 8B FA 85 F6 75 ? 68 ? ? ? ?"));
		vec3_t out;
		_asm{
			
			mov ecx, pstudiohdr
			mov edx, iSequence
			lea eax, [out]
			push eax
			push poseParameter
			call studioHdr
			add     esp, 8
		}
		//studioHdr( pstudiohdr, iSequence, poseParameter, &out);
		return out;
	}

	CStudioHdr *m_studioHdr( ) {
		static auto studioHdr = *( uintptr_t * )( util::find( "client.dll", "8B 86 ? ? ? ? 89 44 24 10 85 C0" ) + 2 );
		return *reinterpret_cast< CStudioHdr ** >( this + 0x293C );// studioHdr);
	}

	CIKContext *&m_Ipk( ) {
		return *( CIKContext ** )( (uintptr_t)this + 0x266C );
	}
	
	CStudioHdr *GetModelPtr( ) {
		using LockStudioHdr_t = int( __thiscall * )( void * );
		static auto LockStudioHdr = ( LockStudioHdr_t )util::find( "client.dll", "55 8B EC 51 53 8B D9 56 57 8D B3" );
		if ( !m_studioHdr( ) )
			LockStudioHdr( this );

		return m_studioHdr( );
	}
	bone_accessor_t &GetBoneAccessor( ) {
		static auto offset = *( uintptr_t * )( util::find( "client.dll", "8D 81 ? ? ? ? 50 8D 84 24" ) + 2 );

		return *( bone_accessor_t * )( ( uintptr_t )this + offset );
	}
	int &m_fEffects( ) {
		return *( int * )( ( uintptr_t )this + 0xEC );
	}

	CBoneMergeCache *&m_pBoneMergeCache( ) {
		//static auto offset = *( uintptr_t * )( util::find( "client.dll", "8B 8E ? ? ? ? E8 ? ? ? ? 8B 06" ) + 2 );

		return *( CBoneMergeCache ** )( ( uintptr_t )this + 0x28FC );

	}
	OFFSET( vec3_t, abs_vel, 0x94 );
	OFFSET( float, sim_time, g.m_offsets->m_player.m_simulation_time );
	OFFSET( vec3_t, origin, g.m_offsets->m_player.m_origin );
	OFFSET( int, active_weapon, g.m_offsets->m_player.m_active_weapon );
	VFUNC( is_player( ), 152, bool( __thiscall * )( decltype( this ) ) )
};
class CBoneMergeCache;
class weapon_world_model_t : public entity_t {
public:
	bool HoldsPlayerAnim( ) {
		using LockStudioHdr_t = bool( __thiscall * )( void * );
		static auto LockStudioHdr = ( LockStudioHdr_t )util::find( "server.dll", "57 8B F9 83 BF ? ? ? ? ? 75 ? 83 BF ? ? ? ? ?" );
		return LockStudioHdr( this );
	}
};

class weapon_t : public entity_t {
public:
	float inaccuracy( ) {
		using original_fn = float( __thiscall * )( void * );
		return ( *( original_fn ** )this )[ 469 ]( this );
	}

	float get_spread( ) {
		using original_fn = float( __thiscall * )( void * );
		return ( *( original_fn ** )this )[ 439 ]( this );
	}

	void update_accuracy_penalty( ) {
		using original_fn = void( __thiscall * )( void * );
		( *( original_fn ** )this )[ 471 ]( this );
	}
	float get_max_speed( ) {
		using original_fn = float( __thiscall* )( void* );
		return ( *( original_fn** )this )[ 429 ]( this );
	}
	//VFUNC( get_weapon_info( ), 446, weapon_info_t *( __thiscall * )( void * ) )
	OFFSET( bool, pin_pulled, g.m_offsets->m_weapon.m_pin_pulled );
	OFFSET( bool, smoke_effect_begin_tick, g.m_offsets->m_weapon.smoke_effect_begin_tick );
	OFFSET( float, throw_time, g.m_offsets->m_weapon.m_throw_time );
	OFFSET( float, throw_strength, g.m_offsets->m_weapon.m_throw_strength );
	OFFSET( float, next_primary_attack, g.m_offsets->m_weapon.next_primary_attack );
	OFFSET( float, next_secondary_attack, g.m_offsets->m_weapon.next_secondary_attack );
	OFFSET( int, item_definition_index, g.m_offsets->m_weapon.item_definition_index );
	OFFSET( int, clip1_count, g.m_offsets->m_weapon.clip1_count );
	OFFSET( int, clip2_count, g.m_offsets->m_weapon.clip2_count );
	OFFSET( int, primary_reserve_ammo_acount, g.m_offsets->m_weapon.primary_reserve_ammo_acount );
	OFFSET( float, recoil_index, g.m_offsets->m_weapon.recoil_index );
	OFFSET( float, last_shot_time, g.m_offsets->m_weapon.last_shot_time );
	OFFSET( float, zoom_level, g.m_offsets->m_weapon.zoom_level );
	OFFSET( int, entity_quality, g.m_offsets->m_weapon.entity_quality );
	OFFSET( int, weapon_model, g.m_offsets->m_weapon.weapon_model );
};

class collideable_t {
public:
	vec3_t &mins( ) {
		using original_fn = vec3_t & ( __thiscall * )( void * );
		return ( *( original_fn ** )this )[ 1 ]( this );
	}
	vec3_t &maxs( ) {
		using original_fn = vec3_t & ( __thiscall * )( void * );
		return ( *( original_fn ** )this )[ 2 ]( this );
	}
};

class player_t : public entity_t {
public:
	bool dormant( ) {
		auto networkable_ = networkable( );
		if ( !networkable_ )
			return false;
		return networkable_->dormant( );
	}
	bool setup_bones( matrix_t *out, int max_bones, int mask, float time ) {
		auto animating_ = animating( );
		if ( !animating_ )
			return false;
		return animating_->setup_bones( out, max_bones, mask, time );
	}

	void set_angles( ang_t angles ) {
		using original_fn = void( __thiscall * )( void *, const ang_t & );
		static auto set_angles_fn = ( original_fn )( util::find( "client.dll", "55 8B EC 83 E4 F8 83 EC 64 53 56 57 8B F1" ) );
		set_angles_fn( this, angles );
	}
	
	collideable_t *collideable( ) {
		using original_fn = collideable_t * ( __thiscall * )( void * );
		return ( *( original_fn ** )this )[ 3 ]( this );
	}
	anim_state *get_anim_state( ) {
		static auto anim_state_w = *( uintptr_t * )( util::find( "client.dll", "8B 8E ? ? ? ? 85 C9 74 3E" ) + 2 );
		return *reinterpret_cast< anim_state ** >( this + anim_state_w );
	}

	static void UpdateButtons( player_t *target, int buttons ) {
		const auto buttons_changed = buttons ^ target->m_buttons( );
		target->m_last_buttons( ) = target->m_buttons( );
		target->m_buttons( ) = buttons;
		target->m_buttons_pressed( ) = buttons & buttons_changed;
		target->m_buttons_unpressed( ) = buttons_changed & ~buttons;
	}

	__forceinline void draw_model( int flags = 0x00000001, const RenderableInstance_t &instance = {} ) {
		return util::get_virtual_function< void( __thiscall * )( void *, int, const RenderableInstance_t & )>( animating( ), 9 )( animating( ), flags, instance );
	}

	datamap_t *GetDataDescMap( ) {
		typedef datamap_t *( __thiscall *o_GetDataDescMap )( void * );
		return util::get_virtual_function<o_GetDataDescMap>( this, 15 )( this );
	}

	datamap_t *GetPredDescMap( ) {
		typedef datamap_t *( __thiscall *o_GetPredDescMap )( void * );
		return util::get_virtual_function<o_GetPredDescMap>( this, 17 )( this );
	}

	/*void update_dispatch_layer( animation_layer_t *layer, CStudioHdr *studio_hdr ) {
		if( !studio_hdr || !layer )
		{
		  if( layer )
		  {
		    layer->m_nDispatchedDst = ACT_INVALID;
		  }
		
		  return;
		}
		
		if( layer->m_pDispatchedStudioHdr != studio_hdr || layer->m_nDispatchedSrc != layer->m_nSequence || layer->m_nDispatchedDst >= studio_hdr->GetNumSeq() )
		{
		  layer->m_pDispatchedStudioHdr = studio_hdr;
		  layer->m_nDispatchedSrc = layer->m_nSequence;
		
		  const char* pszLayerName = GetSequenceName( layer->m_nSequence );
		  layer->m_nDispatchedDst = studio_hdr->LookupSequence( pszLayerNAme );
		}
	}*/

	void calc_abs_velocity() {
		static auto fn_ptr = util::find("client.dll", "55 8B EC 83 E4 F8 83 EC 1C 53 56 57 8B F9 F7 87", 0);
		if (fn_ptr) {
			auto fn = (void(__thiscall*)(void*))(fn_ptr);
			fn(this);
		}
	}

	VFUNC ( world_space_center( ), 78, const vec3_t &( __thiscall * )( decltype( this ) ) )
	VFUNC( get_eye_pos( vec3_t *pos ), 163, void( __thiscall * )( decltype( this ), vec3_t * ), pos )
		OFFSET( int, m_buttons, 0x31E8 )
		OFFSET( int, m_last_buttons, 0x31DC )
		OFFSET( int, m_buttons_pressed, 0x31E0 )
		OFFSET( int, m_buttons_unpressed, 0x31E4 )
		OFFSET( int, m_next_think, 0xF8 )
		OFFSET( cmd_t *, m_cmd, 0x3314 )
		OFFSET( cmd_t, m_cmd_ukn, 0x326C )
		OFFSET( int, m_ground_entity, g.m_offsets->m_player.m_ground_ent );
	OFFSET( ang_t, aim_punch, g.m_offsets->m_player.m_aim_punch_angle );
	OFFSET( ang_t, punch, g.m_offsets->m_player.m_punch_angle );
	OFFSET( int, armor, g.m_offsets->m_player.m_armor );
	OFFSET(float, duck_amount, g.m_offsets->m_player.m_duck_amount);
	OFFSET(float, duck_speed, g.m_offsets->m_player.m_duck_speed);
	OFFSET( bool, ducking, g.m_offsets->m_player.m_ducking );
	OFFSET( ang_t, eye_angles, g.m_offsets->m_player.m_eye_angles );
	OFFSET( int, flags, g.m_offsets->m_player.m_flags );
	OFFSET( float, flash_alpha, g.m_offsets->m_player.m_flash_alpha );
	OFFSET( float, flash_duration, g.m_offsets->m_player.m_flash_duration );
	OFFSET( float, fov_time, g.m_offsets->m_player.m_fov_time );
	OFFSET( bool, defuser, g.m_offsets->m_player.m_defuser );
	OFFSET( bool, gun_game_immunity, g.m_offsets->m_player.m_gun_game_immunity );
	OFFSET( bool, heavy_armor, g.m_offsets->m_player.m_heavy_armor );
	OFFSET( int, health, g.m_offsets->m_player.m_health );
	OFFSET( bool, alive, g.m_offsets->m_player.m_health );
	OFFSET( int, hitbox_set, g.m_offsets->m_player.m_hitbox_set );
	OFFSET( bool, defusing, g.m_offsets->m_player.m_defusing );
	OFFSET( bool, has_helment, g.m_offsets->m_player.m_has_helmet );
	OFFSET( bool, is_scoped, g.m_offsets->m_player.m_is_scoped );
	OFFSET( int, life_state, g.m_offsets->m_player.m_life_state );
	OFFSET( float, lower_body_yaw, g.m_offsets->m_player.m_lower_body_yaw );
	OFFSET( float, velocity_modifier, g.m_offsets->m_player.velocity_modifier );
	OFFSET( float, max_speed, g.m_offsets->m_player.m_max_speed );
	OFFSET( int, money, g.m_offsets->m_player.m_money );
	OFFSET( float, next_attack, g.m_offsets->m_player.m_next_attack );
	OFFSET( bool, night_vision_enabled, g.m_offsets->m_player.m_night_vision_enabled );
	OFFSET( int, observer_target, g.m_offsets->m_player.m_observer_target );
	OFFSET( int, shots_fired, g.m_offsets->m_player.m_shots_fired );
	OFFSET( int, smoke_grenade_tick_begin, g.m_offsets->m_player.m_smoke_grenade_tick_begin );
	OFFSET( int, tick_base, g.m_offsets->m_player.m_tick_base );
	OFFSET( vec3_t, velocity, g.m_offsets->m_player.m_velocity );
	OFFSET( void *, view_model, g.m_offsets->m_player.m_view_model );
	OFFSET( bool, has_night_vision, g.m_offsets->m_player.m_has_night_vision );
	OFFSET( float, old_sim_time, g.m_offsets->m_player.m_simulation_time + 4 );
};
class inferno_t : public entity_t {
public:
	OFFSET(int, m_thrower, g.m_offsets->m_player.m_hOwnerEntity);
	OFFSETPTR(int, m_fire_x, g.m_offsets->m_player.m_fire_x);
	OFFSETPTR(int, m_fire_y, g.m_offsets->m_player.m_fire_y);
	OFFSETPTR(int, m_fire_z, g.m_offsets->m_player.m_fire_z);
	OFFSETPTR(bool, m_fire_burning, g.m_offsets->m_player.m_fire_burning);
	OFFSET(int, m_fire_count, g.m_offsets->m_player.m_fire_count);
};