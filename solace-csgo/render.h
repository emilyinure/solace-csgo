#pragma once

#include <vector>
#include "vec3.h"

#include "d3d9.h"
#pragma comment(lib, "d3dx9.lib")
#include "d3dx9.h"

enum fontflags {
	fontflag_none = ( 1 << 0 ),
	fontflag_antialias = ( 1 << 1 ),
	fontflag_dropshadow = ( 1 << 2 ),
	fontflag_outline = ( 1 << 3 ),
	
};

class area_t {
public:
	float x = 0, y = 0, w = 0, h = 0;
	area_t( float x, float y, float w, float h ) : x( x ), y( y ), w( w ), h( h ) { }
	auto add( const area_t area, const bool add_w = false, const bool add_h = false ) -> area_t {
		this->x += area.x;
		this->y += area.y;

		if ( add_w )
			this->w += area.w;

		if ( add_h )
			this->h += area.h;

		return *this;
	}
};

class font_t {
public:
	void create ( IDirect3DDevice9 *device, const char *font_name, int size, int weight, int flags );

	int flags;
	LPD3DXFONT font;
};

class color {
	unsigned char _color[ 4 ];
public:
	color( ) = default;
	color( const int r, const int g, const int b, const int a = 255 ) {
		_color[ 0 ] = r;
		_color[ 1 ] = g;
		_color[ 2 ] = b;
		_color[ 3 ] = a;
	}

	auto m_r( ) const -> int {
		return _color[ 0 ];
	}

	auto m_g( ) const -> int {
		return _color[ 1 ];
	}

	auto m_b( ) const -> int {
		return _color[ 2 ];
	}

	auto m_a( ) const -> int {
		return _color[ 3 ];
	}

	auto set_r( const int r ) -> void {
		_color[ 0 ] = r;
	}

	auto set_g( const int g ) -> void {
		_color[ 1 ] = g;
	}

	auto set_b( const int b ) -> void {
		_color[ 2 ] = b;
	}

	auto set_a( const int a ) -> void {
		_color[ 3 ] = a;
	}

	operator const D3DCOLOR( ) const {
		return D3DCOLOR_RGBA( ( int )_color[ 0 ], ( int )_color[ 1 ], ( int )_color[ 2 ], ( int )_color[ 3 ] );
	}

	
};
enum Alignment {
	Vertical = ( 1 << 0 ),
	Horizontal = ( 1 << 1 )
};
class render_t {
	font_t tahoma_14_{ 0, nullptr };
	font_t segoe_ui_13_{ 0, nullptr };
	font_t courier_new_13_{ 0, nullptr };
	font_t tahoma_12_{ 0, nullptr }; 
	font_t constantia_12_{ 0, nullptr };

	IDirect3DDevice9 *device_{ nullptr };
	IDirect3DVertexDeclaration9 *vertex_declaration_{ nullptr };
    IDirect3DVertexBuffer9* vertex_buffer{nullptr};
    IDirect3DIndexBuffer9* index_buffer{nullptr};
	IDirect3DVertexShader9 *vertex_shader2_{ nullptr };
	IDirect3DPixelShader9 *pixel_shader_{ nullptr };
	DWORD dwold_d3drs_colorwriteenable_{ 0 };
	DWORD dwold_d3dtexturestagestate_{ 0 };
	DWORD dwold_d3drs_antialiasedlineenable_{ 0 };
	DWORD dwold_d3drs_multisampleantialias_{ 0 };
    DWORD dwold_d3drs_scissortestenable_{0};
    int32_t m_max_vtx_count = 1200;
    int32_t m_max_idx_count = 2400;
    

	std::vector<RECT> scissor_buffer = {};
	RECT backup_scissor_rect_{ 0, 0, 0, 0 };
	D3DVIEWPORT9 screen_size_{ 0, 0, 0, 0, 0, 0 };
	IDirect3DStateBlock9 *state_block_{ nullptr };

	auto create_fonts( ) -> void;


public:
	void on_lost_device() {
		if (tahoma_14_.font)
			tahoma_14_.font->OnLostDevice();

		if (segoe_ui_13_.font)
			segoe_ui_13_.font->OnLostDevice();

		if (courier_new_13_.font)
			courier_new_13_.font->OnLostDevice();

		if (tahoma_12_.font)
			tahoma_12_.font->OnLostDevice();

		if (constantia_12_.font)
			constantia_12_.font->OnLostDevice();

		
	    if (vertex_buffer)
        {
            vertex_buffer->Release();
            vertex_buffer = nullptr;
        }
        if (index_buffer)
        {
            index_buffer->Release();
            index_buffer = nullptr;
        }
	}

	void on_reset_device() {
		if (tahoma_14_.font)
			tahoma_14_.font->OnResetDevice();

		if (segoe_ui_13_.font)
			segoe_ui_13_.font->OnResetDevice();

		if (courier_new_13_.font)
			courier_new_13_.font->OnResetDevice();

		if (tahoma_12_.font)
			tahoma_12_.font->OnResetDevice();

		if (constantia_12_.font)
			constantia_12_.font->OnResetDevice();
	}
	struct vertex_t {
		float x, y, z, h;
		D3DCOLOR col;

		vertex_t( ) {
			this->x = 0.0f;
			this->y = 0.0f;
			this->z = 1.0f;
			this->h = 1.0f;
			this->col = D3DCOLOR_RGBA( 255, 255, 255, 255 );
		}

		vertex_t( float x_, float y_, float z_, float rhw_, D3DCOLOR color_ ) {
			this->x = x_;
			this->y = y_;
			this->z = z_;
			this->h = rhw_;
			this->col = color_;
		}
    };

    std::vector<vertex_t> m_verts = {};
    std::vector<uint16_t> m_idx = {};
	
	auto setup( IDirect3DDevice9 *device ) -> void;
	static RECT get_text_rect( const char* text, font_t& font );
	static int get_text_width( const char* text, font_t& font );
	static auto get_text_height( const char* text, font_t& font ) -> int;
	static auto text( font_t& font, float x, float y, color col, const char* text, const int centered = 0) -> void;
	bool draw_rounded_box( int x, int y, int width, int height, int precision, int offset_x, int offset_y, color col ) const;
	static void DrawLine ( long Xa, long Ya, long Xb, long Yb, DWORD dwWidth, color Color );
	void rounded ( int x, int y, int w, int h, int iSmooth, color Color );
	void rounded ( int x, int y, int w, int h, int iSmooth, D3DCOLOR Color );
	auto render_triangle( vertex_t* vert, int count ) const -> void;
	auto render_trianglefan( vertex_t* vert, int count ) const -> void;
	auto render_lines( vertex_t* vert, int count ) const -> void;
	auto filled_rect( const float x, const float y, const float w, const float h, const color col ) const -> void;
	auto outlined_rect( const float x, const float y, float w, float h, const color col ) const -> void;
	auto line( const float x, const float y, const float x2, const float y2, const color color, const int dwWidth = 1) const -> void;
	auto line( D3DXVECTOR2* lines, int count, const color color, const int dwWidth ) const -> void;
	auto gradient( const float x, const float y, const float w, const float h, const color col, const color col2, const bool vertical = 0 ) -> void;
	auto push_clip( const float x, const float y, const float w, const float h ) -> void;
	auto push_clip( area_t area ) -> void;
	auto pop_clip( ) -> void;
	auto start( ) -> void;
	auto finish( ) const -> void;
	auto world_circle( vec3_t origin, float radius, color clr ) const -> void;

	void Rounded( int x, int y, int w, int h, int iSmooth, color Color );

	auto circle( int x, int y, float radius, int segments, color clr ) const -> void;

	[[nodiscard]] auto is_steam_overlay( ) const -> bool;

	[[nodiscard]] font_t& m_tahoma_14( ) {
		return this->tahoma_14_;
	}

	[[nodiscard]] font_t& m_segoe_ui_13( ) {
		return this->segoe_ui_13_;
	}
	
	[[nodiscard]] font_t& m_courier_new_13( ) {
		return this->courier_new_13_;
	}

	[[nodiscard]] font_t& m_tahoma_12( ) {
		return this->tahoma_12_;
	}
	[[nodiscard]] font_t& m_constantia_12( ) {
		return this->constantia_12_;
	}

	[[nodiscard]] D3DVIEWPORT9& m_screen_size( ) {
		return this->screen_size_;
	}
};