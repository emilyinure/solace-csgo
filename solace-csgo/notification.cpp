#include "notification.h"
#include "includes.h"

void notification::add ( const std::string text, color _color, float time, bool console ) {
	// modelled after 'CConPanel::AddToNotify'
	m_notify_text.push_back(
		std::make_shared< NotifyText >( text, _color, g.m_interfaces->globals( )->m_curtime + time ) );

	va_list list;
	std::string buf;

	if ( text.empty( ) )
		return;

	va_start( list, text );

	// count needed size.
	const int size = std::vsnprintf( 0, 0, text.c_str( ), list );

	// allocate.
	buf.resize( size );

	// print to buffer.
	std::vsnprintf( buf.data( ), size + 1, text.c_str( ), list );

	va_end( list );

	// print to console.
	g.m_interfaces->console( )->console_color_printf( color( 0xDB, 0x2E, 0x2C, 20 ), "[solace] " );
	g.m_interfaces->console( )->console_color_printf( color( 255, 255, 255 ), buf.c_str( ) );
}

void notification::think ( ) {
	auto x{8}, y{5};
	const auto size{g.m_render->get_text_height( "A", g.m_render->m_courier_new_13( ) ) + 1};

	// update lifetimes.
	for ( size_t i{}; i < m_notify_text.size( ); ++i ) {
		const auto notify = m_notify_text[i];
		const auto delta_time = notify->m_time - g.m_interfaces->globals( )->m_curtime;

		if ( delta_time <= 0.f ) {
			m_notify_text.erase( m_notify_text.begin( ) + i );
			i--;
		}
	}

	// we have nothing to draw.
	if ( m_notify_text.empty( ) )
		return;

	// iterate entries.
	for ( size_t i{}; i < m_notify_text.size( ); ++i ) {
		const auto notify = m_notify_text[i];

		const auto left = notify->m_time - g.m_interfaces->globals( )->m_curtime;
		auto color = notify->m_color;

		if ( left < .5f ) {
			auto f = left;
			f = std::clamp( f, 0.f, .5f );

			f /= .5f;

			color.set_a( static_cast< int >(f * 255.f) );

			if ( i == 0 && f < 0.2f )
				y -= size * int(1.f - f / 0.2f);
		}

		else
			color.set_a( 255 );

		g.m_render->text( g.m_render->m_courier_new_13( ), x, y, color, notify->m_text.c_str( ) );
		y += size;
	}
}
