#pragma once
#include "global_vars.h"
#include "math_funcs.h"
#include "vec3.h"

class movement {
public:
	int jump_count = 0;
	void draw ( );
	void bhop ( );
	static void QuickStop ( );
	static void PreciseMove ( );
	void auto_peek ( );
	void move_to ( vec3_t target_origin ) const;
	void edge_bug();
	void move_to ( );
	void auto_strafe( );
	float m_speed;
	float m_ideal;
	float m_ideal2;
	int m_old_yaw;
	int m_switch = 1;
	bool m_invert;
	vec3_t m_stop_pos;
	float m_time_left;
} inline g_movement;

