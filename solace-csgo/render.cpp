#include "render.h"
#include <future>
#include "includes.h"

void font_t::create ( IDirect3DDevice9 *device, const char *font_name, int size, int weight, int flags ) {
	this->flags = flags;
	D3DXCreateFontA( device, size, 0, weight, 1, false, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
	                 this->flags & fontflag_antialias ? ANTIALIASED_QUALITY : NONANTIALIASED_QUALITY, DEFAULT_PITCH,
	                 font_name, &this->font );
	
}

auto render_t::create_fonts( ) -> void {
	this->tahoma_14_.create( this->device_,  "Tahoma", 12, 900, fontflag_antialias | fontflag_none );
	this->courier_new_13_.create( this->device_, "Courier New", 13, 200, fontflag_antialias | fontflag_none );
	this->segoe_ui_13_.create( this->device_, "Segoe UI", 13, 300, fontflag_antialias | fontflag_none );
	this->constantia_12_.create( this->device_, "Tahoma", 12, 300, fontflag_antialias | fontflag_none );
	this->tahoma_12_.create( this->device_, "Tahoma", 12, FW_DONTCARE, fontflag_antialias | fontflag_none );
}

RECT render_t::get_text_rect( const char* text, font_t &font ) {
	RECT rect = { 0,0,0,0 };

	if ( font.font )
		font.font->DrawTextA( nullptr, text, strlen( text ), &rect, DT_CALCRECT, D3DCOLOR_XRGB( 0, 0, 0 ) );

	return rect;
}

int render_t::get_text_width( const char *text, font_t &font )  {
	RECT rect = { 0,0,0,0 };

	if ( font.font )
		font.font->DrawTextA( nullptr, text, strlen( text ), &rect, DT_CALCRECT, D3DCOLOR_XRGB( 0, 0, 0 ) );

	return rect.right - rect.left;
}

auto render_t::get_text_height( const char *text, font_t &font ) -> int {
	RECT rect = { 0,0,0,0 };
	
	if ( font.font )
		font.font->DrawTextA( nullptr, text, strlen( text ), &rect, DT_CALCRECT, D3DCOLOR_XRGB( 0, 0, 0 ) );

	return rect.bottom - rect.top;
}

auto render_t::setup( IDirect3DDevice9 *device ) -> void {
	static std::once_flag setup{ };
	std::call_once( setup, [ device, this ]( ) {
		this->device_ = device;
		this->create_fonts( );
		this->device_->GetViewport( &this->screen_size_ );
					} );
}

auto render_t::text( font_t &font, float x, float y, color col, const char *text, const int centered ) -> void {
	RECT rect = { 0, 0, 0, 0 };

	auto text_size = get_text_width( text, font );

	const auto draw_text = [ text_size, centered, text, font ]( RECT &rect, color &col, unsigned short x, unsigned short y ) -> void {
		const auto set_rect = [ ]( RECT &rect, const float x, const float y ) {
			SetRect( &rect, x, y, x, y );
		};

		if ( font.flags & fontflag_none ) {
			set_rect( rect, x, y );
			font.font->DrawTextA( nullptr, text, -1, &rect, DT_LEFT | DT_NOCLIP, col );
		} else if ( font.flags & fontflag_dropshadow ) {
			set_rect( rect, x + 1, y + 1 );
			font.font->DrawTextA( nullptr, text, -1, &rect, DT_LEFT | DT_NOCLIP, color( 0, 0, 0, col.m_a( ) ) );

			set_rect( rect, x, y );
			font.font->DrawTextA( nullptr, text, -1, &rect, DT_LEFT | DT_NOCLIP, col );
		} else if ( font.flags & fontflag_outline ) {
			const auto outline_color = color( 0, 0, 0, col.m_a( ) );

			set_rect( rect, x, y + 1 );
			font.font->DrawTextA( nullptr, text, -1, &rect, DT_LEFT | DT_NOCLIP, outline_color );
			set_rect( rect, x + 1, y );
			font.font->DrawTextA( nullptr, text, -1, &rect, DT_LEFT | DT_NOCLIP, outline_color );
			set_rect( rect, x, y - 1 );
			font.font->DrawTextA( nullptr, text, -1, &rect, DT_LEFT | DT_NOCLIP, outline_color );
			set_rect( rect, x - 1, y );
			font.font->DrawTextA( nullptr, text, -1, &rect, DT_LEFT | DT_NOCLIP, outline_color );

			set_rect( rect, x + 1, y );
			font.font->DrawTextA( nullptr, text, -1, &rect, DT_LEFT | DT_NOCLIP, outline_color );
			set_rect( rect, x, y + 1 );
			font.font->DrawTextA( nullptr, text, -1, &rect, DT_LEFT | DT_NOCLIP, outline_color );
			set_rect( rect, x - 1, y );
			font.font->DrawTextA( nullptr, text, -1, &rect, DT_LEFT | DT_NOCLIP, outline_color );
			set_rect( rect, x, y - 1 );
			font.font->DrawTextA( nullptr, text, -1, &rect, DT_LEFT | DT_NOCLIP, outline_color );

			set_rect( rect, x + 1, y + 1 );
			font.font->DrawTextA( nullptr, text, -1, &rect, DT_LEFT | DT_NOCLIP, outline_color );
			set_rect( rect, x - 1, y - 1 );
			font.font->DrawTextA( nullptr, text, -1, &rect, DT_LEFT | DT_NOCLIP, outline_color );
			set_rect( rect, x + 1, y - 1 );
			font.font->DrawTextA( nullptr, text, -1, &rect, DT_LEFT | DT_NOCLIP, outline_color );
			set_rect( rect, x - 1, y + 1 );
			font.font->DrawTextA( nullptr, text, -1, &rect, DT_LEFT | DT_NOCLIP, outline_color );

			set_rect( rect, x, y );
			font.font->DrawTextA( nullptr, text, -1, &rect, DT_LEFT | DT_NOCLIP, col );
		}
	};

	if ( centered == 1 )
		x -= text_size / 2.f;
	text_size = get_text_height( text, font );
	if ( centered == 2 )
		y -= text_size / 2.f;

	draw_text( rect, col, x, y );
}
bool render_t::draw_rounded_box( int x, int y, int width, int height, int precision, int offset_x, int offset_y, color col ) const {

	vertex_t *m_pVertexBuffer = nullptr;
	const auto m_PrimCount = 0;

	if ( !width || !height ) {
		return false;
	}

	if ( width < offset_x * 2 ) {
		offset_x = 0;
		offset_y = 0;
		precision = 0;
	}

	if ( height < offset_y * 2 ) {
		offset_x = 0;
		offset_y = 0;
		precision = 0;
	}

	if ( precision && ( offset_x == 0 || offset_y == 0 ) ) {
		precision = 0;
	} else if ( ( offset_x && offset_y ) && precision < 1 ) {
		offset_x = 0;
		offset_y = 0;
	}

	//first part
	auto f_offsetX = static_cast< float >( offset_x );
	auto f_offsetY = static_cast< float >( offset_y );
	auto f_width = static_cast< float >( width );
	auto f_height = static_cast< float >( height );
	auto f_x = static_cast< float >( x );
	auto f_y = static_cast< float >( y );
	vertex_t vertices[ 18 ] = {
	{ static_cast< float >(x),                        static_cast< float >(y + height - offset_y),    0.0f, 1.0f, col },
	{ static_cast< float >(x),                        static_cast< float >(y + offset_y),            0.0f, 1.0f, col },
	{ static_cast< float >(x + offset_x),            static_cast< float >(y + height - offset_y),    0.0f, 1.0f, col },
	{ static_cast< float >(x + offset_x),            static_cast< float >(y + offset_y),            0.0f, 1.0f, col },
	{ static_cast< float >(x + width - offset_x),    static_cast< float >(y + offset_y),            0.0f, 1.0f, col },
	{ static_cast< float >(x + offset_x),            static_cast< float >(y),                        0.0f, 1.0f, col },
	{ static_cast< float >(x + width - offset_x),    static_cast< float >(y),                        0.0f, 1.0f, col },


	{ static_cast< float >(x + width - offset_x),    static_cast< float >(y),                        0.0f, 1.0f, col },
	{ static_cast< float >(x + width - offset_x),    static_cast< float >(y + offset_y),            0.0f, 1.0f, col },
	{ static_cast< float >(x + width - offset_x),    static_cast< float >(y + offset_y),            0.0f, 1.0f, col },
	 { static_cast< float >(x + width),                static_cast< float >(y + offset_y),            0.0f, 1.0f, col },


	 { static_cast< float >(x + width),                static_cast< float >(y + offset_y),            0.0f, 1.0f, col },
	 { static_cast< float >(x + width),                static_cast< float >(y + height - offset_y),    0.0f, 1.0f, col },
	 { static_cast< float >(x + width - offset_x),    static_cast< float >(y + offset_y),            0.0f, 1.0f, col },
	 { static_cast< float >(x + width - offset_x),    static_cast< float >(y + height - offset_y),    0.0f, 1.0f, col },
	 { static_cast< float >(x + offset_x),            static_cast< float >(y + height - offset_y),    0.0f, 1.0f, col },
	 { static_cast< float >(x + width - offset_x),    static_cast< float >(y + height),            0.0f, 1.0f, col },
	 { static_cast< float >(x + offset_x),            static_cast< float >(y + height),            0.0f, 1.0f, col }
	};

	if ( precision ) {
		++precision;

		//m_PrimCount = precision;
		//
		//const int vertex_count = m_PrimCount + 2;
		//m_pVertexBuffer = new vertex_t[ static_cast< size_t >(vertex_count) * 4 ]( );
		//
		//const vertex_t &v0 = m_pVertexBuffer[ vertex_count * 0 ] = { f_x + f_width - f_offsetX,    f_y + f_offsetY,            0.0f, 1.0f, col };
		//const vertex_t &v1 = m_pVertexBuffer[ vertex_count * 1 ] = { f_x + f_offsetX,            f_y + f_offsetY,            0.0f, 1.0f, col };
		//const vertex_t &v2 = m_pVertexBuffer[ vertex_count * 2 ] = { f_x + f_offsetX,            f_y + f_height - f_offsetY,    0.0f, 1.0f, col };
		//const vertex_t &v3 = m_pVertexBuffer[ vertex_count * 3 ] = { f_x + f_width - f_offsetX,    f_y + f_height - f_offsetY,    0.0f, 1.0f, col };
		//
		//float inc = (M_PI_F / 2) / static_cast< float >( m_PrimCount - 1 );
		//float ang = 0;
		//for ( int i = 0; i <= m_PrimCount; ++i ) {
		//	float d_x = cosf( ang ) * f_offsetX;
		//	float d_y = sinf( ang ) * f_offsetY;
		//
		//	m_pVertexBuffer[ vertex_count * 0 + i + 1 ] = { v0.x + d_x, v0.y - d_y, 0.0f, 1.0f, col };
		//	m_pVertexBuffer[ vertex_count * 1 + i + 1 ] = { v1.x - d_x, v1.y - d_y, 0.0f, 1.0f, col };
		//	m_pVertexBuffer[ vertex_count * 2 + i + 1 ] = { v2.x - d_x, v2.y + d_y, 0.0f, 1.0f, col };
		//	m_pVertexBuffer[ vertex_count * 3 + i + 1 ] = { v3.x + d_x, v3.y + d_y, 0.0f, 1.0f, col };
		//	ang += inc;
		//}
	}

	if ( !device_ ) {
		return false;
	}

	this->device_->SetFVF( D3DFVF_XYZRHW | D3DFVF_DIFFUSE );
	auto hr = device_->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 16, vertices, sizeof( vertex_t ) );
	if ( FAILED( hr ) ) {
		return false;
	}

	if ( m_pVertexBuffer ) {
		for ( auto i = 0; i < 4; ++i ) {
			hr = device_->DrawPrimitiveUP( D3DPT_TRIANGLEFAN, m_PrimCount, m_pVertexBuffer + ( m_PrimCount + 2 ) * i, sizeof( vertex_t ) );
			if ( FAILED( hr ) ) {
				return false;
			}
		}
	}

	return ( SUCCEEDED( hr ) );
}
void render_t::DrawLine( long Xa, long Ya, long Xb, long Yb, DWORD dwWidth, color Color ) {
}
auto render_t::render_triangle( vertex_t *vert, int count ) const -> void {
	this->device_->SetFVF( D3DFVF_XYZRHW | D3DFVF_DIFFUSE );
	this->device_->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, count, vert, sizeof( vertex_t ) );
}
auto render_t::render_lines( vertex_t *vert, int count ) const -> void {
	this->device_->SetFVF( D3DFVF_XYZRHW | D3DFVF_DIFFUSE );
	for ( auto i = 0; i < count; i++ )
		this->device_->DrawPrimitiveUP( D3DPT_LINESTRIP, count, vert, sizeof( vertex_t ) );
}

auto render_t::filled_rect( const float x, const float y, const float w, const float h, const color col ) const -> void {
	vertex_t vertices[ 4 ] = {
		{ x, y, 1.f, 1.0f, col },
		{ x + w, y, 1.f, 1.0f, col },
		{ x, y + h, 1.f, 1.0f, col },
		{ x + w, y + h, 1.f, 1.0f, col }
	};

	this->device_->SetFVF( D3DFVF_XYZRHW | D3DFVF_DIFFUSE );
	this->device_->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, &vertices, sizeof( vertex_t ) );
}

auto render_t::outlined_rect( const float x, const float y, float w, float h, const color col ) const -> void {
	w -= 1;
	h -= 1;
	vertex_t vertices[ 5 ] = {
		{ x, y, 1.0f, 1.0f, col },
		{ x + w, y, 1.0f, 1.0f, col },
		{ x + w, y + h, 1.0f, 1.0f, col },
		{ x, y + h, 1.0f, 1.0f, col },
		{ x, y, 1.0f, 1.0f, col }
	};

	this->device_->SetTexture( 0, nullptr );
	this->device_->SetFVF( D3DFVF_XYZRHW | D3DFVF_DIFFUSE );
	this->device_->DrawPrimitiveUP( D3DPT_LINESTRIP, 4, &vertices, sizeof( vertex_t ) );
}

auto render_t::line( const float x, const float y, const float x2, const float y2, const color color, const int dwWidth ) const -> void {
	static LPD3DXLINE line;
	if ( !line )
		D3DXCreateLine( device_, &line );
	D3DXVECTOR2 vLine[ 2 ]; // Two points
	line->SetAntialias( TRUE ); // To smooth edges

	line->SetWidth( dwWidth ); // Width of the line
	line->Begin( );

	vLine[ 0 ][ 0 ] = x; // Set points into array
	vLine[ 0 ][ 1 ] = y;
	vLine[ 1 ][ 0 ] = x2;
	vLine[ 1 ][ 1 ] = y2;

	line->Draw( vLine, 2, color ); // Draw with Line, number of lines, and color
	line->End( ); // finish
}

auto render_t::gradient( const float x, const float y, const float w, const float h, const color col, const color col2, const bool vertical ) const -> void {
	vertex_t vertices[ 4 ] = {
		{ x, y, 1.f, 1.0f, col },
		{ x + w, y, 1.f, 1.0f, (vertical ? col : col2) },
		{ x, y + h, 1.f, 1.0f, (vertical ? col2 : col) },
		{ x + w, y + h, 1.f, 1.0f, (col2) }
	};

	this->device_->SetFVF( D3DFVF_XYZRHW | D3DFVF_DIFFUSE );
	this->device_->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, vertices, sizeof( vertex_t ) );
}

auto render_t::push_clip( const float x, const float y, const float w, const float h ) -> void {
	RECT clip_rect = { static_cast< LONG >(x), static_cast< LONG >(y), static_cast< LONG >(x + w), static_cast< LONG >(y + h) };
	
	this->device_->SetScissorRect( &clip_rect );
	scissor_buffer.emplace( scissor_buffer.begin( ), clip_rect );
}

auto render_t::push_clip( area_t area ) -> void {
	RECT clip_rect = { static_cast< LONG >(area.x), static_cast< LONG >( area.y ), static_cast< LONG >( area.x + area.w ), static_cast< LONG >( area.y + area.h ) };

	this->device_->SetScissorRect( &clip_rect );
	scissor_buffer.emplace( scissor_buffer.begin( ), clip_rect );
}

auto render_t::pop_clip( ) -> void {
	scissor_buffer.erase( scissor_buffer.begin( ) );
	if ( !scissor_buffer.empty(  ) )
		this->device_->SetScissorRect( scissor_buffer.begin( )._Ptr );
	else
		this->device_->SetScissorRect( &this->backup_scissor_rect_ );
}

auto render_t::start( ) -> void {
	// setup state-block.
	this->device_->GetVertexDeclaration( &this->vertex_declaration_ );
	this->device_->CreateStateBlock( D3DSBT_PIXELSTATE, &this->state_block_ );

	// store.
	this->device_->GetRenderState( D3DRS_COLORWRITEENABLE, &this->dwold_d3drs_colorwriteenable_ );
	this->device_->GetRenderState( D3DRS_ANTIALIASEDLINEENABLE, &this->dwold_d3drs_antialiasedlineenable_ );
	this->device_->GetRenderState( D3DRS_MULTISAMPLEANTIALIAS, &this->dwold_d3drs_multisampleantialias_ );
	this->device_->GetRenderState( D3DRS_SCISSORTESTENABLE, &this->dwold_d3drs_scissortestenable_ );
	this->device_->GetTextureStageState( 0, D3DTSS_COLORARG0, &this->dwold_d3dtexturestagestate_ );
	this->device_->GetScissorRect( &this->backup_scissor_rect_);
	this->device_->GetVertexShader( &this->vertex_shader2_ );
	this->device_->GetPixelShader( &this->pixel_shader_ );
	this->device_->SetRenderState( D3DRS_COLORWRITEENABLE, 0xffffffff );
	this->device_->SetRenderState( D3DRS_SRGBWRITEENABLE, false );
	this->device_->SetSamplerState( NULL, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP );
	this->device_->SetSamplerState( NULL, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );
	this->device_->SetSamplerState( NULL, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP );
	this->device_->SetSamplerState( NULL, D3DSAMP_SRGBTEXTURE, NULL );
	this->device_->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );

	// setup.
	this->device_->SetVertexShader( nullptr );
	this->device_->SetPixelShader( nullptr );
	this->device_->SetRenderState( D3DRS_FOGENABLE, FALSE );
	this->device_->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	this->device_->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
	this->device_->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE );
	this->device_->SetRenderState( D3DRS_SCISSORTESTENABLE, TRUE );
	this->device_->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
	this->device_->SetRenderState( D3DRS_STENCILENABLE, FALSE );
	this->device_->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS, FALSE );
	this->device_->SetRenderState( D3DRS_ANTIALIASEDLINEENABLE, FALSE );
	this->device_->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	this->device_->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
	this->device_->SetRenderState( D3DRS_SEPARATEALPHABLENDENABLE, TRUE );
	this->device_->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	this->device_->SetRenderState( D3DRS_SRCBLENDALPHA, D3DBLEND_INVDESTALPHA );
	this->device_->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	this->device_->SetRenderState( D3DRS_DESTBLENDALPHA, D3DBLEND_ONE );
	this->device_->SetRenderState( D3DRS_SRGBWRITEENABLE, FALSE );
	this->device_->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA );
	this->device_->SetTexture( 0, nullptr );
}

auto render_t::finish( ) const -> void {
	// restore.
	this->device_->SetTextureStageState( 0, D3DTSS_COLORARG0, this->dwold_d3dtexturestagestate_ );
	this->device_->SetRenderState( D3DRS_COLORWRITEENABLE, this->dwold_d3drs_colorwriteenable_ );
	this->device_->SetRenderState( D3DRS_ANTIALIASEDLINEENABLE, this->dwold_d3drs_antialiasedlineenable_ );
	this->device_->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS, this->dwold_d3drs_multisampleantialias_ );
	this->device_->SetScissorRect( &this->backup_scissor_rect_ );
	this->device_->SetRenderState( D3DRS_SCISSORTESTENABLE, this->dwold_d3drs_scissortestenable_ );
	this->device_->SetRenderState( D3DRS_COLORWRITEENABLE, this->dwold_d3drs_colorwriteenable_ );
	this->device_->SetRenderState( D3DRS_SRGBWRITEENABLE, true );
	this->device_->SetVertexShader( this->vertex_shader2_ );
	this->device_->SetPixelShader( this->pixel_shader_ );

	// apply state-block.
	this->state_block_->Apply( );
	this->state_block_->Release( );
	this->device_->SetVertexDeclaration( this->vertex_declaration_ );
};

auto render_t::is_steam_overlay( ) const -> bool {
	static std::uintptr_t gameoverlay_return_address = 0;

	if ( !gameoverlay_return_address ) {
		MEMORY_BASIC_INFORMATION info;
		VirtualQuery( _ReturnAddress( ), &info, sizeof( MEMORY_BASIC_INFORMATION ) );

		char mod[ MAX_PATH ];
		GetModuleFileNameA( static_cast< HMODULE >( info.AllocationBase ), mod, MAX_PATH );

		if ( strstr( mod, "gameoverlay" ) )
			gameoverlay_return_address = reinterpret_cast< std::uintptr_t >( _ReturnAddress( ) );
	}

	return gameoverlay_return_address == reinterpret_cast< std::uintptr_t >( _ReturnAddress( ) );
}