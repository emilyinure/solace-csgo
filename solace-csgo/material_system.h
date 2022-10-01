#pragma once
#include "includes.h"

class vec3_t;

enum image_format {
	IMAGE_FORMAT_UNKNOWN = -1,
	IMAGE_FORMAT_RGBA8888 = 0,
	IMAGE_FORMAT_ABGR8888,
	IMAGE_FORMAT_RGB888,
	IMAGE_FORMAT_BGR888,
	IMAGE_FORMAT_RGB565,
	IMAGE_FORMAT_I8,
	IMAGE_FORMAT_IA88,
	IMAGE_FORMAT_P8,
	IMAGE_FORMAT_A8,
	IMAGE_FORMAT_RGB888_BLUESCREEN,
	IMAGE_FORMAT_BGR888_BLUESCREEN,
	IMAGE_FORMAT_ARGB8888,
	IMAGE_FORMAT_BGRA8888,
	IMAGE_FORMAT_DXT1,
	IMAGE_FORMAT_DXT3,
	IMAGE_FORMAT_DXT5,
	IMAGE_FORMAT_BGRX8888,
	IMAGE_FORMAT_BGR565,
	IMAGE_FORMAT_BGRX5551,
	IMAGE_FORMAT_BGRA4444,
	IMAGE_FORMAT_DXT1_ONEBITALPHA,
	IMAGE_FORMAT_BGRA5551,
	IMAGE_FORMAT_UV88,
	IMAGE_FORMAT_UVWQ8888,
	IMAGE_FORMAT_RGBA16161616F,
	IMAGE_FORMAT_RGBA16161616,
	IMAGE_FORMAT_UVLX8888,
	IMAGE_FORMAT_R32F,
	IMAGE_FORMAT_RGB323232F,
	IMAGE_FORMAT_RGBA32323232F,
	IMAGE_FORMAT_NV_DST16,
	IMAGE_FORMAT_NV_DST24,
	IMAGE_FORMAT_NV_INTZ,
	IMAGE_FORMAT_NV_RAWZ,
	IMAGE_FORMAT_ATI_DST16,
	IMAGE_FORMAT_ATI_DST24,
	IMAGE_FORMAT_NV_NULL,
	IMAGE_FORMAT_ATI2N,
	IMAGE_FORMAT_ATI1N,
	IMAGE_FORMAT_DXT1_RUNTIME,
	IMAGE_FORMAT_DXT5_RUNTIME,
	NUM_IMAGE_FORMATS
};

enum material_var_flags_t {
	material_var_debug = ( 1 << 0 ),
	material_var_no_debug_override = ( 1 << 1 ),
	material_var_no_draw = ( 1 << 2 ),
	material_var_use_in_fillrate_mode = ( 1 << 3 ),
	material_var_vertexcolor = ( 1 << 4 ),
	material_var_vertexalpha = ( 1 << 5 ),
	material_var_selfillum = ( 1 << 6 ),
	material_var_additive = ( 1 << 7 ),
	material_var_alphatest = ( 1 << 8 ),
	//material_var_unused = (1 << 9),
	material_var_znearer = ( 1 << 10 ),
	material_var_model = ( 1 << 11 ),
	material_var_flat = ( 1 << 12 ),
	material_var_nocull = ( 1 << 13 ),
	material_var_nofog = ( 1 << 14 ),
	material_var_ignorez = ( 1 << 15 ),
	material_var_decal = ( 1 << 16 ),
	material_var_envmapsphere = ( 1 << 17 ), // obsolete
	material_var_unused = ( 1 << 18 ), // unused
	material_var_envmapcameraspace = ( 1 << 19 ), // obsolete
	material_var_basealphaenvmapmask = ( 1 << 20 ),
	material_var_translucent = ( 1 << 21 ),
	material_var_normalmapalphaenvmapmask = ( 1 << 22 ),
	material_var_needs_software_skinning = ( 1 << 23 ), // obsolete
	material_var_opaquetexture = ( 1 << 24 ),
	material_var_envmapmode = ( 1 << 25 ), // obsolete
	material_var_suppress_decals = ( 1 << 26 ),
	material_var_halflambert = ( 1 << 27 ),
	material_var_wireframe = ( 1 << 28 ),
	material_var_allowalphatocoverage = ( 1 << 29 ),
	material_var_alpha_modified_by_proxy = ( 1 << 30 ),
	material_var_vertexfog = ( 1 << 31 ),
};

enum preview_image_retval_t {
	material_preview_image_bad = 0,
	material_preview_image_ok,
	material_no_preview_image,
};

typedef int ImageFormat;
class IMaterialVar;
typedef int VertexFormat_t;
typedef int MaterialPropertyTypes_t;
class i_material_var;
struct model_t;
class i_material;
class c_studio_hdr;
class c_key_values;
class i_material_var;
struct studiohwdata_t;
struct color_mesh_info_t;
struct draw_model_info_t;
class i_client_renderable;
class data_cache_handle_t;
class i_mat_render_context;
struct material_lighting_state_t;
typedef int vertex_format_t;
typedef void *light_cache_handle_t;
typedef void *studio_decal_handle_t;
typedef int material_property_types_t;
typedef unsigned short model_instance_handle_t;
using material_handle_t = unsigned short;

class i_material {
public:
	virtual const char *get_name( ) const = 0;
	virtual const char *get_texture_group_name( ) const = 0;
	virtual preview_image_retval_t get_preview_image_properties( int *width, int *height, image_format *image_format, bool *is_translucent ) const = 0;
	virtual preview_image_retval_t get_preview_image( unsigned char *data, int width, int height, image_format image_format ) const = 0;
	virtual int get_mapping_width( ) = 0;
	virtual int get_mapping_height( ) = 0;
	virtual int get_num_animation_frames( ) = 0;
	virtual bool in_material_page( void ) = 0;
	virtual void get_material_offset( float *offset ) = 0;
	virtual void get_material_scale( float *scale ) = 0;
	virtual i_material *get_material_page( void ) = 0;
	virtual i_material_var *find_var( const char *var_name, bool *found, bool complain = true ) = 0;
	virtual void increment_reference_count( void ) = 0;
	virtual void decrement_reference_count( void ) = 0;
	inline void add_ref( ) { increment_reference_count( ); }
	inline void release( ) { decrement_reference_count( ); }
	virtual int get_enumeration_id( void ) const = 0;
	virtual void get_low_res_color_sample( float s, float t, float *color ) const = 0;
	virtual void recompute_state_snapshots( ) = 0;
	virtual bool is_translucent( ) = 0;
	virtual bool is_alpha_tested( ) = 0;
	virtual bool is_vertex_lit( ) = 0;
	virtual vertex_format_t get_vertex_format( ) const = 0;
	virtual bool has_proxy( void ) const = 0;
	virtual bool uses_env_cubemap( void ) = 0;
	virtual bool needs_tangent_space( void ) = 0;
	virtual bool needs_power_of_two_frame_buffer_texture( bool check_specific_to_this_frame = true ) = 0;
	virtual bool needs_full_frame_buffer_texture( bool check_specific_to_this_frame = true ) = 0;
	virtual bool needs_software_skinning( void ) = 0;
	virtual void alpha_modulate( float alpha ) = 0;
	virtual void color_modulate( float r, float g, float b ) = 0;
	virtual void set_material_var_flag( material_var_flags_t flag, bool on ) = 0;
	virtual bool get_material_var_flag( material_var_flags_t flag ) const = 0;
	virtual void get_reflectivity( vec3_t &reflect ) = 0;
	virtual bool get_property_flag( material_property_types_t  type ) = 0;
	virtual bool is_two_sided( ) = 0;
	virtual void set_shader( const char *shader_name ) = 0;
	virtual int get_num_passes( void ) = 0;
	virtual int get_texture_memory_bytes( void ) = 0;
	virtual void refresh( ) = 0;
	virtual bool needs_lightmap_blend_alpha( void ) = 0;
	virtual bool needs_software_lighting( void ) = 0;
	virtual int shader_param_count( ) const = 0;
	virtual i_material_var **get_shader_params( void ) = 0;
	virtual bool is_error_material( ) const = 0;
	virtual void unused( ) = 0;
	virtual float get_alpha_modulation( ) = 0;
	virtual void get_color_modulation( float *r, float *g, float *b ) = 0;
	virtual bool is_translucent_under_modulation( float alpha_modulation = 1.0f ) const = 0;
	virtual i_material_var *find_var_fast( char const *var_name, unsigned int *token ) = 0;
	virtual void set_shader_and_params( c_key_values *key_values ) = 0;
	virtual const char *get_shader_name( ) const = 0;
	virtual void delete_if_unreferenced( ) = 0;
	virtual bool is_sprite_card( ) = 0;
	virtual void call_bind_proxy( void *proxy_data ) = 0;
	virtual void refresh_preserving_material_vars( ) = 0;
	virtual bool was_reloaded_from_whitelist( ) = 0;
	virtual bool set_temp_excluded( bool set, int excluded_dimension_limit ) = 0;
	virtual int get_reference_count( ) const = 0;
};


#define DECLARE_POINTER_HANDLE(name) struct name##__ { int unused; }; typedef struct name##__ *name
#define MAXSTUDIOSKINS		32

// These are given to FindMaterial to reference the texture groups that Show up on the
#define TEXTURE_GROUP_LIGHTMAP				"Lightmaps"
#define TEXTURE_GROUP_WORLD				"World textures"
#define TEXTURE_GROUP_MODEL				"Model textures"
#define TEXTURE_GROUP_VGUI				"VGUI textures"
#define TEXTURE_GROUP_PARTICLE				"Particle textures"
#define TEXTURE_GROUP_DECAL				"Decal textures"
#define TEXTURE_GROUP_SKYBOX				"SkyBox textures"
#define TEXTURE_GROUP_CLIENT_EFFECTS			"ClientEffect textures"
#define TEXTURE_GROUP_OTHER				"Other textures"
#define TEXTURE_GROUP_PRECACHED				"Precached"
#define TEXTURE_GROUP_CUBE_MAP				"CubeMap textures"
#define TEXTURE_GROUP_RENDER_TARGET			"RenderTargets"
#define TEXTURE_GROUP_UNACCOUNTED			"Unaccounted textures"
//#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER		"Static Vertex"
#define TEXTURE_GROUP_STATIC_INDEX_BUFFER		"Static Indices"
#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER_DISP		"Displacement Verts"
#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER_COLOR	"Lighting Verts"
#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER_WORLD	"World Verts"
#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER_MODELS	"Model Verts"
#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER_OTHER	"Other Verts"
#define TEXTURE_GROUP_DYNAMIC_INDEX_BUFFER		"Dynamic Indices"
#define TEXTURE_GROUP_DYNAMIC_VERTEX_BUFFER		"Dynamic Verts"
#define TEXTURE_GROUP_DEPTH_BUFFER			"DepthBuffer"
#define TEXTURE_GROUP_VIEW_MODEL			"ViewModel"
#define TEXTURE_GROUP_PIXEL_SHADERS			"Pixel Shaders"
#define TEXTURE_GROUP_VERTEX_SHADERS			"Vertex Shaders"
#define TEXTURE_GROUP_RENDER_TARGET_SURFACE		"RenderTarget Surfaces"
#define TEXTURE_GROUP_MORPH_TARGETS			"Morph Targets"

class	i_ms_mat;
class	i_ms_mesh;
class	i_ms_vertex_buffer;
class	i_ms_index_buffer;
struct	i_ms_system_config_t;
class	i_ms_v_matrix;
class	matrix3x4_t;
class	i_ms_texture;
struct	i_ms_hwid_t;
class	i_ms_key_values;
class	i_ms_shader;
class	i_ms_vtx_texture;
class	i_ms_morph;
class	i_mat_render_ctx;
class	i_ms_call_queue;
struct	i_ms_morph_weight_t;
class	i_ms_file_list;
struct	i_ms_vtx_stream_spec_t;
struct	i_ms_shader_stencil_state_t;
struct	i_ms_mesh_instance_data_t;
class	i_ms_client_mat_sys;
class	i_ms_paint_mat;
class	i_ms_paint_map_data_mgr;
class	i_ms_paint_map_texture_mgr;
class	i_ms_gpu_mem_stats;
struct	i_ms_aspect_ratio_info_t;
struct	i_ms_cascaded_shadow_mapping_state_t;

class	i_ms_proxy_factory;
class	i_ms_texture;
class	i_ms_sys_hardware_cfg;
class	i_ms_shadow_mgr;

enum i_ms_compiled_vtf_flags {
	TEXTURE_FLAGS_POINT_SAMPLE = 1 << 0,
	TEXTURE_FLAGS_TRILINEAR = 1 << 1,
	TEXTURE_FLAGS_CLAMPS = 1 << 2,
	TEXTURE_FLAGS_CLAMPT = 1 << 3,
	TEXTURE_FLAGS_ANISOTROPIC = 1 << 4,
	TEXTURE_FLAGS_HINT_DXT5 = 1 << 5,
	TEXTURE_FLAGS_PWL_CORRECTED = 1 << 6,
	TEXTURE_FLAGS_NORMAL = 1 << 7,
	TEXTURE_FLAGS_NO_MIP = 1 << 8,
	TEXTURE_FLAGS_NO_LOD = 1 << 9,
	TEXTURE_FLAGS_ALL_MIPS = 1 << 10,
	TEXTURE_FLAGS_PROCEDURAL = 1 << 11,
	TEXTURE_FLAGS_ONE_BIT_ALPHA = 1 << 12,
	TEXTURE_FLAGS_EIGHT_BIT_ALPHA = 1 << 13,
	TEXTURE_FLAGS_ENVMAP = 1 << 14,
	TEXTURE_FLAGS_RENDER_TARGET = 1 << 15,
	TEXTURE_FLAGS_DEPTH_RENDER_TARGET = 1 << 16,
	TEXTURE_FLAGS_NO_DEBUG_OVERRIDE = 1 << 17,
	TEXTURE_FLAGS_SINGLE_COPY = 1 << 18,
	TEXTURE_FLAGS_PRE_SRGB = 1 << 19,
	TEXTURE_FLAGS_UNUSED_0x001 = 1 << 20,
	TEXTURE_FLAGS_UNUSED_0x002 = 1 << 21,
	TEXTURE_FLAGS_UNUSED_0x004 = 1 << 22,
	TEXTURE_FLAGS_NO_DEPTH_BUFFER = 1 << 23,
	TEXTURE_FLAGS_UNUSED_0x01 = 1 << 24,
	TEXTURE_FLAGS_CLAMPU = 1 << 25,
	TEXTURE_FLAGS_VERTEX_TEXTURE = 1 << 26,
	TEXTURE_FLAGS_SSBUMP = 1 << 27,
	TEXTURE_FLAGS_UNUSED_0x1 = 1 << 28,
	TEXTURE_FLAGS_BORDER = 1 << 29,
	TEXTURE_FLAGS_UNUSED_0x4 = 1 << 30,
	TEXTURE_FLAGS_UNUSED_0x8 = 1 << 31
};

enum i_ms_standard_lightmap_t {
	MATERIAL_SYSTEM_LIGHTMAP_PAGE_WHITE = -1,
	MATERIAL_SYSTEM_LIGHTMAP_PAGE_WHITE_BUMP = -2,
	MATERIAL_SYSTEM_LIGHTMAP_PAGE_USER_DEFINED = -3
};

struct i_ms_sort_info_t {
	i_ms_mat *material;
	int		lightmap_page_id;
};

enum i_matsys_material_thread_mode_t {
	MATERIAL_SINGLE_THREADED,
	MATERIAL_QUEUED_SINGLE_THREADED,
	MATERIAL_QUEUED_THREADED
};

enum i_ms_material_ctx_type_t {
	MATERIAL_HARDWARE_CONTEXT,
	MATERIAL_QUEUED_CONTEXT,
	MATERIAL_NULL_CONTEXT
};

enum {
	MATERIAL_ADAPTER_NAME_LENGTH = 1 << 9
};

struct i_ms_material_texture_info_t {
	int exclude_information;
};

struct i_ms_app_perf_counters_info_t {
	float ms_main;
	float ms_mst;
	float ms_gpu;
	float ms_flip;
	float ms_total;
};

struct i_ms_app_instant_counters_info_t {
	uint32_t cpu_activity_mask;
	uint32_t deferred_words_allocated;
};

struct i_matsys_material_adapter_info_t {
	char		driver_name[ MATERIAL_ADAPTER_NAME_LENGTH ];
	unsigned int	vendor_id;
	unsigned int	device_id;
	unsigned int	sub_sys_id;
	unsigned int	revision;
	int		dx_support_level;
	int		min_dx_support_level;
	int		max_dx_support_level;
	unsigned int	driver_version_high;
	unsigned int	driver_version_low;
};

struct i_ms_mat_video_mode_t {
	int		width;
	int		height;
	image_format 	format;
	int		refresh_rate;
};

enum i_ms_hdr_type_t {
	HDR_TYPE_NONE,
	HDR_TYPE_INTEGER,
	HDR_TYPE_FLOAT,
};

enum i_ms_res_change_flags_t {
	MATERIAL_RESTORE_VERTEX_FORMAT_CHANGED = 1,
	MATERIAL_RESTORE_RELEASE_MANAGED_RESOURCES
};

enum i_ms_render_target_size_mode_t {
	RT_SIZE_NO_CHANGE,
	RT_SIZE_DEFAULT,
	RT_SIZE_PICMIP,
	RT_SIZE_HDR,
	RT_SIZE_FULL_FRAME_BUFFER,
	RT_SIZE_OFFSCREEN,
	RT_SIZE_FULL_FRAME_BUFFER_ROUNDED_UP
};

enum i_ms_mat_render_target_depth_t {
	MATERIAL_RT_DEPTH_SHARED,
	MATERIAL_RT_DEPTH_SEPARATE,
	MATERIAL_RT_DEPTH_NONE,
	MATERIAL_RT_DEPTH_ONLY
};

typedef void( *mat_buffer_release_func_t )( int flags );
typedef void( *mat_buffer_restore_func_t )( int flags );
typedef void( *mode_change_callback_function_t )( void );
typedef void( *end_frame_cleanup_function_t )( void );
typedef bool( *end_frame_prior_to_next_ctx_function_t )( void );
typedef void( *on_level_shutdown_function_t )( void *data );

typedef unsigned short mat_handle_t;
DECLARE_POINTER_HANDLE( mat_lock_t );

class i_material_system {
public:
	i_material *find_material( char const *material_name, const char *group_name, bool complain = true, const char *complain_prefix = 0 ) {
		using fn = i_material * ( __thiscall * )( i_material_system *, char const *, const char *, bool, const char * );
		return ( *( fn ** )this )[ 84 ]( this, material_name, group_name, complain, complain_prefix );
	}
	material_handle_t first_material( ) {
		using fn = material_handle_t( __thiscall * )( i_material_system * );
		return ( *( fn ** )this )[ 86 ]( this );
	}
	material_handle_t next_material( material_handle_t handle ) {
		using fn = material_handle_t( __thiscall * )( i_material_system *, material_handle_t );
		return ( *( fn ** )this )[ 87 ]( this, handle );
	}
	material_handle_t invalid_material_handle( ) {
		using fn = material_handle_t( __thiscall * )( i_material_system * );
		return ( *( fn ** )this )[ 88 ]( this );
	}
	i_material *get_material( material_handle_t handle ) {
		using fn = i_material * ( __thiscall * )( i_material_system *, material_handle_t );
		return ( *( fn ** )this )[ 89 ]( this, handle );
	}
	int	get_materials_count( ) {
		using fn = int( __thiscall * )( i_material_system * );
		return ( *( fn ** )this )[ 90 ]( this );
	}
};


class iv_model_render {
public:
	void override_material( i_material *material ) {
		using fn = void( __thiscall * )( iv_model_render *, i_material *, int, int );
		return ( *( fn ** )this )[ 1 ]( this, material, 0, 0 );
	}
};


class i_render_view {
private:
	virtual void __pad0( );
	virtual void __pad1( );
	virtual void __pad2( );
	virtual void __pad3( );

public:
	virtual void set_blend( float blend ) = 0;
	virtual float get_blend( void ) = 0;

	virtual void SetColorModulation( float const *blend ) = 0;
	virtual void get_color_modulation( float *blend ) = 0;

	__forceinline void modulate_color( color col ) {
		float color[ ] = { ( float )col.m_r(  ) / 255,  ( float )col.m_g(  ) / 255, ( float )col.m_b(  ) / 255,  1.f };
		SetColorModulation( color );
	}
};