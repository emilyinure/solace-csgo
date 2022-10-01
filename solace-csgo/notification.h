#pragma once


#include <memory>
#include <string>

#include "render.h"

class notification {
	class NotifyText {
	public:
		std::string m_text;
		color		m_color;
		float		m_time;

	public:
		__forceinline NotifyText( const std::string &text, color color, float time ) : m_text{ text }, m_color{ color }, m_time{ time } {}
	};

	private:
		std::vector< std::shared_ptr< NotifyText > > m_notify_text;

	public:
		__forceinline notification( ) : m_notify_text{} { }

	void add ( const std::string text, color _color = color( 255, 255, 255, 255 ), float time = 8.f,
	                         bool console = true );

		// modelled after 'CConPanel::DrawNotify' and 'CConPanel::ShouldDraw'
	void think ( );
} inline g_notification;

