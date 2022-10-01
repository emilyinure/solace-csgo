#pragma once

class player_t;

#define ANIM_TRANSITION_WALK_TO_RUN 0
#define ANIM_TRANSITION_RUN_TO_WALK 1

#define CSGO_ANIM_WALK_TO_RUN_TRANSITION_SPEED 2.0f

const float CS_PLAYER_SPEED_WALK_MODIFIER = 0.52f;

const float CS_PLAYER_SPEED_RUN = 260.0f;

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
	PAD( 0x14 );				// 0x0088
	float feetYawRate;
	PAD( 0x4 );				// 0x0088
	float   m_dip_cycle;        // 0x00A4
	float   m_dip_adj;          // 0x00A8
	PAD(0x34);				// 0x00A8
	vec3_t m_vecVelocityNormalizedNonZero;
	float   m_speed;			// 0x00EC
	float   m_fall_velocity;    // 0x00F0
	float					m_flSpeedAsPortionOfRunTopSpeed;
	float					m_flSpeedAsPortionOfWalkTopSpeed;
	float					m_flSpeedAsPortionOfCrouchTopSpeed;

	PAD(0x8);				// 0x00F0
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

	void ModifyEyePosition( matrix_t *mat, vec3_t *pos ) {
		//  if ( *(this + 0x50) && (*(this + 0x100) || *(this + 0x94) != 0.0 || !sub_102C9480(*(this + 0x50))) )
		m_bSmoothHeightValid = false;
		if ( m_player &&
			 ( m_land || m_player->duck_amount( ) != 0.f || !m_player->m_ground_entity( ) ) ) {
			const auto v5 = 8;

			if constexpr ( v5 != -1 ) {
				const vec3_t head_pos(
					mat[ 8 ][ 0 ][ 3 ],
					mat[ 8 ][ 1 ][ 3 ],
					mat[ 8 ][ 2 ][ 3 ] );

				const auto v12 = head_pos;
				const float v7 = v12.z + 1.7;

				const auto v8 = pos->z;
				if ( v8 > v7 ) // if (v8 > (v12 + 1.7))
				{
					auto v13 = 0.f;
					const auto v3 = pos->z - v7;

					const float v4 = ( v3 - 4.f ) * 0.16666667;
					if ( v4 >= 0.f )
						v13 = std::fminf( v4, 1.f );

					pos->z = static_cast<float>( ( ( v7 - ( *pos ).z ) ) * ( ( static_cast< double >(v13) * v13 * 3.0 ) - ( static_cast< double >( v13 ) * v13  * 2.0  * v13 ) ) ) + ( *pos ).z;
				}
			}
		}
	}
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
