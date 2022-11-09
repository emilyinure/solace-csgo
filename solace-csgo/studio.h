#pragma once

#include "memory_shit.h"
#include "vec3.h"

using rad_euler = float[3];

#define MAX_QPATH  260

#define BONE_CALCULATE_MASK             0x1F
#define BONE_PHYSICALLY_SIMULATED       0x01    // bone is physically simulated when physics are active
#define BONE_PHYSICS_PROCEDURAL         0x02    // procedural when physics is active
#define BONE_ALWAYS_PROCEDURAL          0x04    // bone is always procedurally animated
#define BONE_SCREEN_ALIGN_SPHERE        0x08    // bone aligns to the screen, not constrained in motion.
#define BONE_SCREEN_ALIGN_CYLINDER      0x10    // bone aligns to the screen, constrained by it's own axis.

#define BONE_USED_MASK                  0x0007FF00
#define BONE_USED_BY_ANYTHING           0x0007FF00
#define BONE_USED_BY_HITBOX             0x00000100    // bone (or child) is used by a hit box
#define BONE_USED_BY_ATTACHMENT         0x00000200    // bone (or child) is used by an attachment point
#define BONE_USED_BY_VERTEX_MASK        0x0003FC00
#define BONE_USED_BY_VERTEX_LOD0        0x00000400    // bone (or child) is used by the toplevel model via skinned vertex
#define BONE_USED_BY_VERTEX_LOD1        0x00000800
#define BONE_USED_BY_VERTEX_LOD2        0x00001000
#define BONE_USED_BY_VERTEX_LOD3        0x00002000
#define BONE_USED_BY_VERTEX_LOD4        0x00004000
#define BONE_USED_BY_VERTEX_LOD5        0x00008000
#define BONE_USED_BY_VERTEX_LOD6        0x00010000
#define BONE_USED_BY_VERTEX_LOD7        0x00020000
#define BONE_USED_BY_BONE_MERGE         0x00040000    // bone is available for bone merge to occur against it

#define BONE_USED_BY_VERTEX_AT_LOD(lod) ( BONE_USED_BY_VERTEX_LOD0 << (lod) )
#define BONE_USED_BY_ANYTHING_AT_LOD(lod) ( ( BONE_USED_BY_ANYTHING & ~BONE_USED_BY_VERTEX_MASK ) | BONE_USED_BY_VERTEX_AT_LOD(lod) )

#define MAX_NUM_LODS 8
#define MAXSTUDIOBONES		128		// total bones actually used

#define BONE_TYPE_MASK                  0x00F00000
#define BONE_FIXED_ALIGNMENT            0x00100000    // bone can't spin 360 degrees, all interpolation is normalized around a fixed orientation

#define BONE_HAS_SAVEFRAME_POS          0x00200000    // Vector48
#define BONE_HAS_SAVEFRAME_ROT64        0x00400000    // Quaternion64
#define BONE_HAS_SAVEFRAME_ROT32        0x00800000    // Quaternion32

enum bone_flags {
	bone_calculate_mask = 0x1f,
	bone_physically_simulated = 0x01,
	bone_physics_procedural = 0x02,
	bone_always_procedural = 0x04,
	bone_screen_align_sphere = 0x08,
	bone_screen_align_cylinder = 0x10,
	bone_used_mask = 0x0007ff00,
	bone_used_by_anything = 0x0007ff00,
	bone_used_by_hitbox = 0x00000100,
	bone_used_by_attachment = 0x00000200,
	bone_used_by_vertex_mask = 0x0003fc00,
	bone_used_by_vertex_lod0 = 0x00000400,
	bone_used_by_vertex_lod1 = 0x00000800,
	bone_used_by_vertex_lod2 = 0x00001000,
	bone_used_by_vertex_lod3 = 0x00002000,
	bone_used_by_vertex_lod4 = 0x00004000,
	bone_used_by_vertex_lod5 = 0x00008000,
	bone_used_by_vertex_lod6 = 0x00010000,
	bone_used_by_vertex_lod7 = 0x00020000,
	bone_used_by_bone_merge = 0x00040000,
	bone_type_mask = 0x00f00000,
	bone_fixed_alignment = 0x00100000,
	bone_has_saveframe_pos = 0x00200000,
	bone_has_saveframe_rot = 0x00400000
};

enum hitgroups {
	hitgroup_generic = 0,
	hitgroup_head = 1,
	hitgroup_chest = 2,
	hitgroup_stomach = 3,
	hitgroup_leftarm = 4,
	hitgroup_rightarm = 5,
	hitgroup_leftleg = 6,
	hitgroup_rightleg = 7,
	hitgroup_gear = 10
};

enum modtypes {
	mod_bad = 0,
	mod_brush,
	mod_sprite,
	mod_studio
};

enum hitboxes {
	hitbox_head = 0,
	hitbox_neck,
	hitbox_lower_neck,
	hitbox_pelvis,
	hitbox_body,
	hitbox_thorax,
	hitbox_chest,
	hitbox_upper_chest,
	hitbox_r_thigh,
	hitbox_l_thigh,
	hitbox_r_calf,
	hitbox_l_calf,
	hitbox_r_foot,
	hitbox_l_foot,
	hitbox_r_hand,
	hitbox_l_hand,
	hitbox_r_upper_arm,
	hitbox_r_forearm,
	hitbox_l_upper_arm,
	hitbox_l_forearm,
	hitbox_max
};

struct studio_bone_t {
	int name_index;
	inline char* const name(void) const {
		return ((char*)this) + name_index;
	}
	int parent;
	int bone_controller[6];

	vec3_t pos;
	quaternion_t quat;
	rad_euler rotation;

	vec3_t pos_scale;
	vec3_t rot_scale;

	matrix_t pose_to_bone;
	quaternion_t quat_alignement;
	int flags;
	int proc_type;
	int proc_index;
	mutable int physics_bone;

	inline void* procedure() const {
		if (proc_index == 0) return NULL;
		else return static_cast< void * >(((unsigned char *)this) + proc_index);
	};

	int surface_prop_idx;
	inline char* const surface_prop(void) const {
		return ((char*)this) + surface_prop_idx;
	}
	inline int get_surface_prop(void) const {
		return surf_prop_lookup;
	}

	int contents;
	int surf_prop_lookup;
	int unused[7];
};

struct studio_box_t {
	int     bone;                 // 0x0000
	int     group;                // 0x0004
	vec3_t  mins;                 // 0x0008
	vec3_t  maxs;                 // 0x0014
	int     name_id;				// 0x0020
	ang_t   angle;                // 0x0024
	float   radius;               // 0x0030
	char pad0[ 0x10 ];                    // 0x0034
};

struct studio_hitbox_set_t {
	int name_index;
	int hitbox_count;
	int hitbox_index;

	char *name ( void ) const {
		return ((char*)this) + name_index;
	}

	studio_box_t* hitbox(int i) const {
		return (studio_box_t*)(((unsigned char*)this) + hitbox_index) + i;
	}
};

struct mstudioseqdesc_t;

class studio_hdr_t {
public:
	int id;
	int version;
	long checksum;
	char name_char_array[64];
	int length;
	vec3_t eye_pos;
	vec3_t illium_pos;
	vec3_t hull_mins;
	vec3_t hull_maxs;
	vec3_t mins;
	vec3_t maxs;
	int flags;
	int bones_count;
	int bone_index;
	int bone_controllers_count;
	int bone_controller_index;
	int hitbox_sets_count;
	int hitbox_set_index;
	int local_anim_count;
	int local_anim_index;
	int local_seq_count;
	int local_seq_index;
	int activity_list_version;
	int events_indexed;
	int textures_count;
	int texture_index;
	int					numcdtextures;
	int					cdtextureindex;

	// replaceable textures tables
	int					numskinref;
	int					numskinfamilies;
	int					skinindex;

	int					numbodyparts;
	int					bodypartindex;

	// queryable attachable points
//private:
	int					numlocalattachments;
	int					localattachmentindex;
	int					numlocalnodes;
	int					localnodeindex;
	int					localnodenameindex;
	int					numflexdesc;
	int					flexdescindex;
	int					numflexcontrollers;
	int					flexcontrollerindex;
	int					numflexrules;
	int					flexruleindex;
	int					numikchains;
	int					ikchainindex;
	int					nummouths;
	int					mouthindex;
	int					numlocalposeparameters;
	int					localposeparamindex;

	int					surfacepropindex;

	// Key values
	int					keyvalueindex;
	int					keyvaluesize;

	int					numlocalikautoplaylocks;
	int					localikautoplaylockindex;
	// The collision model mass that jay wanted
	float				mass;
	int					contents;

	// external animations, models, etc.
	int					numincludemodels;
	int					includemodelindex;

	inline mstudioseqdesc_t *pLocalSeqdesc( int i ) const;
	
	studio_hitbox_set_t* hitbox_set(int i) {
		if (i > hitbox_sets_count) return nullptr;
		return (studio_hitbox_set_t*)((uint8_t*)this + hitbox_set_index) + i;
	}
	studio_bone_t* bone(int i) const {
		if (i > bones_count) return nullptr;
		return (studio_bone_t*)((uint8_t*)this + bone_index) + i;
	}

	int boneParent ( int i ) const {
		return bone( i )->parent;
	}

	int boneFlags ( int i ) const {
		return bone( i )->flags;
	}
};

struct mstudioseqdesc_t {
	int					baseptr;

	int					szlabelindex;

	int					szactivitynameindex;

	int					flags;		// looping/non-looping flags

	int					activity;	// initialized at loadtime to game DLL values
	int					actweight;

	int					numevents;
	int					eventindex;

	vec3_t				bbmin;		// per sequence bounding box
	vec3_t				bbmax;

	int					numblends;

	// Index into array of shorts which is groupsize[0] x groupsize[1] in length
	int					animindexindex;

	int					movementindex;	// [blend] float array for blended movement
	int					groupsize[ 2 ];
	int					paramindex[ 2 ];	// X, Y, Z, XR, YR, ZR
	float				paramstart[ 2 ];	// local (0..1) starting value
	float				paramend[ 2 ];	// local (0..1) ending value
	int					paramparent;

	float				fadeintime;		// ideal cross fate in time (0.2 default)
	float				fadeouttime;	// ideal cross fade out time (0.2 default)

	int					localentrynode;		// transition node at entry
	int					localexitnode;		// transition node at exit
	int					nodeflags;		// transition rules

	float				entryphase;		// used to match entry gait
	float				exitphase;		// used to match exit gait

	float				lastframe;		// frame that should generation EndOfSequence

	int					nextseq;		// auto advancing sequences
	int					pose;			// index of delta animation between end and nextseq

	int					numikrules;

	int					numautolayers;	//
	int					autolayerindex;

	int					weightlistindex;

	// FIXME: make this 2D instead of 2x1D arrays
	int					posekeyindex;

	int					numiklocks;
	int					iklockindex;

	// Key values
	int					keyvalueindex;
	int					keyvaluesize;

	int					cycleposeindex;		// index of pose parameter to use as cycle index

	int					activitymodifierindex;
	int					numactivitymodifiers;

	int					animtagindex;
	int					numanimtags;

	int					rootDriverIndex;

	int					unused[ 2 ];		// remove/add as appropriate (grow back to 8 ints on version change!)
};

inline mstudioseqdesc_t *studio_hdr_t::pLocalSeqdesc( int i ) const { if ( i < 0 || i >= local_seq_count ) i = 0; return reinterpret_cast< mstudioseqdesc_t * >(reinterpret_cast< uintptr_t >(this) + local_seq_index) + i; };


struct virtualsequence_t {
	int	flags;
	int activity;
	int group;
	int index;
};
struct virtualmodel_t {
	uint8_t pad[ 128 ];
	CUtlVector< virtualsequence_t > m_seq;
};


class CStudioHdr {
public:
	class mstudioposeparamdesc_t {
	public:
		int					sznameindex;
		[[nodiscard]] __forceinline char *const name( void ) const { return ( ( char * )this ) + sznameindex; }
		int					flags;	// ????
		float				start;	// starting value
		float				end;	// ending value
		float				loop;	// looping range, 0 for no looping, 360 for rotations, etc.
	};

	int GetNumSeq_Internal( void ) const {
		return m_pVModel->m_seq.Count( );
	}
	inline int			GetNumSeq( void ) const {
		if ( !m_pVModel )
			return m_pStudioHdr->local_seq_count;
		return GetNumSeq_Internal( );
	}

	const studio_hdr_t *GroupStudioHdr ( int i );

	mstudioseqdesc_t &pSeqdesc_Internal( int i ) {
		if ( i < 0 || i >= GetNumSeq( ) ) {
			// Avoid reading random memory.
			i = 0;
		}

		const studio_hdr_t *pStudioHdr = GroupStudioHdr( m_pVModel->m_seq[ i ].group );

		return *pStudioHdr->pLocalSeqdesc( m_pVModel->m_seq[ i ].index );
	}
	inline mstudioseqdesc_t &pSeqdesc( int iSequence ) {
		if ( !m_pVModel )
			return *m_pStudioHdr->pLocalSeqdesc( iSequence );

		return pSeqdesc_Internal( iSequence );
	}
	studio_hdr_t *m_pStudioHdr;
	virtualmodel_t *m_pVModel;
};

struct CIKTarget {
	int m_iFrameCount;

private:
	char pad_00004[ 0x51 ];
};


class CBoneMergeCache {
public:
	char pad[ 676 ] = {};
	CBoneMergeCache ( );
	static CBoneMergeCache *init( void * );
	void UpdateCache( );
	void MergeMatchingPoseParams ( );
	void CopyFromFollow ( vec3_t * pos, quaternion_t * quaternion, int i, vec3_t vec3s[256], quaternion_t quaternions[256] );
	void CopyToFollow ( vec3_t vec3s[256], quaternion_t quaternions[256], int i, vec3_t * pos, quaternion_t * quaternion );

};

class CIKContext {
public:
	char pad[ 4208 ] = {};
	// You need to use this constructor, because you need to allocate memory using g_pMemAlloc exported by tier0
	//void  operator delete( void *ptr );
	void init( );

	void unk_1 ( );

	void unk_2 ( );

	// This somehow got inlined so we need to rebuild it
	void ClearTargets ( );
	void Init ( CStudioHdr *hdr, ang_t angles, vec3_t origin, float currentTime, int frames, int boneMask );
	void UpdateTargets ( vec3_t *pos, quaternion_t *qua, matrix_t *matrix, uint8_t *boneComputed );
	void SolveDependencies ( vec3_t *pos, quaternion_t *qua, matrix_t *matrix, uint8_t *boneComputed );
	void CopyTo ( CIKContext *ik, int p );

	void AddDependencies ( mstudioseqdesc_t &seqdesc, int iSequence, float flCycle, const float poseParameters[],
	                       float flWeight );
};