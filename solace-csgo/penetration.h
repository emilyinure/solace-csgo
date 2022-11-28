#pragma once

#include "vec3.h"
struct trace_t;
class player_t;
class entity_t;

namespace penetration {
    struct PenetrationInput_t {
        player_t *m_from = nullptr;
        player_t *m_target = nullptr;
        vec3_t  m_pos;
        float	m_damage;
        float   m_damage_pen;
        bool	m_can_pen = true;
        int m_group = -1;
        vec3_t m_start = vec3_t(0,0,0);
        bool m_resolving = false;
        bool m_simulated_shot = false;
        int m_hitgroup = 0;
    };

    struct PenetrationOutput_t {
        player_t *m_target = nullptr;
        float   m_damage = 0;
        int     m_hitgroup = -1;
        bool    m_pen = false;

        __forceinline PenetrationOutput_t( ) = default;
    };

    bool IsBreakable( entity_t* ent );
    float scale( player_t *player, float damage, float armor_ratio, int hitgroup );
    bool  TraceToExit( vec3_t &start, const vec3_t &dir, vec3_t &out, trace_t *enter_trace, trace_t *exit_trace );
    void  ClipTraceToPlayer( vec3_t &start, const vec3_t &end, uint32_t mask, trace_t *tr, player_t *player, float min );
    bool  run( PenetrationInput_t *in, PenetrationOutput_t *out );
}