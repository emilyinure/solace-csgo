#pragma once
#include "global_vars.h"
#include "math_funcs.h"
#include "vec3.h"

class movement {
	bool m_force_strafe = false;
	bool m_should_stop = false;
public:
	void set_should_stop( bool state ) { m_should_stop = state; };
	void set_force_strafe( bool state ) { m_force_strafe = state; };
	void draw ( );
	void bhop ( );
	void QuickStop ( );
	static void PreciseMove ( );
	void auto_peek ( );
	void move_to ( vec3_t target_origin ) const;
	void edge_bug();
	void move_to ( );
	void auto_strafe( );
	float m_old_yaw;
	int m_switch = 1;
	bool m_invert;
	vec3_t m_stop_pos;
	float m_time_left;
} inline g_movement;

