#pragma once

class ang_t;
class player_t;

class AdaptiveAngle {
public:
	float m_yaw;
	float m_dist;

public:
	// ctor.
	__forceinline AdaptiveAngle ( float yaw, float penalty = 0.f );
};

enum AntiAimMode : size_t {
	STAND = 0,
	WALK,
	AIR,
};

class hvh {
public:
	float last_lby_update = -1;
	int m_pitch = 0;
	float m_view = 0;
	float m_auto = 0;
	float m_auto_dist = 0;
	float m_auto_last = 0;
	float m_auto_time = 0;
	float m_direction = 0;
	int m_break_dir = 0;
	int m_base_angle = 0;
	int m_dir = 0;
	int m_yaw = 0;
	bool m_breaking = false;
	float m_jitter_range = 0;
	float m_rot_range = 0;
	float m_rot_speed = 0;
	float m_next_random_update = 0;
	float m_random_angle = 0;
	float m_rand_update = 0;
	int m_dir_custom = 0;
	int m_mode = 0;
	int m_random_lag = 0;
	bool m_step_switch = 0;
	float m_view_angle;
	bool m_just_updated_body;
	int m_switch = 1;
	int m_lby_side = 1;
	int m_side = 1;
	void fake_walk ( ) const;
	void break_resolver();
	static void IdealPitch ( );
	void AntiAimPitch ( ) const;
	void AutoDirection ( );
	void GetAntiAimDirection ( );
	void DoRealAntiAim ( );
	void DoFakeAntiAim ( ) const;
	void AntiAim ( );
	void SendPacket ( );
} inline g_hvh;
