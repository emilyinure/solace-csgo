#pragma once
#include "../menu.hh"

class c_input_helper {
	vector_2d mouse_position_{ 0, 0 };
	float scroll_y{ 0 };
	bool key_state_[ 256 ]{ };
	bool prestine_key_state_[ 256 ]{ };
	bool previous_key_state_[ 256 ]{ };
	const char* keys_[ 253 ] = { "Unassigned", "Left Mouse", "Right Mouse", "Control+Break", "Middle Mouse", "Mouse 4", "Mouse 5", "No bind", "Backspace", "TAB", "No bind", "No bind", "No bind", "ENTER", "No bind", "No bind", "SHIFT", "CTRL", "ALT", "PAUSE", "CAPS LOCK", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "ESC", "No bind", "No bind", "No bind", "No bind", "Spacebar", "Page up", "Page down", "End", "Home", "Left", "Up", "Right", "Down", "No bind", "Print", "No bind", "Print Screen", "Insert", "Delete", "No bind", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "Left Windows", "Right Windows", "No bind", "No bind", "No bind", "NUM 0", "NUM 1", "NUM 2", "NUM 3", "NUM 4", "NUM 5", "NUM 6", "NUM 7", "NUM 8", "NUM 9", "*", "+", "_", "-", ".", "/", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "F13", "F14", "F15", "F16", "F17", "F18", "F19", "F20", "F21", "F22", "F23", "F24", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "NUM LOCK", "SCROLL LOCK", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "LSHIFT", "RSHIFT", "LCONTROL", "RCONTROL", "LMENU", "RMENU", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "Next Track", "Previous Track", "Stop", "Play/Pause", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", ";", "+", ",", "-", ".", "/?", "~", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "[{", "\\|", "}]", "'\"", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind", "No bind" };
public:
	auto update( ) -> void {
		for ( auto i { 0 }; i < 256; i++ ) {
			previous_key_state_[ i ] = prestine_key_state_[ i ];
			key_state_[ i ] = GetAsyncKeyState( i );
			prestine_key_state_[ i ] = key_state_[ i ];
		}
	}

	auto scroll( ) -> float {
		float ret = scroll_y;
		if ( abs(ret) > 1 )
			ret /= 10;
		scroll_y -= ret;
		return ret;
	}

	auto key_pressed( const int key ) const -> bool {
		return key_state_[ key ] && !previous_key_state_[ key ];
	}
	
	auto key_pressed_prestine( const int key ) const -> bool {
		return prestine_key_state_[ key ] && !previous_key_state_[ key ];
	}

	auto set_key( const int key, bool set ) -> void {
		key_state_[ key ] = set;
	}

	auto key_down( const int key ) const -> bool {
		return prestine_key_state_[ key ];
	}

	auto hovering( const area_t area ) const -> bool {
		return this->mouse_position_.x > area.x && this->mouse_position_.y > area.y && this->mouse_position_.x < area.x + area.w && this->mouse_position_.y < area.y + area.h;
	}

	auto m_mouse_position( ) const -> vector_2d {
		return this->mouse_position_;
	}

	auto set_mouse_position( const int x, const int y ) -> void {
		this->mouse_position_.x = x;
		this->mouse_position_.y = y;
	}
	
	void set_scroll_position( float i ) {
		this->scroll_y = 0;
		this->scroll_y = i*30.f;
	}

	auto set_mouse_position( const vector_2d mouse_position ) -> void {
		this->mouse_position_ = mouse_position;
	}

	[[nodiscard]] auto m_key( const int index ) const -> const char* {
		return this->keys_[ index ];
	}

}; inline c_input_helper input_helper;