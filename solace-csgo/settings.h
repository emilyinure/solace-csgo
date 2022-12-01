#pragma once

namespace settings {
	namespace rage {
		namespace general {
			inline bool enabled = true;
			inline bool key = false;
			inline bool auto_shoot = true;
			inline bool silent = true;
			inline float delay_shot = 0;
		}
		namespace hitbox {
			inline int hitboxes = 0;
			inline float point_scale = 85;
			inline float body_scale = 80;
			inline int baim_conditions = 0;
		}
		namespace selection {
			inline float hitchance = 58;
			inline float min_damage = 20;
			inline float lethal_damage = 10;
			inline float fov = 0;
		}
	}
	namespace visuals {
		namespace players {
			inline bool box = false;
			inline bool name = true;
			inline bool weapon = true;
			inline float offscreen = 0.5;
			inline bool health = true;
			inline int chams = 2;
			inline int chams_covered = 2;
			inline int chams_team = false;
			inline int chams_team_covered = 2;
			inline bool team = false;
		}
		namespace weapons {
			inline bool box = false;
			inline bool name = false;
			inline bool noscope = true;
			inline bool grenade_prediction = true;
		}
		namespace world {
			inline bool wire_smoke = true;
			inline bool molotov = true;
		}
		namespace misc {
			inline float fov = 0;
			inline float aspectratio = 0;
		}
	}
	namespace legit {
		namespace general {
			inline bool enabled = false;
			inline bool auto_shoot = false;
			inline bool silent = false;
			inline bool key = false;
			inline float smoothing = 0;
		}
		namespace selection {
			inline float hitchance = 0;
			inline float min_damage = 0;
			inline int hitboxes = 0;
			inline float fov = 0;
		}
		namespace recoil {
			inline bool enabled = false;
			inline float x_factor = 0;
			inline float y_factor = 0;
		}
	}
	namespace hvh {
		namespace antiaim {
			inline bool enabled = true;
			inline float lag_limit = 15;
			inline int body_fake_stand = 4;
			inline int body_fake_air = 0;
			inline int fake_yaw = 3;
			inline float fake_relative = 0;
			inline float fake_jitter_range = 45;
			inline int pitch_stand = 1;
			inline int yaw_stand = 1;
			inline float jitter_range_stand = 0;
			inline float rot_range_stand= 0;
			inline float rot_speed_stand= 0;
			inline float rand_update_stand= 0;
			inline int dir_stand= 0;
			inline int dir_custom_stand= 0;
			inline int base_angle_stand= 0;
			inline float dir_time_stand= 0;
			inline int pitch_walk= 0;
			inline int yaw_walk= 0;
			inline float jitter_range_walk= 0;
			inline float rot_range_walk= 0;
			inline float rot_speed_walk= 0;
			inline float rand_update_walk= 0;
			inline int dir_walk= 0;
			inline int dir_custom_walk= 0;
			inline int base_angle_walk= 0;
			inline float dir_time_walk= 0;
			inline int pitch_air= 0;
			inline int yaw_air= 0;
			inline float jitter_range_air= 0;
			inline float rot_range_air= 0;
			inline float rot_speed_air= 0;
			inline float rand_update_air= 0;
			inline int dir_air= 0;
			inline int dir_custom_air= 0;
			inline int base_angle_air= 0;
			inline float dir_time_air= 0;
			inline bool lag_enable= 1;
			inline int lag_active= 1;
			inline int lag_mode= 0;
			inline bool lag_land= 0;
			inline bool fakewalk = 0;
			inline bool fakehead = 0;
			inline bool dir_lock= 0;
			inline bool edge = 0;
			inline bool auto_peek = 0;
		}
	}
	namespace misc {
		namespace movement {
			inline bool bhop = true;
			inline bool pre_speed = false;
			inline int autostrafe = 1;
		}
		namespace griefing {
			inline bool block_bot = false;
		}
		namespace config {
			inline int slot = 1;
		}
		namespace misc {
			inline bool thirdperson = false;
			inline bool fake_latency = false;
			inline float fake_latency_amt = 0;
		}
	}
}
