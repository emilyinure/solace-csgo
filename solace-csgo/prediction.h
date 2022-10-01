#pragma once
class cmd_t;
class player_t;
namespace prediction {
	void start( cmd_t *cmd );
	void update( );
	void end( );
	void finish_partial_frame( player_t *player, cmd_t *cmd );
	void re_predict( cmd_t *cmd );

	inline float old_cur_time;
	inline float old_frame_time;
	inline int *prediction_random_seed;
	inline player_t *prediction_player;
	inline int *should_predict;
};
