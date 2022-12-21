#pragma once

class player_t;
class entity_t;
class CStudioHdr;

#define ANIM_TRANSITION_WALK_TO_RUN 0
#define ANIM_TRANSITION_RUN_TO_WALK 1

#define CSGO_ANIM_WALK_TO_RUN_TRANSITION_SPEED 2.0f

const float CS_PLAYER_SPEED_WALK_MODIFIER = 0.52f;

const float CS_PLAYER_SPEED_RUN = 260.0f;

class animation_layer_t {
public:
	float   m_anim_time;			// 0x0000
	float   m_fade_out_time;		// 0x0004
	CStudioHdr* m_pDispatchedStudioHdr;				// 0x0008
	int     m_nDispatchedSrc;				// 0x000C
	int     m_nDispatchedDst;				// 0x0010
	int     m_order;				// 0x0014
	int     m_sequence;				// 0x0018
	float   m_prev_cycle;			// 0x001C
	float   m_weight;				// 0x0020
	float   m_weight_delta_rate;	// 0x0024
	float   m_playback_rate;		// 0x0028
	float   m_cycle;				// 0x002C
	entity_t* m_owner;				// 0x0030
	int     m_bits;					// 0x0034
}; // size: 0x0038

class activity_modifiers_wrapper {
private:
	uint32_t gap[ 0x4D ]{ 0 };
	CUtlVector<uint16_t> modifiers{};

public:
	activity_modifiers_wrapper( ) = default;

	explicit activity_modifiers_wrapper( CUtlVector<uint16_t> current_modifiers ) {
		modifiers.RemoveAll( );
		modifiers.GrowVector( current_modifiers.Count( ) );

		for ( auto i = 0; i < current_modifiers.Count( ); i++ )
			modifiers[ i ] = current_modifiers[ i ];
	}

	void add_modifier( const char* name ) {
		using add_activity_modifier_fn_t = void( __thiscall* )( activity_modifiers_wrapper*, const char* );
		static const auto add_activity_modifier =
			reinterpret_cast< add_activity_modifier_fn_t >( util::find( "server.dll", "55 8B EC 8B 55 ? 83 EC ? 56" ) );

		if ( add_activity_modifier && name )
			add_activity_modifier( this, name );
	};

	CUtlVector<uint16_t> get( ) const {
		return modifiers;
	}
};

class anim_state {
public:
	PAD( 0x1C );				// 0x0000
	player_t *m_outer;			// 0x001C
	PAD( 0x40 );				// 0x0020
	player_t *m_player;			// 0x0060
	PAD( 0x8 );					// 0x0064
	float   m_time;				// 0x006C
	int     m_frame;			// 0x0070
	float   m_update_delta;		// 0x0074
	float   m_eye_yaw;			// 0x0078
	float   m_eye_pitch;		// 0x007C
	float   m_goal_feet_yaw;	// 0x0080
	float   m_cur_feet_yaw;		// 0x0084
	float m_flCurrentTorsoYaw;                // 0x88
	float m_flUnknownVelocityLean;            // 0x8C changes when moving/jumping/hitting ground
	float m_flLeanAmount;                     // 0x90
	float m_flUnknown1;                       // 0x94
	float feetCycle;
	float feetYawRate;
	PAD( 0x4 );				// 0x0088
	float   m_dip_cycle;        // 0x00A4
	float   m_dip_adj;          // 0x00A8
	float m_fUnknown3;								  // 0xAC
	vec3_t m_vOrigin;								  // 0xB0, 0xB4, 0xB8
	vec3_t m_vLastOrigin;							  // 0xBC, 0xC0, 0xC4
	vec3_t m_vecVelocity;							  // 0xC8
	vec3_t m_vecNormalizedVelocity;
	vec3_t m_vecVelocityNormalizedNonZero;
	float   m_speed;			// 0x00EC
	float   m_fall_velocity;    // 0x00F0
	float					m_flSpeedAsPortionOfRunTopSpeed;
	float					m_flSpeedAsPortionOfWalkTopSpeed;
	float					m_flSpeedAsPortionOfCrouchTopSpeed;
	float m_flTimeSinceStartedMoving;			  // 0x100
	float m_flTimeSinceStoppedMoving;
	bool    m_ground;			// 0x0108
	bool    m_land;             // 0x0109

	float					m_flJumpToFall;
	float					m_flDurationInAir;
	float					m_flLeftGroundHeight;
	float					m_flLandAnimMultiplier;

	float					m_flWalkToRunTransition;

	bool					m_bLandedOnGroundThisFrame;
	bool					m_bLeftTheGroundThisFrame;

	float					m_flInAirSmoothValue;

	bool					m_bOnLadder;
	float					m_flLadderWeight;
	float					m_flLadderSpeed;

	bool					m_bWalkToRunTransitionState;

	PAD(0x1F9);				// 0x010C
	bool    m_dip_air;			// 0x0328
	bool					m_bSmoothHeightValid;
	vec3_t					m_flLastTimeVelocityOverTen;

	float					m_flAimYawMin;
	float					m_flAimYawMax;
	float   m_min_pitch;        // 0x033C
	float   m_max_pitch;        // 0x033C
	PAD( 0x4 );					// 0x0340


	using CreateAnimState_t = void( __thiscall * ) ( anim_state *, player_t * );
	inline void CreateAnimationState ( player_t *holder ) {

		static auto func = util::find( "client.dll", "55 8B EC 56 8B F1 B9 ? ? ? ? C7 46" );
		if ( !func )
			return;

		reinterpret_cast< CreateAnimState_t >(func)( this, holder );
	}

	typedef void( __vectorcall *fnUpdateAnimState ) ( PVOID, PVOID, float, float, float, char );
	inline void UpdateAnimationState ( ang_t ang ) {

		static auto UpdateAnimState = address( util::find( "client.dll", "E8 ? ? ? ? 0F 57 C0 0F 11 86" ) ).rel32<
			uintptr_t >( 0x1 );

		if ( !UpdateAnimState || (this == nullptr) )
			return;

		__asm {
			push 1
			mov ecx, this
			movss xmm1, dword ptr[ ang + 4 ]
			movss xmm2, dword ptr[ ang ]
			call UpdateAnimState
			}
	}

	using ResetAnimState_t = void( __thiscall * ) ( anim_state * );
	inline void ResetAnimationState ( ) {

		static auto func = reinterpret_cast< ResetAnimState_t >(util::find( "client.dll", "56 6A 01 68 ? ? ? ? 8B F1" ));
		if ( !func )
			return;

		func( this );
	}

};
