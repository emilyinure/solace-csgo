#include "penetration.h"


#include "includes.h"
#include "thread_handler.h"

using IsBreakableEntity_t = bool( __thiscall * )( entity_t * );
bool penetration::IsBreakable( entity_t *ent ) {
    bool        ret;
    c_client_class *cc;
    const char *name;
    char *takedmg, old_takedmg;

    static auto IsBreakableEntity = ( IsBreakableEntity_t )util::find( "client.dll", "55 8B EC 51 56 8B F1 85 F6 74 68 83 BE" );

    static size_t m_takedamage_offset{ *( size_t * )( ( uintptr_t )IsBreakableEntity + 38 ) };

    // skip null ents and the world ent.
    if ( !ent || ent->index( ) == 0 )
        return false;

    // get m_takedamage and save old m_takedamage.
    takedmg = ( char * )( ( uintptr_t )ent + m_takedamage_offset );
    old_takedmg = *takedmg;

    // get clientclass.
    cc = ent->networkable( )->client_class(  );

    if ( cc ) {
        // get clientclass network name.
        name = cc->m_pNetworkName;

        // CBreakableSurface, CBaseDoor, ...
        if ( name[ 1 ] != 'F'
             || name[ 4 ] != 'c'
             || name[ 5 ] != 'B'
             || name[ 9 ] != 'h' ) {
            *takedmg = 2;
        }
    }

    ret = IsBreakableEntity( ent );
    *takedmg = old_takedmg;

    return ret;
}
static bool UTIL_ClipTraceToPlayers( const vec3_t &vecAbsStart, const vec3_t &vecAbsEnd, unsigned int mask, trace_filter *filter, trace_t *tr, float range ) {
    static auto clptrtp = util::find( "client.dll", "E8 ? ? ? ? 83 C4 14 8A 56 37" ) + 0x1;

    if ( !clptrtp )
        return false;

    __asm {
        mov  ecx, vecAbsStart
        mov	 edx, vecAbsEnd
        push range
        push tr
        push filter
        push mask
        call clptrtp
        add	 esp, 16
    }
    return true;
}

float penetration::scale( player_t *player, float damage, float armor_ratio, int hitgroup ) {
    bool  has_heavy_armor;
    int   armor;
    float heavy_ratio, bonus_ratio, ratio, new_damage;

    static auto is_armored = [ ]( player_t *player, int armor, int hitgroup ) {
        // the player has no armor.
        if ( armor <= 0 )
            return false;

        // if the hitgroup is head and the player has a helment, return true.
        // otherwise only return true if the hitgroup is not generic / legs / gear.
        if ( hitgroup == hitgroup_head && player->has_helment( ) )
            return true;

        else if ( hitgroup >= hitgroup_chest && hitgroup <= hitgroup_rightarm )
            return true;

        return false;
    };

    // check if the player has heavy armor, this is only really used in operation stuff.
    has_heavy_armor = player->heavy_armor( );

    // scale damage based on hitgroup.
    switch ( hitgroup ) {
    case hitgroup_head:
        if ( has_heavy_armor )
            damage = ( damage * 4.f ) * 0.5f;
        else
            damage *= 4.f;
        break;

    case hitgroup_stomach:
        damage *= 1.25f;
        break;

    case hitgroup_leftleg:
    case hitgroup_rightleg:
        damage *= 0.75f;
        break;

    default:
        break;
    }

    // grab amount of player armor.
    armor = player->armor( );

    // check if the ent is armored and scale damage based on armor.
    if ( is_armored( player, armor, hitgroup ) ) {
        heavy_ratio = 1.f;
        bonus_ratio = 0.5f;
        ratio = armor_ratio * 0.5f;

        // player has heavy armor.
        if ( has_heavy_armor ) {
            // calculate ratio values.
            bonus_ratio = 0.33f;
            ratio = armor_ratio * 0.25f;
            heavy_ratio = 0.33f;

            // calculate new damage.
            new_damage = ( damage * ratio ) * 0.85f;
        }

        // no heavy armor, do normal damage calculation.
        else
            new_damage = damage * ratio;

        if ( ( ( damage - new_damage ) * ( heavy_ratio * bonus_ratio ) ) > armor )
            new_damage = damage - ( armor / bonus_ratio );

        damage = new_damage;
    }

    return std::floor( damage );
}

bool penetration::TraceToExit( vec3_t &start, const vec3_t &dir, vec3_t &out, trace_t *enter_trace, trace_t *exit_trace ) {
    static trace_filter filter{};

    float  dist{};
    vec3_t new_end;
    int    contents, first_contents{};

    // max pen distance is 90 units.
    while ( dist <= 90.f ) {
        // step forward a bit.
        dist += 4.f;

        // set out pos.
        out = start + ( dir * dist );

        {
            if ( !first_contents )
                first_contents = g.m_interfaces->trace( )->get_point_contents( out, MASK_SHOT, nullptr );

            contents = g.m_interfaces->trace( )->get_point_contents( out, MASK_SHOT, nullptr );

            if ( ( contents & MASK_SHOT_HULL ) && ( !( contents & CONTENTS_HITBOX ) || ( contents == first_contents ) ) )
                continue;

            // move end pos a bit for tracing.
            new_end = out - ( dir * 4.f );

            // do first trace aHR0cHM6Ly9zdGVhbWNvbW11bml0eS5jb20vaWQvc2ltcGxlcmVhbGlzdGlj.
            g.m_interfaces->trace( )->trace_ray( ray_t( out, new_end ), MASK_SHOT, nullptr, exit_trace );


            // note - dex; this is some new stuff added sometime around late 2017 ( 10.31.2017 update? ).
            //static auto sv_clip_penetration_traces_to_players = g.m_interfaces->console(  )->get_convar( "sv_clip_penetration_traces_to_players" );
            //if ( sv_clip_penetration_traces_to_players->GetBool( ) )
            //    UTIL_ClipTraceToPlayers( out, new_end, MASK_SHOT, nullptr, exit_trace, -60.f );

            // we hit an ent's hitbox, do another trace.
            if ( exit_trace->startSolid && ( exit_trace->surface.flags & SURF_HITBOX ) ) {
                filter.skip = exit_trace->entity;
                g.m_interfaces->trace( )->trace_ray( ray_t( out, start ), MASK_SHOT_HULL, &filter, exit_trace );

                if ( exit_trace->did_hit( ) && !exit_trace->startSolid ) {
                    out = exit_trace->end;
                    return true;
                }

                continue;
            }
        }

        if ( !exit_trace->did_hit( ) || exit_trace->startSolid ) {
            if ( IsBreakable( enter_trace->entity ) ) {
                *exit_trace = *enter_trace;
                exit_trace->end = start + dir;
                return true;
            }

            continue;
        }

        if ( ( exit_trace->surface.flags & SURF_NODRAW ) ) {
            // note - dex; ok, when this happens the game seems to not ignore world?
            if ( IsBreakable( exit_trace->entity ) && IsBreakable( enter_trace->entity ) ) {
                out = exit_trace->end;
                return true;
            }

            if ( !( enter_trace->surface.flags & SURF_NODRAW ) )
                continue;
        }

        if ( exit_trace->plane.normal.dot( dir ) <= 1.f ) {
            out -= ( dir * ( exit_trace->flFraction * 4.f ) );
            return true;
        }
    }

    return false;
}

void penetration::ClipTraceToPlayer( vec3_t &start, const vec3_t &end, uint32_t mask, trace_t *tr, player_t *player, float min ) {
    vec3_t     pos, to, dir, on_ray;
    float      len, range_along, range;
    ray_t        ray;
    trace_t new_trace;

    // reference: https://github.com/alliedmodders/hl2sdk/blob/3957adff10fe20d38a62fa8c018340bf2618742b/game/shared/util_shared.h#L381

    // set some local vars.
    pos = player->origin( ) + ( ( player->mins( ) + player->maxs( ) ) * 0.5f );
    to = pos - start;
    dir = start - end;
    len = dir.length( );
    dir /= len;
    range_along = dir.dot( to );

    // off start point.
    if ( range_along < 0.f )
        range = -( to ).length( );

    // off end point.
    else if ( range_along > len )
        range = -( pos - end ).length( );

    // within ray bounds.
    else {
        on_ray = start + ( dir * range_along );
        range = ( pos - on_ray ).length( );
    }

    if ( /*min <= range &&*/ range <= 60.f ) {
        // clip to player.
        g.m_interfaces->trace(  )->clip_ray_to_entity( ray_t( start, end ), mask, player, &new_trace );

        if ( tr->flFraction > new_trace.flFraction )
            *tr = new_trace;
    }
}

bool penetration::run( PenetrationInput_t *in, PenetrationOutput_t *out ) {

    int			  pen{ 4 }, enter_material, exit_material;
    float		  damage, penetration, penetration_mod, player_damage, remaining, trace_len{}, total_pen_mod, damage_mod = 0.5f, modifier, damage_lost;
    surfacedata_t *enter_surface, *exit_surface;
    bool		  nodraw, grate;
    vec3_t		  start, dir, end, pen_end;
    trace_t	  trace, exit_trace;
    weapon_info_t *weapon_info;

    // if we are tracing from our local player perspective.
    if ( in->m_from == g.m_local ) {
        weapon_info = g.m_weapon_info;
        start = in->m_start;
    }

    // not local player.
    else {
        weapon_t *weapon;
        weapon = static_cast< weapon_t * >( g.m_interfaces->entity_list( )->get_client_entity_handle( in->m_from->active_weapon( ) ) );
        if ( !weapon )
            return false;

        // get weapon info.
        weapon_info = g.m_interfaces->weapon_system( )->get_weapon_data( weapon->item_definition_index( ) );;
        if ( !weapon_info )
            return false;

        // set trace start.
        if ( !in->m_resolving )
            start = g.m_shoot_pos;
        else
            start = in->m_start;
    }
    if ( !weapon_info )
        return false;
    // get some weapon data.
    float fPenetrationPower = 35;
    float flPenetrationDistance = 3000.0;
    damage = static_cast< float >(weapon_info->m_damage);
    penetration = weapon_info->m_penetration;
    float flCurrentDistance = 0.f;
    // used later in calculations.
    penetration_mod = fmaxf( 0.f, ( 3.f / penetration ) * 1.25f );

    // get direction to end point.
    dir = ( in->m_pos - start );
    dir /= dir.length( );

    auto *studio_model = g.m_interfaces->model_info( )->get_studio_model( in->m_target->model( ) );
    if ( !studio_model )
        return false;

    float flTraceDistance = 0;

    float flPenMod = 0;

    float flPercentDamageChunk = 0;
    float flPenWepMod = 0;

    float flLostDamageObject = 0;
    float lost = 0;

    // setup trace filter for later.
    static trace_filter filter; filter.skip = ( in->m_from );

    while ( true ) {
        // calculating remaining len.
        remaining = weapon_info->m_range - flCurrentDistance;

        // set trace end.
        end = start + ( dir * remaining );

        // setup ray and trace.
        // TODO; use UTIL_TraceLineIgnoreTwoEntities?
        {
            g.m_interfaces->trace( )->trace_ray( ray_t( start, end ), MASK_SHOT, &filter, &trace );


            // check for player hitboxes extending outside their collision bounds.
            // if no target is passed we clip the trace to a specific player, otherwise we clip the trace to any player.

            if ( in->m_target ) {
                ClipTraceToPlayer( start, end + ( dir * 40.f ), MASK_SHOT, &trace, in->m_target, -60.f );
            }
            else
                return false;// UTIL_ClipTraceToPlayers( start, end + ( dir * 40.f ), MASK_SHOT, ( trace_filter * )&filter, &trace, -60.f );
        }


        // calculate damage based on the distance the bullet traveled.
        flCurrentDistance += trace.flFraction * remaining;
        damage *= std::pow( weapon_info->m_range_modifier, flCurrentDistance / 500.f );
    	
        // we didn't hit anything.
        if ( trace.flFraction == 1.f ) {
            if ( in->m_simulated_shot ) {
                //reached end point of trace, calculate damage as if it were a headshot
                scale( in->m_target, damage, weapon_info->m_armor_ratio, hitgroup_head );
                return true;
            }

            return false;
        }

        // if a target was passed.
        if ( in->m_target ) {

            // validate that we hit the target we aimed for.
            if ( trace.entity && trace.entity == in->m_target ) {
                if ( damage < 0 ) {
                    out->m_damage = damage;
                    return false;
                }

                // scale damage based on the hitgroup we hit.
                player_damage = scale( in->m_target, damage, weapon_info->m_armor_ratio, in->m_simulated_shot ? hitgroup_head : trace.hitGroup );

                // set result data for when we hit a player.
                out->m_pen = pen != 4;
                out->m_hitgroup = trace.hitGroup;
                out->m_damage = player_damage;
                out->m_target = in->m_target;

                // non-penetrate damage.
                if ( pen == 4 )
                    return player_damage >= in->m_damage;

                // penetration damage.
                return player_damage >= in->m_damage_pen;
            }
        }

        // no target was passed, check for any player hit or just get final damage done.
        else {
            out->m_pen = pen != 4;

            // todo - dex; team checks / other checks / etc.
            if ( trace.entity && trace.entity->is_player( ) ) {
                if ( damage < 0 ) {
                    out->m_damage = damage;
                    return false;
                }

                player_damage = scale( static_cast< player_t * >(trace.entity), damage, weapon_info->m_armor_ratio, in->m_simulated_shot ? hitgroup_head : trace.hitGroup );

                // set result data for when we hit a player.
                out->m_hitgroup = trace.hitGroup;
                out->m_damage = player_damage;
                out->m_target = static_cast< player_t * >(trace.entity);

                // non-penetrate damage.
                if ( pen == 4 )
                    return player_damage >= in->m_damage;

                // penetration damage.
                return player_damage >= in->m_damage_pen;
            }

            // if we've reached here then we didn't hit a player yet, set damage and hitgroup.
            out->m_damage = damage;
        }

        // don't run pen code if it's not wanted.
        if ( !in->m_can_pen )
            return false;

        // get surface at entry point.
        enter_surface = g.m_interfaces->phys_surface(  )->GetSurfaceData( trace.surface.surfaceProps );

        // this happens when we're too far away from a surface and can penetrate walls or the surface's pen modifier is too low.
        if ( ( flCurrentDistance > flPenetrationDistance && penetration > 0.f ) || enter_surface->game.m_penetration_modifier < 0.1f ) {
            return false;
        }

        // store data about surface flags / contents.
        nodraw = ( trace.surface.flags & SURF_NODRAW );
        grate = ( trace.contents & CONTENTS_GRATE );

        // get material at entry point.
        enter_material = enter_surface->game.m_material;

        // note - dex; some extra stuff the game does.
        if ( !pen && !nodraw && !grate && enter_material != CHAR_TEX_GRATE && enter_material != CHAR_TEX_GLASS )
            return false;

        // no more pen.
        if ( penetration <= 0.f || pen <= 0 ) {
            return false;
        }

        // try to penetrate object.
        if ( !TraceToExit( trace.end, dir, pen_end, &trace, &exit_trace ) ) {
            if ( !( g.m_interfaces->trace( )->get_point_contents( pen_end, MASK_SHOT_HULL ) & MASK_SHOT_HULL ) ) {
                return false;
            }
        }

        // get surface / material at exit point.
        exit_surface = g.m_interfaces->phys_surface(  )->GetSurfaceData( exit_trace.surface.surfaceProps );
        exit_material = exit_surface->game.m_material;

        // todo - dex; check for CHAR_TEX_FLESH and ff_damage_bullet_penetration / ff_damage_reduction_bullets convars?
        //             also need to check !isbasecombatweapon too.
        float flDamLostPercent = 0.16;
        if ( enter_material == CHAR_TEX_GRATE || enter_material == CHAR_TEX_GLASS ) {
            total_pen_mod = 3.f;
            flDamLostPercent = 0.05;//damage_mod = 0.05f;
        }

        else if ( nodraw || grate ) {
            total_pen_mod = 1.f;
            flDamLostPercent = 0.16f;
            damage_mod = 0.99f;
        }

        else {
            total_pen_mod = ( enter_surface->game.m_penetration_modifier + exit_surface->game.m_penetration_modifier ) * 0.5f;
            flDamLostPercent = 0.16f;
            damage_mod = ( damage_mod + exit_surface->game.m_damage_modifier ) / 2;
        }

        // thin metals, wood and plastic get a penetration bonus.
        if ( enter_material == exit_material ) {
            if ( exit_material == CHAR_TEX_CARDBOARD || exit_material == CHAR_TEX_WOOD )
                total_pen_mod = 3.f;

            else if ( exit_material == CHAR_TEX_PLASTIC )
                total_pen_mod = 2.f;
        }

        // set some local vars.
        flTraceDistance = ( exit_trace.end - trace.end ).length();

        flPenMod = fmaxf( 0, ( 1 / total_pen_mod ) );

        flPercentDamageChunk = damage * flDamLostPercent;
        flPenWepMod = flPercentDamageChunk + fmaxf( 0.f,  3.f / fPenetrationPower ) * 1.25f * ( flPenMod * 3.0f );

        flLostDamageObject = ( ( flPenMod * ( flTraceDistance * flTraceDistance ) ) / 24 );
        lost = flPenWepMod + flLostDamageObject;
        //flCurrentDistance += flTraceDistance;
        //trace_len = ( exit_trace.end - trace.end ).length( );
        //modifier = fmaxf( 0.f, 1.f / total_pen_mod );
        //damage_lost = ( ( modifier * 3.f ) * penetration_mod + ( damage * damage_mod ) ) + ( ( ( trace_len * trace_len ) * modifier ) / 24.f );

        // subtract from damage.
        damage -= fmaxf( 0.f, lost );
        if ( damage < 1.f )
            return false;

        // set new start pos for successive trace.
        start = exit_trace.end;

        // decrement pen.
        --pen;
    }

    return false;
}