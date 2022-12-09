#include "animstate.h"
#include "player.h"

void anim_state::ModifyEyePosition( matrix_t* mat, vec3_t* pos ) {
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

				pos->z = static_cast<float>( ( ( v7 - ( *pos ).z ) ) * ( ( static_cast< double >( v13 ) * v13 * 3.0 ) - ( static_cast< double >( v13 ) * v13 * 2.0 * v13 ) ) ) + ( *pos ).z;
			}
		}
	}
}

using CreateAnimState_t = void( __thiscall* ) ( anim_state*, player_t* );
void anim_state::CreateAnimationState( player_t* holder ) {

	static auto func = util::find( "client.dll", "55 8B EC 56 8B F1 B9 ? ? ? ? C7 46" );
	if ( !func )
		return;

	reinterpret_cast< CreateAnimState_t >( func )( this, holder );
}

typedef void( __vectorcall* fnUpdateAnimState ) ( PVOID, PVOID, float, float, float, char );
void anim_state::UpdateAnimationState( ang_t ang ) {

	static auto UpdateAnimState = address( util::find( "client.dll", "E8 ? ? ? ? 0F 57 C0 0F 11 86" ) ).rel32<
		uintptr_t >( 0x1 );

	if ( !UpdateAnimState || ( this == nullptr ) )
		return;

	__asm {
		push 1
		mov ecx, this
		movss xmm1, dword ptr[ ang + 4 ]
		movss xmm2, dword ptr[ ang ]
		call UpdateAnimState
	}
}

using ResetAnimState_t = void( __thiscall* ) ( anim_state* );
void anim_state::ResetAnimationState( ) {

	static auto func = reinterpret_cast< ResetAnimState_t >( util::find( "client.dll", "56 6A 01 68 ? ? ? ? 8B F1" ) );
	if ( !func )
		return;

	func( this );
}