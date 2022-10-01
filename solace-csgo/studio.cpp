#include "studio.h"
#include "includes.h"

const studio_hdr_t *CStudioHdr::GroupStudioHdr ( int i ) {
	static auto group = reinterpret_cast< studio_hdr_t *(__thiscall *) ( void *, int ) >(util::find(
		"client.dll", "55 8B EC 56 8B F1 57 85 F6" ));
	return group( this, i );
}

CBoneMergeCache::CBoneMergeCache( ) {
}

CBoneMergeCache* CBoneMergeCache::init ( void *ent ) {
	typedef int( __thiscall *Construct ) ( void * );
	static auto construct = reinterpret_cast< Construct >( util::find( "server.dll", "56 8B F1 0F 57 C0 C7 86 ? ? ? ? ? ? ? ? C7 86 ? ? ? ? ? ? ? ?" ) );

	if ( !construct )
		return nullptr;
	auto *const cache = static_cast< CBoneMergeCache * >(g.m_interfaces->mem_alloc( )->alloc( 676 ));
	if ( !cache )
		return nullptr;
	construct( cache );
	typedef void *( __thiscall *Construct_1 ) ( void *, void * );
	static auto construct_1 = reinterpret_cast< Construct_1 >( util::find( "server.dll", "55 8B EC 8B 45 ? 56 8B F1 89 06 C7 46 ? ? ? ? ? C7 46 ? ? ? ? ? C7 46 ? ? ? ? ? C7 46 ? ? ? ? ? C7 46 ? ? ? ? ? C6 86 ? ? ? ? ?" ) );

	if ( !construct_1 || !cache )
		return nullptr;
	construct_1( cache, ent );
	return cache;
}

void CBoneMergeCache::UpdateCache ( ) {
	typedef void( __thiscall *Init_t ) ( void * );
	static auto init = ( Init_t )util::find( "server.dll", "55 8B EC 83 EC ? 53 8B D9 57 8B 3B" );
	init( ( DWORD ** )this );
	
}

void CBoneMergeCache::MergeMatchingPoseParams( ) {
	typedef void( __thiscall *Init_t ) ( void * );
	static auto init = ( Init_t )util::find( "client.dll", "55 8B EC 83 EC ? 53 56 8B F1 57 89 75 ? E8 ? ? ? ? 83 7E ? ? 0F 84 ? ? ? ?" );
	init( this );
	return;
	UpdateCache( );

	if ( *( DWORD * )( this + 0x10 ) && *( DWORD * )( this + 0x8C ) ) {
		auto v26 = 0;
		auto index = ( int * )( this + 0x20 );
		do {
			if ( *index != -1 ) {
				typedef int( __thiscall *Init_t ) ( void *, int );
				typedef uint64_t( __thiscall *Init_t2 ) ( void *, int, int );
				static auto ukn = reinterpret_cast< Init_t >(util::find( "server.dll",
				                                                         "55 8B EC 56 57 8B F9 83 BF ? ? ? ? ? 75 ? A1 ? ? ? ? 8B 30 8B 07 FF 50 ? 8B 0D ? ? ? ? 50 FF 56 ? 85 C0 74 ? 8B CF E8 ? ? ? ? 8B B7 ? ? ? ? 85 F6 74 ? 83 3E ? 74 ? 8B CE E8 ? ? ? ? 84 C0 74 ?" )
				);
				static auto ukn_1 = reinterpret_cast< Init_t2 >(util::find( "server.dll",
				                                                            "55 8B EC 51 56 57 8B F9 0F 28 C2 F3 0F 11 45 ? 83 BF ? ? ? ? ? 75 ? A1 ? ? ? ? 8B 30 8B 07 FF 50 ? 8B 0D ? ? ? ? 50 FF 56 ? 85 C0 74 ? 8B CF E8 ? ? ? ? F3 0F 10 45 ? 8B 8F ? ? ? ? 85 C9 74 ? 83 39 ? 75 ? 33 C9 85 C9" )
				);
				auto v28 = ukn( reinterpret_cast< DWORD * >(*reinterpret_cast< DWORD * >(this + 4)), v26 );
				ukn_1( *reinterpret_cast< DWORD ** >(this), v28, *index );
			}
			++v26;
			++index;
		} while ( v26 < 24 );
	}
}

void CBoneMergeCache::CopyFromFollow ( vec3_t *pos, quaternion_t *quaternion, int i, vec3_t vec3s[],
	quaternion_t quaternions[] ) {
	typedef void( __thiscall *Init_t ) ( void *, vec3_t *, quaternion_t *, int , vec3_t*,
										 quaternion_t* );
	static auto init = reinterpret_cast< Init_t >( util::find( "client.dll",
												   "55 8B EC 83 EC ? 53 56 57 8B F9 89 7D ? E8 ? ? ? ? 83 7F ? ? 0F 84 ? ? ? ? 8B 87 ? ? ? ? 85 C0 74 ?" )
												   );
	init( this, pos, quaternion, i, vec3s, quaternions );
}

void CBoneMergeCache::CopyToFollow ( vec3_t vec3s[], quaternion_t quaternions[], int i, vec3_t *pos,
	quaternion_t *quaternion ) {
	typedef void( __thiscall *Init_t ) ( void *, vec3_t *, quaternion_t *, int, vec3_t *,
										 quaternion_t * );
	static auto init = reinterpret_cast< Init_t >( util::find( "client.dll",
												   "55 8B EC 83 EC ? 53 56 57 8B F9 89 7D ? E8 ? ? ? ? 83 7F ? ? 0F 84 ? ? ? ? 8B 87 ? ? ? ? 85 C0 0F 84 ? ? ? ?" )
												   );
	init( this, vec3s, quaternions, i, pos, quaternion );
}

void CIKContext::init ( ) const {
	typedef void( __thiscall *Construct ) ( void * );
	static auto construct = reinterpret_cast< Construct >( util::find( "client.dll", "56 8B F1 6A ? 6A ? C7 86 ? ? ? ? ? ? ? ?" ) );

	if ( !construct )
		return;

	auto *context = static_cast< CIKContext * >( g.m_interfaces->mem_alloc( )->alloc( 0x1070 ) );
	construct( context );
	return;
}
//void CIKContext::operator delete ( void *ptr ) {
//	g.m_interfaces->mem_alloc( )->free( ptr );
//}
//
void CIKContext::unk_1( ) {
	typedef void( __thiscall *Init_t ) ( void * );
	static auto init = reinterpret_cast< Init_t >( util::find( "server.dll",
												   "56 8B F1 57 8D 8E ? ? ? ? E8 ? ? ? ? 8D 8E ? ? ? ? E8 ? ? ? ? 83 BE ? ? ? ? ?" )
												   );
	init( this );
}

void CIKContext::unk_2( ) {
	typedef void( __thiscall *Init_t ) ( void * );
	static auto init = reinterpret_cast< Init_t >( util::find( "server.dll", "53 8B D9 F6 C3 ?" ) );
	init( this );
}

void CIKContext::ClearTargets( ) {
	const auto m_iTargetCount = *reinterpret_cast< int * >( reinterpret_cast< uintptr_t >( this ) + 0xFF0 );
	auto m_pIkTarget = reinterpret_cast< CIKTarget * >( reinterpret_cast< uintptr_t >( this ) + 0xD0 );
	for ( auto i = 0; i < m_iTargetCount; i++ ) {
		m_pIkTarget->m_iFrameCount = -9999;
		m_pIkTarget++;
	}
}

void CIKContext::Init ( CStudioHdr *hdr, ang_t angles, vec3_t origin, float currentTime, int frames, int boneMask ) {
	typedef void ( __thiscall *Initfn ) ( /*CIKContext*/CIKContext *, CStudioHdr *, const ang_t &, const vec3_t &,
	                                                    float, int, int );
	static auto init = (Initfn)util::find( "client.dll", "55 8B EC 83 EC ? 8B 45 ? 56 57 8B F9 8D 8F ? ? ? ?" );
	init( this, hdr, angles, origin, currentTime, frames, boneMask );
}

void CIKContext::UpdateTargets ( vec3_t pos[], quaternion_t qua[], matrix_t *matrix, uint8_t *boneComputed ) {
	typedef void ( __thiscall *UpdateTargets_t ) ( void *, vec3_t[], quaternion_t[], matrix_t *, uint8_t * );
	static auto updateTargets = reinterpret_cast< UpdateTargets_t >(util::find(
		"client.dll", "55 8B EC 83 E4 ? 81 EC ? ? ? ? 33 D2" ));
	updateTargets( this, pos, qua, matrix, boneComputed );
}

void CIKContext::SolveDependencies ( vec3_t pos[], quaternion_t qua[], matrix_t *matrix, uint8_t *boneComputed ) {
	typedef void ( __thiscall *SolveDependencies_t ) ( void *, vec3_t [], quaternion_t[], matrix_t *, uint8_t * );
	static auto solveDependencies = reinterpret_cast< SolveDependencies_t >(util::find(
		"client.dll", "55 8B EC 83 E4 ? 81 EC ? ? ? ? 8B 81 ? ? ? ?" ));
	solveDependencies( this, pos, qua, matrix, boneComputed );
}

void CIKContext::CopyTo ( CIKContext * ik, int p ) {
	typedef int( __thiscall *SolveDependencies_t ) ( CIKContext *, CIKContext *, int );
	static auto solveDependencies = reinterpret_cast< SolveDependencies_t >( util::find(
		"client.dll", "55 8B EC 83 EC ? 8B 45 ? 57 8B F9 89 7D ?" ) );
	solveDependencies( this, ik, p );
	
}

void CIKContext::AddDependencies ( mstudioseqdesc_t &seqdesc, int iSequence, float flCycle,
                                   const float poseParameters[], float flWeight ) {
	using AddDependenciesFn = void( __thiscall *) ( CIKContext *, mstudioseqdesc_t &, int, float, const float [],
	                                                float );
	static auto AddDependencies = reinterpret_cast< AddDependenciesFn >( util::find(
		"client.dll", "55 8B EC 81 EC ? ? ? ? 53 56 57 8B F9 0F 28 CB" ) );
	// server.dll - 

	AddDependencies( this, seqdesc, iSequence, flCycle, poseParameters, flWeight );
}
