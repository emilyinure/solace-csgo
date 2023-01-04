#include "movement.h"

#include "includes.h"
#include "prediction.h"
#include "predictioncopy.h"
#include "render.h"

void movement::draw()
{
    if (!g.m_running_client)
        return;

    // auto s = std::to_string(jump_count);
    //
    // g.m_render->text(g.m_render->m_tahoma_14(), 960, 540, color(255, 255, 255),
    // s.c_str(), 2);

    if (settings::hvh::antiaim::auto_peek)
        m_time_left += g.m_interfaces->globals()->m_abs_frametime;
    else
        m_time_left -= g.m_interfaces->globals()->m_abs_frametime;
    m_time_left = fminf(0.5f, fmaxf(m_time_left, 0.f));
    const auto radius = 17.f * (m_time_left / 0.5f);
    if (radius > 0)
    {
        const auto origin = m_stop_pos;
        vec3_t origin_w2s;

        if (!math::world_to_screen(origin, origin_w2s))
            return;

        render_t::vertex_t verts[3] = {};
        auto point_color = color{0xFF, 0x08, 0xFF};
        for (auto i = 0; i < 360; i += 5)
        {
            auto rot = origin + ang_t(0, i, 0).forward() * radius;
            auto rot_2 = origin + ang_t(0, i + 5.f, 0).forward() * radius;

            vec3_t point_wts;
            vec3_t point_wts_2;

            if (!math::world_to_screen(rot, point_wts))
                continue;
            if (!math::world_to_screen(rot_2, point_wts_2))
                continue;

            point_color.set_a(20);
            verts[0] = {roundf(point_wts.x), roundf(point_wts.y), 0, 1, point_color};
            verts[1] = {roundf(point_wts_2.x), roundf(point_wts_2.y), 0, 1, point_color};
            point_color.set_a(100);
            verts[2] = {roundf(origin_w2s.x), roundf(origin_w2s.y), 0, 1, point_color};
            g.m_render->render_triangle(verts, 1);
            point_color.set_a(50);
            g.m_render->line(roundf(point_wts.x), roundf(point_wts.y), roundf(point_wts_2.x), roundf(point_wts_2.y),
                             point_color, 1);
        }
    }
}

void movement::bhop()
{
    if (!settings::misc::movement::bhop)
        return;
    if (!(g.m_cmd->m_buttons & IN_JUMP))
        return;
    if (g.m_interfaces->entity_list()->get_client_entity_handle(g.m_local->m_ground_entity()))
        return;

    g.m_cmd->m_buttons &= ~IN_JUMP;
}

void movement::QuickStop()
{
    if (!m_should_stop)
        return;

    set_should_stop(false);

    if (!g.m_interfaces->entity_list()->get_client_entity_handle(g.m_local->m_ground_entity()))
        return;

    static auto sv_friction = g.m_interfaces->console()->get_convar("sv_friction");
    static auto sv_stopspeed = g.m_interfaces->console()->get_convar("sv_stopspeed");
    const auto friction = sv_friction->GetFloat() * g.m_local->surface_friction();

    const auto speed = g.m_local->velocity().length();
    // calculate speed.

    if (speed <= 0.1f)
    {
        g.m_cmd->m_forwardmove = 0.f;
        g.m_cmd->m_sidemove = 0.f;
        return;
    }

    // bleed off some speed, but if we have less than the bleed, threshold, bleed
    // the threshold amount.
    const auto control = fmaxf(speed, sv_stopspeed->GetFloat());

    // calculate the drop amount.
    const auto drop = control * friction * g.m_interfaces->globals()->m_interval_per_tick;

    // scale the velocity.
    const auto newspeed = fmaxf(0.f, speed - drop);

    g.m_view_angles = g.m_local->velocity().look(vec3_t());

    if (newspeed > 0.1f)
    {
        g.m_cmd->m_forwardmove = newspeed;
        g.m_cmd->m_sidemove = 0.f;
    }
    else
    {
        g.m_cmd->m_forwardmove = 0.f;
        g.m_cmd->m_sidemove = 0.f;
    }
}

void movement::PreciseMove()
{
    if (!g.m_interfaces->entity_list()->get_client_entity_handle(g.m_local->m_ground_entity()) ||
        settings::hvh::antiaim::fakewalk)
        return;
    if (g.m_cmd->m_buttons & IN_JUMP)
        return;
    if (g.m_cmd->m_forwardmove != 0.f || g.m_cmd->m_sidemove != 0.f)
        return;
    static auto sv_friction = g.m_interfaces->console()->get_convar("sv_friction");
    static auto sv_stopspeed = g.m_interfaces->console()->get_convar("sv_stopspeed");
    const auto friction = sv_friction->GetFloat() * g.m_local->surface_friction();

    const auto speed = g.m_local->velocity().length();
    // calculate speed.

    if (speed <= 0.1f)
    {
        g.m_cmd->m_forwardmove = 0.f;
        g.m_cmd->m_sidemove = 0.f;
        return;
    }

    // bleed off some speed, but if we have less than the bleed, threshold, bleed
    // the threshold amount.
    const auto control = fmaxf(speed, sv_stopspeed->GetFloat());

    // calculate the drop amount.
    const auto drop = control * friction * g.m_interfaces->globals()->m_interval_per_tick;

    // scale the velocity.
    const auto newspeed = fmaxf(0.f, speed - drop);
    g.m_view_angles = g.m_local->velocity().look(vec3_t());

    if (newspeed > 0.1f)
    {
        g.m_cmd->m_forwardmove = newspeed;
        g.m_cmd->m_sidemove = 0.f;
    }
    else
    {
        g.m_cmd->m_forwardmove = 0.f;
        g.m_cmd->m_sidemove = 0.f;
    }
}

void movement::auto_peek()
{
    // set to invert if we press the button.
    static auto last = false;
    if (settings::hvh::antiaim::auto_peek)
    {
        if (last != true)
        {
            m_stop_pos = g.m_local->origin();
            m_time_left = 0;
        }

        if (m_should_unpeek)
            m_invert = true;

        if (m_invert)
        {
            move_to(m_stop_pos);
            set_should_stop(false);
        }
    }

    else
    {
        m_invert = false;
    }
    set_should_unpeek(false);
    last = settings::hvh::antiaim::auto_peek;
}

void movement::move_to(vec3_t target_origin) const
{
    const auto local_origin = g.m_local->origin();
    static auto sv_friction = g.m_interfaces->console()->get_convar("sv_friction");
    static auto sv_stopspeed = g.m_interfaces->console()->get_convar("sv_stopspeed");

    auto velocity = g.m_local->velocity();
    auto speed = velocity.length_2d();

    if (speed > 0.1f)
    {
        const auto friction = sv_friction->GetFloat() * g.m_local->surface_friction();
        auto stop_speed = std::max<float>(speed, sv_stopspeed->GetFloat());
        auto control = fminf(speed, stop_speed);
        auto drop = control * friction * g.m_interfaces->globals()->m_interval_per_tick;
        auto newspeed = speed - drop;
        if (newspeed < 0)
            newspeed = 0;

        newspeed /= speed;
        velocity *= newspeed;
    }

    auto local_delta = (target_origin - local_origin) / g.m_interfaces->globals()->m_interval_per_tick;
    local_delta -= velocity;
    local_delta.z = 0.f;

    // if (local_delta.length_2d() > 0.1f) {
    //	drop = sv_stopspeed->GetFloat() * friction *
    // g.m_interfaces->globals()->m_interval_per_tick; 	newspeed = speed + drop;
    //
    //	local_delta *= newspeed / speed;
    //}

    static auto sv_accelerate = g.m_interfaces->console()->get_convar("sv_accelerate");

    float max_accel = 450;
    auto accel_speed = sv_accelerate->GetFloat() * g.m_interfaces->globals()->m_interval_per_tick * 450;
    if (max_accel > accel_speed)
        max_accel = accel_speed;

    auto current_speed = velocity.dot(local_delta.normalized());

    auto projected_delta = fminf(450.f, local_delta.length_2d() + current_speed);

    if (g.m_weapon_info && g.m_weapon_info->m_max_player_speed > 0.f)
    {
        const auto fRatio = projected_delta / g.m_weapon_info->m_max_player_speed;
        if (fRatio > 0.f)
            projected_delta /= fRatio;
    }

    g.m_view_angles = ang_t(0, static_cast<float>(atan2(local_delta.y, local_delta.x) * (180.f / M_PI)), 0);
    ;

    const auto add_speed = projected_delta - current_speed;
    if (max_accel <= add_speed)
    {
        // distance is farther than we can account for
        // go as fast as we can
        g.m_cmd->m_forwardmove = 450;
        g.m_cmd->m_sidemove = 0;
    }
    else
    {
        // const float kAccelerationScale = MAX( 250.0f, wishspeed );
        // accelspeed = accel * gpGlobals->frametime * kAccelerationScale *
        // player->m_surfaceFriction; accel_speed = sv_accelerate->GetFloat( ) *
        // g.m_interfaces->globals( )->m_interval_per_tick * fmaxf( 250,
        // projected_delta );

        if (accel_speed <= add_speed)
        {
            // add speed is too high, try to correct the accelspeed
            projected_delta =
                projected_delta / (sv_accelerate->GetFloat() * g.m_interfaces->globals()->m_interval_per_tick);
        }
        g.m_cmd->m_forwardmove = std::clamp<float>(projected_delta, -450.f, 450.f);
        g.m_cmd->m_sidemove = 0.f;
    }
}

bool bCheck() // checks for edgebug
{
    if (g.m_local->velocity().z >= -7.f && floorf(g.m_local->velocity().z) != 7.f &&
        !(g.m_local->flags() & fl_onground))
        return true;
    else
        return false;
}

void movement::edge_bug()
{
    // return;
    auto* map = g.m_local->GetPredDescMap();

    if (map)
    {
        const auto size = max(map->m_packed_size, 4);
        static auto startdata = new byte[size];
        auto CopyHelper =
            CPredictionCopy(PC_EVERYTHING, static_cast<byte*>(startdata), true,
                            reinterpret_cast<const byte*>(g.m_local), false, CPredictionCopy::TRANSFERDATA_COPYONLY);
        CopyHelper.TransferData("edgebug_pre", g.m_local->index(), map);
        float backup_fmove = g.m_cmd->m_forwardmove;
        float backup_smove = g.m_cmd->m_sidemove;
        bool hit = false;

        static bool found = false;
        if (found)
        {
            if (bCheck() || g.m_local->flags() & fl_onground)
                found = false;
            else
            {
                g.m_cmd->m_forwardmove = g.m_cmd->m_sidemove = 0.f;
            }
        }

        if (!found)
        {
            g.m_cmd->m_forwardmove = g.m_cmd->m_sidemove = 0.f;
            bool ran = false;
            vec3_t original_orig = g.m_local->origin();
            for (auto i = 0; i < 1 / (g.m_interfaces->globals()->m_interval_per_tick / 10.f); i++)
            {
                if (g.m_local->flags() & fl_onground)
                {
                    break;
                }
                ran = true;
                prediction::start(g.m_cmd);
                if (bCheck() && original_orig.z > g.m_local->origin().z)
                {
                    hit = true;
                    found = true;
                    break;
                }
            }
            if (!hit)
            {
                g.m_cmd->m_forwardmove = backup_fmove;
                g.m_cmd->m_sidemove = backup_smove;
            }
            if (ran)
                prediction::end();
        }

        CopyHelper = CPredictionCopy(PC_EVERYTHING, reinterpret_cast<byte*>(g.m_local), false,
                                     static_cast<const byte*>(startdata), true, CPredictionCopy::TRANSFERDATA_COPYONLY);
        CopyHelper.TransferData("edgebug_post", g.m_local->index(), map);
    }
}

void movement::auto_strafe()
{
    if (settings::misc::movement::autostrafe == 0)
        return;

    if (!(g.m_cmd->m_buttons & IN_JUMP) && !m_force_strafe)
        return;

    m_force_strafe = false; // this will be reset every tick anyways

    if (g.m_interfaces->entity_list()->get_client_entity_handle(g.m_local->m_ground_entity()))
        return;

    const auto velocity = g.m_local->velocity();

    const float speed = velocity.length_2d();

    // compute the ideal strafe angle for our velocity.
    const float ideal = (speed > 0.f) ? RAD2DEG(std::asin(15.f / speed)) : 90.f;
    const float ideal2 = (speed > 0.f) ? RAD2DEG(std::asin(30.f / speed)) : 90.f;

    m_switch *= -1;

    auto direction = vec3_t();
    if (g.m_cmd->m_sidemove == 0 && g.m_cmd->m_forwardmove == 0)
        direction += vec3_t(1, 0, 0);

    if (g.m_cmd->m_forwardmove < 0)
        direction += vec3_t(-1, 0, 0);
    else if (g.m_cmd->m_forwardmove > 0)
        direction += vec3_t(1, 0, 0);

    if (g.m_cmd->m_sidemove > 0)
        direction += vec3_t(0, -1, 0);
    else if (g.m_cmd->m_sidemove < 0)
        direction += vec3_t(0, 1, 0);

    g.m_view_angles.y += static_cast<float>(atan2(direction.y, direction.x) * (180.f / M_PI));

    const auto delta = math::normalize_angle(g.m_view_angles.y - m_old_yaw, 180);

    // convert to absolute change.
    const auto abs_delta = std::abs(delta);

    g.m_cmd->m_sidemove = 0;
    g.m_cmd->m_forwardmove = 0;
    // set strafe direction based on mouse direction change.
    if (delta > 0.f)
        g.m_cmd->m_sidemove = -450.f;

    else if (delta < 0.f)
        g.m_cmd->m_sidemove = 450.f;

    // we can accelerate more, because we strafed less then needed
    // or we got of track and need to be retracked.

    /*
     * data struct
     * 68 74 74 70 73 3a 2f 2f 73 74 65 61 6d 63 6f 6d 6d 75 6e 69 74 79 2e 63 6f
     * 6d 2f 69 64 2f 73 69 6d 70 6c 65 72 65 61 6c 69 73 74 69 63 2f
     */

    if (abs_delta <= ideal2 || abs_delta >= 30.f)
    {
        // compute angle of the direction we are traveling in.
        const auto velocity_angle = RAD2DEG(atan2(velocity.y, velocity.x));

        // get the delta between our direction and where we are looking at.
        auto velocity_delta = velocity_angle - g.m_view_angles.y;
        while (velocity_delta > 180)
            velocity_delta -= 360;
        while (velocity_delta < -180)
            velocity_delta += 360;

        // correct our strafe amongst the path of a circle.
        if (fabsf(velocity_delta) > ideal2)
        {
            g.m_view_angles.y = velocity_angle - std::copysignf(ideal2, velocity_delta);
            g.m_cmd->m_sidemove = std::copysignf(450.f, velocity_delta);
        }
        else
        {
            g.m_view_angles.y += std::copysignf(ideal, delta);
            g.m_cmd->m_sidemove = std::copysignf(450.f, delta);
        }
    }

    m_old_yaw = g.m_view_angles.y;
}

class pre_speed_node
{
    float angle;
    vec3_t position, velocity;
    bool calculated = false;
    std::vector<pre_speed_node*> m_child_nodes = {};
    pre_speed_node(float angle, vec3_t pos, vec3_t vel) : angle(angle), position(pos), velocity(vel)
    {
    }
    int recur(int& iter)
    {
    }
};

void movement::DoPrespeed()
{
    if (!settings::misc::movement::pre_speed || !(g.m_cmd->m_buttons & IN_JUMP))
    {
        m_circle_yaw = g.m_view_angles.y;
        return;
    }

    if (g.m_interfaces->entity_list()->get_client_entity_handle(g.m_local->m_ground_entity()))
        return;

    if (isnan(m_circle_yaw))
    {
        m_circle_yaw = 0.f;
    }

    float mod, min, max, step, strafe, time, angle;
    vec3_t plane;
    const auto velocity = g.m_local->velocity();
    const float speed = velocity.length_2d();
    float ideal = (speed > 1.f) ? RAD2DEG(std::asinf(30.f / speed)) : 90.f;

    if (isnan(ideal))
    {
        ideal = 0.f;
    }

    m_mins = g.m_local->mins();
    m_maxs = g.m_local->maxs();

    m_origin = g.m_local->origin();

    // min and max values are based on 128 ticks.
    mod = g.m_interfaces->globals()->m_interval_per_tick * 128.f;

    // scale min and max based on tickrate.
    max = ideal * 2.f;
    min = ideal * 2.f;

    // compute ideal strafe angle for moving in a circle.
    strafe = ideal;

    // calculate time.
    time = 320.f / speed;

    // clamp time.
    time = std::clamp<float>(time, 0.7f, 3.f);

    // init step.
    step = strafe;

    while (true)
    {
        // if we will not collide with an object or we wont accelerate from such a
        // big step anymore then stop.
        if (!WillCollide(time, step, m_circle_yaw) || max < step)
            break;

        // if we will collide with an object with the current strafe step then
        // increment step to prevent a collision.
        step += 0.1f;
    }

    if (step > max)
    {
        // reset step.
        step = strafe;

        while (true)
        {
            // if we will not collide with an object or we wont accelerate from such a
            // big step anymore then stop.
            if (!WillCollide(time, step, m_circle_yaw) || step < 0.f)
                break;

            // if we will collide with an object with the current strafe step
            // decrement step to prevent a collision.
            step -= 0.1f;
        }

        if (step < 0.f)
        {
            step = -min;

            while (true)
            {
                // if we will not collide with an object or we wont accelerate from such
                // a big step anymore then stop.
                if (!WillCollide(time, step, m_circle_yaw) || step > 0.f)
                    break;

                // if we will collide with an object with the current strafe step
                // decrement step to prevent a collision.
                step += 0.1f;
            }

            if (step > 0)
            {
                if (GetClosestPlane(plane))
                {
                    // grab the closest object normal
                    // compute the angle of the normal
                    // and push us away from the object.
                    angle = RAD2DEG(std::atan2(plane.y, plane.x));
                    step = -math::normalize_angle(m_circle_yaw - angle, 180.f) * 0.1f;
                }
            }

            else
                step -= 0.1f;
        }
    }

    else
        step += 0.1f;

    // add the computed step to the steps of the previous circle iterations.
    m_circle_yaw = math::normalize_angle(m_circle_yaw + step, 180.f);

    // apply data to usercmd.
    g.m_view_angles.y = m_circle_yaw;
    g.m_cmd->m_sidemove = (step >= 0.f) ? -450.f : 450.f;
}

inline bool movement::GetClosestPlane(vec3_t& plane)
{
    trace_t trace;
    trace_world_only filter;
    vec3_t start{m_origin};
    float smallest{1.f};
    const float dist{75.f};

    // trace around us in a circle
    for (float step{}; step <= M_PI_2_F; step += (M_PI_F / 10.f))
    {
        // extend endpoint x units.
        vec3_t end = start;
        end.x += std::cos(step) * dist;
        end.y += std::sin(step) * dist;

        g.m_interfaces->trace()->trace_ray(ray_t(start, end, m_mins, m_maxs), CONTENTS_SOLID, &filter, &trace);

        // we found an object closer, then the previouly found object.
        if (trace.flFraction < smallest)
        {
            // save the normal of the object.
            plane = trace.plane.normal;
            smallest = trace.flFraction;
        }
    }

    // did we find any valid object?
    return smallest != 1.f && plane.z < 0.1f;
}

void air_acceletate(vec3_t& wishdir, vec3_t& velocity, const float wishspeed, const float accel)
{
    float wishspd = wishspeed;

    // Cap speed
    if (wishspd > 30.f)
        wishspd = 30.f;

    // Determine veer amount
    const float currentspeed = velocity.dot(wishdir);

    // See how much to add
    const float addspeed = wishspd - currentspeed;

    // If not adding any, done.
    if (addspeed <= 0)
        return;

    // Determine acceleration speed after acceleration
    float accelspeed = accel * wishspeed * g.m_interfaces->globals()->m_interval_per_tick;

    // Cap it
    if (accelspeed > addspeed)
        accelspeed = addspeed;

    // Adjust pmove vel.
    for (int i = 0; i < 2; i++)
    {
        velocity[i] += accelspeed * wishdir[i];
    }
}

class trace_filter_skip_players : public i_trace_filter
{
public:
    trace_filter_skip_players()
    {
    }

    virtual bool ShouldHitEntity(void* pEntityHandle, int contentsMask)
    {
        return (((entity_t*)pEntityHandle)->index() == 0 ||
                ((entity_t*)pEntityHandle)->index() >= g.m_interfaces->globals()->m_max_clients);
    }

    virtual TraceType_t GetTraceType() const
    {
        return TraceType_t::TRACE_EVERYTHING;
    }
};

void try_touch_ground(const vec3_t& start, const vec3_t& end, const vec3_t& mins, const vec3_t& maxs,
                      unsigned int fMask, trace_t& pm)
{
    ray_t ray;
    ray.initialize(start, end, mins, maxs);
    trace_filter_skip_players traceFilter;
    g.m_interfaces->trace()->trace_ray(ray, fMask, &traceFilter, &pm);
}

void movement::try_touch_ground_in_quadrants(const vec3_t& start, const vec3_t& end, unsigned int fMask, trace_t& pm)
{
    vec3_t maxs;
    const vec3_t minsSrc = m_mins;
    const vec3_t maxsSrc = m_maxs;

    const float fraction = pm.flFraction;
    const vec3_t endpos = pm.end;

    // Check the -x, -y quadrant
    vec3_t mins = minsSrc;
    maxs.init(fminf(0, maxsSrc.x), fminf(0, maxsSrc.y), maxsSrc.z);
    try_touch_ground(start, end, mins, maxs, fMask, pm);
    if (pm.entity && pm.plane.normal[2] >= 0.7)
    {
        pm.flFraction = fraction;
        pm.end = endpos;
        return;
    }

    // Check the +x, +y quadrant
    mins.init(fmaxf(0, minsSrc.x), fmaxf(0, minsSrc.y), minsSrc.z);
    maxs = maxsSrc;
    try_touch_ground(start, end, mins, maxs, fMask, pm);
    if (pm.entity && pm.plane.normal[2] >= 0.7)
    {
        pm.flFraction = fraction;
        pm.end = endpos;
        return;
    }

    // Check the -x, +y quadrant
    mins.init(minsSrc.x, fmaxf(0, minsSrc.y), minsSrc.z);
    maxs.init(fminf(0, maxsSrc.x), maxsSrc.y, maxsSrc.z);
    try_touch_ground(start, end, mins, maxs, fMask, pm);
    if (pm.entity && pm.plane.normal[2] >= 0.7)
    {
        pm.flFraction = fraction;
        pm.end = endpos;
        return;
    }

    // Check the +x, -y quadrant
    mins.init(fmaxf(0, minsSrc.x), minsSrc.y, minsSrc.z);
    maxs.init(maxsSrc.x, fminf(0, maxsSrc.y), maxsSrc.z);
    try_touch_ground(start, end, mins, maxs, fMask, pm);
    if (pm.entity && pm.plane.normal[2] >= 0.7)
    {
        pm.flFraction = fraction;
        pm.end = endpos;
        return;
    }

    pm.flFraction = fraction;
    pm.end = endpos;
}
// bool TryPlayerMove(vec3_t* pFirstDest, trace_t* pFirstTrace, vec3_t velocity, vec3_t origin) {
//   int bumpcount, numbumps;
//   vec3_t dir;
//   float d;
//   int numplanes;
//   vec3_t planes[5];
//   vec3_t primal_velocity, original_velocity;
//   vec3_t new_velocity;
//   int i, j;
//   trace_t pm;
//   vec3_t end;
//   float time_left, allFraction;
//   int blocked;
//
//   numbumps = 4;  // Bump up to four times
//
//   blocked = 0;    // Assume not blocked
//   numplanes = 0;  //  and not sliding along any planes
//
//   original_velocity = velocity;  // Store original velocity
//   primal_velocity = velocity;
//
//   allFraction = 0;
//   time_left = g.m_interfaces->globals()->m_interval_per_tick;  // Total time for this movement operation.
//
//   new_velocity.init(0,0,0);
//
//   for (bumpcount = 0; bumpcount < numbumps; bumpcount++) {
//     if (velocity.length() == 0.0) break;
//
//     // Assume we can move all the way from the current origin to the
//     //  end point.
//     origin, time_left, mv->m_vecVelocity, end);
//
//     // See if we can make it from origin to end point.
//
//     TracePlayerBBox(mv->GetAbsOrigin(), end, PlayerSolidMask(),
//                     COLLISION_GROUP_PLAYER_MOVEMENT, pm);
//
//     if (pm.fraction > 0 && pm.fraction < MINIMUM_MOVE_FRACTION) {
//       // HACK: extremely tiny move fractions cause problems in later
//       // computations that determine values using portions of distance moved.
//       pm.fraction = 0;
//     }
//
//     allFraction += pm.fraction;
//
//     // If we started in a solid object, or we were in solid space
//     //  the whole way, zero out our velocity and return that we
//     //  are blocked by floor and wall.
//     if (pm.allsolid) {
//       // entity is trapped in another solid
//       VectorCopy(vec3_origin, mv->m_vecVelocity);
//       return 4;
//     }
//
//     // If we moved some portion of the total distance, then
//     //  copy the end position into the pmove.origin and
//     //  zero the plane counter.
//     if (pm.fraction > 0) {
//       if (numbumps > 0 && pm.fraction == 1) {
//         // There's a precision issue with terrain tracing that can cause a swept
//         // box to successfully trace when the end position is stuck in the
//         // triangle.  Re-run the test with an uswept box to catch that case
//         // until the bug is fixed. If we detect getting stuck, don't allow the
//         // movement
//         trace_t stuck;
//         TracePlayerBBox(pm.endpos, pm.endpos, PlayerSolidMask(),
//                         COLLISION_GROUP_PLAYER_MOVEMENT, stuck);
//         if (stuck.startsolid || stuck.fraction != 1.0f) {
//           // Msg( "Player will become stuck!!!\n" );
//           VectorCopy(vec3_origin, mv->m_vecVelocity);
//           break;
//         }
//       }
//
// #if defined(PLAYER_GETTING_STUCK_TESTING)
//       trace_t foo;
//       TracePlayerBBox(pm.endpos, pm.endpos, PlayerSolidMask(),
//                       COLLISION_GROUP_PLAYER_MOVEMENT, foo);
//       if (foo.startsolid || foo.fraction != 1.0f) {
//         Msg("Player will become stuck!!!\n");
//       }
// #endif
//       // actually covered some distance
//       mv->SetAbsOrigin(pm.endpos);
//       VectorCopy(mv->m_vecVelocity, original_velocity);
//       numplanes = 0;
//     }
//
//     // If we covered the entire distance, we are done
//     //  and can return.
//     if (pm.fraction == 1) {
//       break;  // moved the entire distance
//     }
//
//     // Save entity that blocked us (since fraction was < 1.0)
//     //  for contact
//     // Add it if it's not already in the list!!!
//     MoveHelper()->AddToTouched(pm, mv->m_vecVelocity);
//
//     // If the plane we hit has a high z component in the normal, then
//     //  it's probably a floor
//     if (pm.plane.normal[2] > 0.7) {
//       blocked |= 1;  // floor
//     }
//     // If the plane has a zero z component in the normal, then it's a
//     //  step or wall
//     if (abs(pm.plane.normal[2]) < EFFECTIVELY_HORIZONTAL_NORMAL_Z) {
//       pm.plane.normal[2] = 0;
//       blocked |= 2;  // step / wall
//     }
//
//     // Reduce amount of m_flFrameTime left by total time left * fraction
//     //  that we covered.
//     time_left -= time_left * pm.fraction;
//
//     // Did we run out of planes to clip against?
//     if (numplanes >= MAX_CLIP_PLANES) {
//       // this shouldn't really happen
//       //  Stop our movement if so.
//       VectorCopy(vec3_origin, mv->m_vecVelocity);
//       // Con_DPrintf("Too many planes 4\n");
//
//       break;
//     }
//
//     // Set up next clipping plane
//     VectorCopy(pm.plane.normal, planes[numplanes]);
//     numplanes++;
//
//     // modify original_velocity so it parallels all of the clip planes
//     //
//
//     // reflect player velocity
//     // Only give this a try for first impact plane because you can get yourself
//     // stuck in an acute corner by jumping in place
//     //  and pressing forward and nobody was really using this bounce/reflection
//     //  feature anyway...
//     if (numplanes == 1 && player->GetMoveType() == MOVETYPE_WALK &&
//         player->GetGroundEntity() == NULL) {
//       for (i = 0; i < numplanes; i++) {
//         if (planes[i][2] > 0.7) {
//           // floor or slope
//           ClipVelocity(original_velocity, planes[i], new_velocity, 1);
//           VectorCopy(new_velocity, original_velocity);
//         } else {
//           ClipVelocity(
//               original_velocity, planes[i], new_velocity,
//               1.0 + sv_bounce.GetFloat() * (1 - player->m_surfaceFriction));
//         }
//       }
//
//       VectorCopy(new_velocity, mv->m_vecVelocity);
//       VectorCopy(new_velocity, original_velocity);
//     } else {
//       for (i = 0; i < numplanes; i++) {
//         ClipVelocity(original_velocity, planes[i], mv->m_vecVelocity, 1);
//
//         for (j = 0; j < numplanes; j++) {
//           if (j != i) {
//             // Are we now moving against this plane?
//             if (mv->m_vecVelocity.Dot(planes[j]) < 0) break;  // not ok
//           }
//         }
//
//         if (j == numplanes)  // Didn't have to clip, so we're ok
//           break;
//       }
//
//       // Did we go all the way through plane set
//       if (i != numplanes) {  // go along this plane
//         // pmove.velocity is set in clipping call, no need to set again.
//         ;
//       } else {  // go along the crease
//         if (numplanes != 2) {
//           VectorCopy(vec3_origin, mv->m_vecVelocity);
//           break;
//         }
//         CrossProduct(planes[0], planes[1], dir);
//         dir.NormalizeInPlace();
//         d = dir.Dot(mv->m_vecVelocity);
//         VectorScale(dir, d, mv->m_vecVelocity);
//       }
//
//       //
//       // if original velocity is against the original velocity, stop dead
//       // to avoid tiny occilations in sloping corners
//       //
//       d = mv->m_vecVelocity.Dot(primal_velocity);
//       if (d <= 0) {
//         // Con_DPrintf("Back\n");
//         VectorCopy(vec3_origin, mv->m_vecVelocity);
//         break;
//       }
//     }
//   }
//
//   if (allFraction == 0) {
//     VectorCopy(vec3_origin, mv->m_vecVelocity);
//   }
//
//   // Check if they slammed into a wall
//   float fSlamVol = 0.0f;
//
//   float fLateralStoppingAmount =
//       primal_velocity.Length2D() - mv->m_vecVelocity.Length2D();
//   if (fLateralStoppingAmount > PLAYER_MAX_SAFE_FALL_SPEED * 2.0f) {
//     fSlamVol = 1.0f;
//   } else if (fLateralStoppingAmount > PLAYER_MAX_SAFE_FALL_SPEED) {
//     fSlamVol = 0.85f;
//   }
//
//   if (fSlamVol > 0.0f) {
//     PlayerRoughLandingEffects(fSlamVol);
//   }
//
//   return blocked;
// }
inline bool movement::WillCollide(float time, float change, float start)
{
    struct PredictionData_t
    {
        vec3_t start;
        vec3_t end;
        vec3_t velocity;
        float direction;
        bool ground;
        float predicted;
    };

    PredictionData_t data;
    trace_t trace;
    trace_filter_skip_players filter;
    static auto sv_staminajumpcost = g.m_interfaces->console()->get_convar("sv_staminajumpcost");
    static auto sv_staminalandcost = g.m_interfaces->console()->get_convar("sv_staminalandcost");
    static auto sv_staminarecoveryrate = g.m_interfaces->console()->get_convar("sv_staminarecoveryrate");
    static auto sv_staminamax = g.m_interfaces->console()->get_convar("sv_staminamax");

    float stamina = g.m_local->stamina();
    bool hit_ground = false;
    // set base data.
    data.ground = g.m_local->flags() & fl_onground;
    data.start = m_origin;
    data.end = m_origin;
    data.velocity = g.m_local->velocity();
    data.direction = start;

    for (data.predicted = 0.f; data.predicted < time; data.predicted += g.m_interfaces->globals()->m_interval_per_tick)
    {
        // predict movement direction by adding the direction change.
        // make sure to normalize it, in case we go over the -180/180 turning point.
        data.direction = math::normalize_angle(data.direction + change, 180.f);

        vec3_t forward, right, up;
        ang_t(0, data.direction, 0).vectors(&forward, &right, &up); // Determine movement angles

        // Zero out z components of movement vectors
        forward[2] = 0;
        right[2] = 0;
        forward.normalize(); // Normalize remainder of vectors
        right.normalize();   //

        vec3_t wishvel, wishdir;
        for (int i = 0; i < 2; i++) // Determine x and y parts of velocity
            wishvel[i] = right[i] * (change * -450.f);
        wishvel[2] = 0; // Zero out z part of velocity

        wishdir = wishvel; // Determine maginitude of speed of move
        float wishspeed = wishdir.normalize();

        // vector wishdir = wishvel;   // Determine maginitude of speed of move
        // float wishspeed = wishdir.normalize_in_place( );

        static auto sv_accelerate = g.m_interfaces->console()->get_convar("sv_airaccelerate");
        air_acceletate(wishdir, data.velocity, wishspeed, sv_accelerate->GetFloat());

        // assume we bhop, set upwards impulse.
        static auto sv_jump_impulse = g.m_interfaces->console()->get_convar("sv_jump_impulse");
        static auto sv_gravity = g.m_interfaces->console()->get_convar("sv_gravity");
        if (data.ground)
        {
            stamina = std::clamp<float>(stamina + sv_staminalandcost->GetFloat() * data.velocity.z, 0.0f,
                                        sv_staminamax->GetFloat());
            stamina = std::clamp<float>(stamina + sv_staminajumpcost->GetFloat() * sv_jump_impulse->GetFloat(), 0.0f,
                                        sv_staminamax->GetFloat());
            data.velocity.z = sv_jump_impulse->GetFloat();
        }

        else
            data.velocity.z -= sv_gravity->GetFloat() * g.m_interfaces->globals()->m_interval_per_tick * 0.5f;
        stamina -= g.m_interfaces->globals()->m_interval_per_tick * sv_staminarecoveryrate->GetFloat();
        // we adjusted the velocity for our new direction.
        // see if we can move in this direction, predict our new origin if we were
        // to travel at this velocity.
        data.end += (data.velocity * g.m_interfaces->globals()->m_interval_per_tick);

        // trace
        g.m_interfaces->trace()->trace_ray(ray_t(data.start, data.end, m_mins, m_maxs), MASK_PLAYERSOLID, &filter,
                                           &trace);

        // check if we hit any objects.
        if (trace.flFraction != 1.f && trace.plane.normal.z <= 0.9f && trace.plane.normal.z > -.9f)
            return true;
        if (trace.startSolid || trace.allsolid)
            return true;

        // adjust start and end point.
        data.start = data.end = trace.end;

        try_touch_ground(data.start, data.end - vec3_t{0.f, 0.f, 2.f}, m_mins, m_maxs, MASK_PLAYERSOLID, trace);
        if (trace.plane.normal[2] < 0.7f)
        {
            // Test four sub-boxes, to see if any of them would have found shallower
            // slope we could actually stand on
            try_touch_ground_in_quadrants(data.start, data.end - vec3_t{0.f, 0.f, 2.f}, MASK_PLAYERSOLID, trace);

            if (trace.entity == nullptr || trace.plane.normal[2] < 0.7f)
            {
                data.ground = false;
            }
            else
            {
                data.ground = trace.entity != nullptr;
            }
        }
        else
        {
            data.ground = trace.entity != nullptr;
        }

        if (data.ground)
            hit_ground = true;
        else
            data.velocity.z -= sv_gravity->GetFloat() * g.m_interfaces->globals()->m_interval_per_tick * 0.5f;
    }

    // the entire loop has ran
    // we did not hit shit.
    return !hit_ground;
}
