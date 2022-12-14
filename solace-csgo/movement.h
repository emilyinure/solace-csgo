#pragma once
#include "global_vars.h"
#include "math_funcs.h"
#include "vec3.h"

class movement {
	bool m_force_strafe = false;
	bool m_should_stop = false;
	bool m_should_unpeek = false;
	float m_circle_yaw = 0.f;
	vec3_t m_origin;
	vec3_t m_mins;
	vec3_t m_maxs;
    float m_direction = 1;

public:
	void set_should_stop( bool state ) { m_should_stop = state; };
	void set_force_strafe( bool state ) { m_force_strafe = state; };
	void set_should_unpeek( bool state ) { m_should_unpeek = state; };
	void draw ( );
	void bhop ( );
	void QuickStop ( );
	static void PreciseMove ( );
	void auto_peek ( );
	void move_to ( vec3_t target_origin ) const;
	void edge_bug();
	void auto_strafe( );
	float m_old_yaw;
	int m_switch = 1;
	bool m_invert;
	vec3_t m_stop_pos;
	float m_time_left;
	

    bool check_rotations(float start_yaw, vec3_t origin, vec3_t velocity, float& end_speed, float stamina, int iter);

    void DoPrespeed();

	bool try_player_move( vec3_t& velocity, vec3_t& position, bool on_ground );
	bool GetClosestPlane( vec3_t& plane );
    void try_touch_ground_in_quadrants(const vec3_t& start, const vec3_t& end, unsigned int fMask, trace_t& pm);
    bool WillCollide(float time, float change, float start);
} inline g_movement;

