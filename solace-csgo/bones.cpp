#include "bones.h"
#include "includes.h"
#include "thread_handler.h"


bool bones_t::setup( player_t *player, bone_array_t *out, std::shared_ptr<player_record_t> record, CIKContext *ipk ) {
	// if the record isnt setup yet.

	//std::unique_lock<std::mutex> lock( g_thread_handler.queue_mutex );
	// run setupbones rebuilt.
	g.m_interfaces->mdlcache( )->begin_coarse_lock( );
	g.m_interfaces->mdlcache( )->begin_lock( );

	record->m_setup = BuildBones( player, bone_used_by_anything & ~bone_used_by_bone_merge, out, record, ipk );

	g.m_interfaces->mdlcache( )->end_lock( );
	g.m_interfaces->mdlcache( )->end_coarse_lock( );

	return record->m_setup;
}


class PoseDebugger;

class _BoneSetup {
public:
	void init( const CStudioHdr* pStudioHdr, int boneMask, const float poseParameter[ ], PoseDebugger* pPoseDebugger = nullptr ) {
		m_boneMask = boneMask;
		m_flPoseParameters = poseParameter;
		m_pStudioHdr = pStudioHdr;
		m_pPoseDebugger = pPoseDebugger;
	}
	_BoneSetup( const CStudioHdr* pStudioHdr, int boneMask, const float poseParameter[ ], PoseDebugger* pPoseDebugger = nullptr ) {
		m_boneMask = boneMask;
		m_flPoseParameters = poseParameter;
		m_pStudioHdr = pStudioHdr;
		m_pPoseDebugger = pPoseDebugger;
	}

	void InitPose( vec3_t* pos, quaternion_t* q ) const {
		static void* fn = nullptr;
		if ( !fn ) fn = util::find( "server.dll", "55 8B EC 83 EC 10 53 8B D9 89 55" );

		auto studioHdr = m_pStudioHdr;
		auto boneMask = m_boneMask;

		__asm
		{
			pushad
			pushfd

			mov ecx, studioHdr
			mov edx, pos
			push boneMask
			push q
			call fn
			add esp, 8

			popfd
			popad
		}
	}

	void AccumulatePose( vec3_t pos[ ], quaternion_t q[ ], int iSequence, float flCycle, float flWeight, float flTime, CIKContext* pIKContext ) const {
#ifdef _DEBUG
		//Remove breakpoint when debugger is attached
		static bool onceOnly = false;
		if ( !onceOnly ) {
			onceOnly = true;
			auto pattern = util::find( "server.dll", "? F3 0F 10 4D ? 0F 57 C0 0F 2F C1 F3 0F 11 44 24 ?" );
			if ( pattern ) {
				DWORD oldProt;
				VirtualProtect( pattern, 1, PAGE_EXECUTE_READWRITE, &oldProt );
				*pattern = 0x90;
				VirtualProtect( pattern, 1, oldProt, &oldProt );
			}
		}
#endif

		static void* fn = nullptr;
		if ( !fn ) fn = ( util::find( "server.dll", "B8 ? ? ? ? E8 ? ? ? ? A1 ? ? ? ? 56 57 8B F9 B9" ) - 0x6 );

		__asm
		{
			pushad
			pushfd

			mov ecx, this
			push pIKContext
			push flTime
			push flWeight
			push flCycle
			push iSequence
			push q
			push pos
			call fn

			popfd
			popad
		}
	}

	void CalcAutoplaySequences( vec3_t pos[ ], quaternion_t q[ ], float flRealTime, CIKContext* pIKContext ) const {
		static void* fn = nullptr;
		if ( !fn ) fn = ( util::find( "server.dll", "55 8B EC 83 EC 10 53 56 57 8B 7D 10" ) );
		auto* globals = g.m_interfaces->globals( ).operator void* ( );
		__asm
		{
			mov     eax, globals
			mov     ecx, this
			push    0
			push    q
			push    pos
			movss   xmm3, flRealTime
			call    fn
		}
	}

	void CalcBoneAdj( vec3_t pos[ ], quaternion_t q[ ], const float* encodedControllerArray ) const {
		static void* fn = nullptr;
		if ( !fn ) fn = ( util::find( "server.dll", "55 8B EC 83 E4 F8 81 EC ? ? ? ? 8B C1 89" ) );

		const auto* studioHdr = m_pStudioHdr;
		auto boneMask = m_boneMask;

		__asm
		{
			pushad
			pushfd

			mov ecx, studioHdr
			mov edx, pos
			push boneMask
			push encodedControllerArray
			push q
			call fn
			add esp, 0ch

			popfd
			popad
		}

	}

	const CStudioHdr* m_pStudioHdr;
	int m_boneMask;
	const float* m_flPoseParameters;
	PoseDebugger* m_pPoseDebugger;
};



class BoneSetup {
public:
	BoneSetup( const CStudioHdr *pStudioHdr, int boneMask, const float *poseParameter ) {
		m_pBoneSetup = ( _BoneSetup* )malloc( sizeof( _BoneSetup ) );
		m_pBoneSetup->init( pStudioHdr, boneMask, poseParameter );
	}
	~BoneSetup( ) {
		free( m_pBoneSetup );
	}
	void InitPose( vec3_t *pos, quaternion_t *q ) const {
		m_pBoneSetup->InitPose( pos, q );
	}
	void AccumulatePose( vec3_t pos[ ], quaternion_t q[ ], int sequence, float cycle, float flWeight, float flTime, CIKContext *pIKContext ) const {
		m_pBoneSetup->AccumulatePose( pos, q, sequence, cycle, flWeight, flTime, pIKContext );
	}
	void CalcAutoplaySequences( vec3_t pos[ ], quaternion_t q[ ], float flRealTime, CIKContext *pIKContext ) const {
		m_pBoneSetup->CalcAutoplaySequences( pos, q, flRealTime, pIKContext );
	}
	void CalcBoneAdj( vec3_t pos[ ], quaternion_t q[ ], const float *encodedControllerArray ) const {
		m_pBoneSetup->CalcBoneAdj( pos, q, encodedControllerArray );
	}
private:
	_BoneSetup *m_pBoneSetup;
};

void GetSkeleton( player_t *player, CStudioHdr *studioHdr, vec3_t *pos, quaternion_t *q, int boneMask, float sim_time, CIKContext *ik ) {
	const BoneSetup boneSetup( studioHdr, boneMask, player->pose_parameters( ) );
	boneSetup.InitPose( pos, q );
	boneSetup.AccumulatePose( pos, q, player->sequence( ), player->cycle( ), 1.f, sim_time, ik );

	constexpr const auto MAX_LAYER_COUNT = 15;
	int layer[ MAX_LAYER_COUNT ]{};
	for ( auto i = 0; i < 15; i++ ) {
		layer[ i ] = MAX_LAYER_COUNT;
	}

	for ( auto i = 0; i < 15; i++ ) {
		auto *pLayer = &player->anim_overlay()[ i ];
		pLayer->m_owner = player;
		if ( ( pLayer->m_weight > 0.f ) && pLayer->m_sequence != -1 && pLayer->m_order >= 0 && pLayer->m_order < 15 ) {
			layer[ pLayer->m_order ] = i;
		}
	}
	weapon_world_model_t *weapon_world_model = nullptr;
	CBoneMergeCache *bone_merge_cache = nullptr;
	CBoneMergeCache *bone_merge_backup = nullptr;

	auto do_weapon_setup = false;

	auto *weapon = static_cast< weapon_t * >(g.m_interfaces->entity_list( )->get_client_entity_handle(
		                                           player->active_weapon( ) ));
	if ( weapon ) {
		weapon_world_model = static_cast< weapon_world_model_t * >(g.m_interfaces->entity_list( )->get_client_entity_handle(
			weapon->weapon_model( ) ));
		// Currently just return true in HoldsPlayerAnimations, something fucked on knives and I can't be bothered
		if ( weapon_world_model && weapon_world_model->GetModelPtr( ) ) {
			//bone_merge_backup = weapon_world_model->m_pBoneMergeCache( );
			//
			//bone_merge_cache = ( CBoneMergeCache::init( weapon_world_model ));
			//weapon_world_model->m_pBoneMergeCache( ) = bone_merge_cache;
	
			if ( weapon_world_model->m_pBoneMergeCache( ) )
				do_weapon_setup = true;
		}
	}

	if ( do_weapon_setup ) {

		const auto weapon_studio_hdr = weapon_world_model->GetModelPtr( );
		if ( weapon_studio_hdr ) {

			weapon_world_model->m_pBoneMergeCache( )->MergeMatchingPoseParams( );

			CIKContext weaponIK;
			weaponIK.Init( weapon_studio_hdr, player->abs_angles( ), player->abs_origin( ), sim_time, 0, BONE_USED_BY_BONE_MERGE );

			const BoneSetup weaponSetup( weapon_studio_hdr, BONE_USED_BY_BONE_MERGE, weapon_world_model->pose_parameters( ) );
			alignas( 16 ) vec3_t weaponPos[ 128 ];
			alignas( 16 ) quaternion_t weaponQ[ 128 ];

			weaponSetup.InitPose( weaponPos, weaponQ );

			for ( auto i = 0; i < 15; i++ ) {
				auto *pLayer = &player->anim_overlay( )[ i ];
				if ( pLayer->m_sequence <= 1 || pLayer->m_cycle <= 0.f ) {
					//if ( weapon_studio_hdr->m_pVModel )
					//	seq_count = ((CUtlVector< virtualsequence_t > *)(weapon_studio_hdr->m_pVModel + 20))->Count( );

					if ( pLayer->m_nDispatchedDst <= 0 || pLayer->m_nDispatchedDst >= studioHdr->GetNumSeq( ) ) {
						boneSetup.AccumulatePose( pos, q, pLayer->m_sequence, pLayer->m_cycle, pLayer->m_weight, sim_time, ik );
					} else {
						weapon_world_model->m_pBoneMergeCache( )->CopyFromFollow( pos, q, BONE_USED_BY_BONE_MERGE, weaponPos, weaponQ );

						if ( ik ) {
							auto &seqdesc = studioHdr->pSeqdesc( pLayer->m_sequence );
							ik->AddDependencies( seqdesc, pLayer->m_sequence, pLayer->m_cycle, player->pose_parameters( ), pLayer->m_weight );
						}

						weaponSetup.AccumulatePose( weaponPos, weaponQ, pLayer->m_nDispatchedDst, pLayer->m_cycle, pLayer->m_weight, sim_time, &weaponIK );

						weapon_world_model->m_pBoneMergeCache( )->CopyToFollow( weaponPos, weaponQ, BONE_USED_BY_BONE_MERGE, pos, q );

						weaponIK.CopyTo( ik, *( int * )( weapon_world_model->m_pBoneMergeCache( ) + 160 ) );
					}
				}
				boneSetup.AccumulatePose( pos, q, pLayer->m_sequence, pLayer->m_cycle, pLayer->m_weight, sim_time, ik );
			}

		}
	}
	else {
		for ( auto i = 0; i < 15; i++ ) {
			if ( layer[ i ] >= 0 && layer[ i ] < 15 ) {
				const auto pLayer = player->anim_overlay( )[ layer[ i ] ];
				boneSetup.AccumulatePose( pos, q, pLayer.m_sequence, pLayer.m_cycle, pLayer.m_weight, sim_time, ik );
			}
		}
	}

	CIKContext auto_ik;
	auto_ik.Init( studioHdr, player->abs_angles( ), player->abs_origin( ), sim_time, 0, boneMask );
	boneSetup.CalcAutoplaySequences( pos, q, sim_time, &auto_ik );

	if ( studioHdr->m_pStudioHdr->bone_controllers_count > 0 ) {
		boneSetup.CalcBoneAdj( pos, q, player->encoder_controller( ) );
	}

	//if ( weapon_world_model && bone_merge_cache ) {
	//	weapon_world_model->m_pBoneMergeCache( ) = bone_merge_backup;
	//	g.m_interfaces->mem_alloc( )->free( bone_merge_cache );
	//}
}
void Studio_BuildMatrices( const studio_hdr_t *pStudioHdr, const ang_t &angles, const vec3_t &origin,
						   const vec3_t pos[ ], const quaternion_t q[ ], int iBone, float flScale,
						   matrix_t bonetoworld[ 128 ], int boneMask ) {
	int i;

	int chain[ 128 ] = {};
	auto chainlength = 0;

	if ( iBone < -1 || iBone >= pStudioHdr->bones_count )
		iBone = 0;

	// build list of what bones to use
	if ( iBone == -1 ) {
		// all bones
		chainlength = pStudioHdr->bones_count;
		for ( i = 0; i < pStudioHdr->bones_count; i++ ) {
			chain[ chainlength - i - 1 ] = i;
		}
	} else {
		// only the parent bones
		i = iBone;
		while ( i != -1 ) {
			chain[ chainlength++ ] = i;
			i = pStudioHdr->boneParent( i );
		}
	}

	matrix_t bonematrix;
	matrix_t rotationmatrix; // model to world transformation

	auto q2 = angles;
	q2.z = 0;
	math::angle_matrix( q2, origin, &rotationmatrix );

	// Account for a change in scale
	if ( flScale < 1.0f - FLT_EPSILON || flScale > 1.0f + FLT_EPSILON ) {
		vec3_t vecOffset;
		math::MatrixGetColumn( rotationmatrix, 3, vecOffset );
		vecOffset -= origin;
		vecOffset *= flScale;
		vecOffset += origin;
		math::MatrixSetColumn( vecOffset, 3, rotationmatrix );

		// Scale it uniformly
		math::VectorScale( rotationmatrix[ 0 ], flScale, rotationmatrix[ 0 ] );
		math::VectorScale( rotationmatrix[ 1 ], flScale, rotationmatrix[ 1 ] );
		math::VectorScale( rotationmatrix[ 2 ], flScale, rotationmatrix[ 2 ] );
	}

	for ( int j = chainlength - 1; j >= 0; j-- ) {
		i = chain[ j ];
		if ( pStudioHdr->boneFlags( i ) & boneMask ) {
			math::QuaternionMatrix( q[ i ], pos[ i ], &bonematrix );

			const auto boneParent = pStudioHdr->boneParent( i );

			if ( boneParent == -1 ) {
				math::ConcatTransforms( rotationmatrix, bonematrix, &bonetoworld[ i ] );
			} else {
				math::ConcatTransforms( bonetoworld[ boneParent ], bonematrix, &bonetoworld[ i ] );
			}
		}
	}
}

bool bones_t::BuildBonesStripped( player_t *target, int mask, bone_array_t *out, CIKContext *ipk ) {
	alignas( 16 ) vec3_t		     pos[ 128 ];
	alignas( 16 ) quaternion_t     q[ 128 ];
	// get hdr.
	const auto hdr = target->GetModelPtr( );
	if ( !hdr ) 
		return false;

	// get ptr to bone accessor.
	const auto accessor = &target->GetBoneAccessor( );
	if ( !accessor )
		return false;
	// store origial output matrix.
	// likely cachedbonedata.
	const auto backup_matrix = accessor->m_pBones;
	if ( !backup_matrix ) 
		return false;
	//force game to call AccumulateLayers - pvs fix.
   //auto ipk = target->m_Ipk();
   //if (ipk) {
   //	ipk->ClearTargets();
   //	ipk->New();
   //}
    const int backup_eflags = target->iEFlags();
	const auto backup = target->m_Ipk( );
	target->m_Ipk( ) = ipk;
    m_running = true;
    target->InvalidatePhysicsRecursive(entity_t::ANGLES_CHANGED);
    target->InvalidatePhysicsRecursive(entity_t::ANIMATION_CHANGED);
    target->InvalidatePhysicsRecursive(entity_t::SEQUENCE_CHANGED);
    target->iEFlags() &= ~(1 << 11);
    target->iEFlags() |= (1 << 3);

	// set bone array for write.
	accessor->m_pBones = out;

	auto* computed = static_cast< uint8_t* >( g.m_interfaces->mem_alloc( )->alloc( sizeof( uint8_t[ 0x100 ] ) ) );
	//auto* computed = static_cast< uint8_t* >( malloc( sizeof( uint8_t[ 0x100 ] ) ) );
	std::memset( computed, 0, 0x100 );

	//target->StandardBlendingRules(hdr, pos, q, record->m_sim_time, mask);
	if ( ipk ) {
		ipk->Init( hdr, target->abs_angles( ), target->abs_origin( ), g.m_interfaces->globals( )->m_curtime, g.m_interfaces->globals( )->m_tickcount, mask );
		target->UpdateIKLocks( g.m_interfaces->globals( )->m_curtime );
		GetSkeleton( target, hdr, pos, q, mask, g.m_interfaces->globals( )->m_curtime, ipk );
		ipk->UpdateTargets( pos, q, accessor->m_pBones, &computed[ 0 ] );
		target->CalculateIKLocks( g.m_interfaces->globals( )->m_curtime );
		ipk->SolveDependencies( pos, q, accessor->m_pBones, computed );

		// target->DoExtraBoneProcessing(hdr, pos, q, accessor->m_pBones, computed, ipk);

	}
	else 
		return false; 
	// compute and build bones.
	Studio_BuildMatrices( hdr->m_pStudioHdr, target->abs_angles( ), target->abs_origin( ), pos, q, -1, 1, accessor->m_pBones, mask );

	g.m_interfaces->mem_alloc( )->free( computed );
	accessor->m_pBones = backup_matrix;
    target->m_Ipk() = backup;
    target->iEFlags() = backup_eflags;
	return true;
}
bool bones_t::BuildBones( player_t *target, int mask, bone_array_t *out, std::shared_ptr<player_record_t> record, CIKContext *ipk ) {
	alignas(16) vec3_t		     pos[ 128 ];
	alignas(16) quaternion_t     q[ 128 ];
	float            backup_poses[ 24 ];
	animation_layer_t backup_layers[ 15 ];

	// get hdr.

	const auto hdr = target->GetModelPtr( );
	if ( !hdr ) 
		return false;

	// get ptr to bone accessor.
	const auto accessor = &target->GetBoneAccessor( );
	if ( !accessor ) 
		return false;
	// store origial output matrix.
	// likely cachedbonedata.
	const auto backup_matrix = accessor->m_pBones;
	if ( !backup_matrix )
        return false;


	const auto curtime = g.m_interfaces->globals( )->m_curtime;
	const auto frametime = g.m_interfaces->globals( )->m_frametime;

	g.m_interfaces->globals( )->m_curtime = record->m_pred_time;
	g.m_interfaces->globals( )->m_frametime = g.m_interfaces->globals( )->m_interval_per_tick;

	// backup original.
	const auto mins = target->mins( ), maxs = target->maxs( );
	const vec3_t backup_origin = target->abs_origin( );
	const ang_t backup_angles = target->abs_angles( );
    const int backup_eflags = target->iEFlags();
	target->GetPoseParameters( backup_poses );
	target->GetAnimLayers( backup_layers );

	// set non interpolated data.
    const auto old_effects = target->m_fEffects();
    target->iEFlags() &= ~0x1000;
    target->iEFlags() &= ~(1 << 11);
    target->iEFlags() |= (1 << 3);
    target->InvalidatePhysicsRecursive(entity_t::ANGLES_CHANGED);
    target->InvalidatePhysicsRecursive(entity_t::ANIMATION_CHANGED);
    target->InvalidatePhysicsRecursive(entity_t::SEQUENCE_CHANGED);
	target->set_abs_origin( record->m_pred_origin );
	target->set_abs_angles( record->m_abs_angles );
	target->SetPoseParameters( record->m_poses );
	target->SetAnimLayers( record->m_layers );

	//target->setup_bones( record->m_render_bones, 128, bone_used_by_anything, record->m_pred_time );

	const auto backup = target->m_Ipk( );
	target->m_Ipk( ) = ipk;
	
	//force game to call AccumulateLayers - pvs fix.
   //auto ipk = target->m_Ipk();
   //if (ipk) {
   //	ipk->ClearTargets();
   //	ipk->New();
   //}

	auto* computed = static_cast< uint8_t* >( g.m_interfaces->mem_alloc( )->alloc( sizeof( uint8_t[ 0x100 ] ) ) );
	std::memset( computed, 0, 0x100 );
	{
		m_running = true;

		// set bone array for write.
		accessor->m_pBones = out;

		//target->StandardBlendingRules(hdr, pos, q, record->m_sim_time, mask);
		if ( ipk ) {
			ipk->Init( hdr, target->abs_angles( ), target->abs_origin( ), record->m_pred_time, g.time_to_ticks( record->m_pred_time ), mask );
			target->UpdateIKLocks( record->m_pred_time );
			GetSkeleton( target, hdr, pos, q, mask, record->m_pred_time, ipk );
			ipk->UpdateTargets( pos, q, accessor->m_pBones, &computed[ 0 ] );
			target->CalculateIKLocks( record->m_pred_time );
			ipk->SolveDependencies( pos, q, accessor->m_pBones, computed );

			// target->DoExtraBoneProcessing(hdr, pos, q, accessor->m_pBones, computed, ipk);

		}
		// compute and build bones.
		Studio_BuildMatrices( hdr->m_pStudioHdr, target->abs_angles( ), target->abs_origin( ), pos, q, -1, 1, accessor->m_pBones, mask );

		accessor->m_pBones = backup_matrix;
	}
	target->m_Ipk( ) = backup;
	// restore original interpolated entity data.
	target->mins( ) = mins;
	target->maxs( ) = maxs;
	target->set_abs_origin( backup_origin );
	target->set_abs_angles( backup_angles );
	target->SetPoseParameters( backup_poses );
	target->SetAnimLayers( backup_layers );

	// revert to old game behavior.
	m_running = false;

	target->m_fEffects( ) = old_effects;
    target->iEFlags() = backup_eflags;
	g.m_interfaces->mem_alloc(  )->free( computed );
	g.m_interfaces->globals( )->m_curtime = curtime;
	g.m_interfaces->globals( )->m_frametime = frametime;
	return true;
}
