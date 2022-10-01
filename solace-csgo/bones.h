#pragma once

#include "vec3.h"
class CStudioHdr;


struct player_record_t;
class bone_array_t;
class player_t;
class CIKContext;

class bones_t {
public:
	bool setup ( player_t *player, bone_array_t *out, std::shared_ptr<player_record_t> record, CIKContext *ipk );
	bool BuildBonesStripped( player_t *target, int mask, bone_array_t *out, CIKContext *ipk );
	bool BuildBones ( player_t *target, int mask, bone_array_t *out, std::shared_ptr<player_record_t> record, CIKContext *ipk );
	bool m_running = false;
} inline g_bones;

