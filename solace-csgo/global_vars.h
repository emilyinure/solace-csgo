#pragma once

class c_global_vars {
public:
	float m_realtime;
	int m_framecount;
	float m_abs_frametime;
	float m_abs_frame_start_time;
	float m_curtime;
	float m_frametime;
	int m_max_clients;
	int m_tickcount;
	float m_interval_per_tick;
	float m_interp_amount;
	int m_frame_simulation_ticks;
	int m_network_protocol;
	void* m_save_data;
	bool m_client;
	bool m_remote_client;
	int m_timestamp_networking_base;
	int m_timestamp_randomize_window;
};