#pragma once
#include "includes.h"

struct model_t {
	void *handle;
	char	name[ 260 ];
	int	load_flags;
	int	server_count;
	int	type;
	int	flags;
	vec3_t	vec_mins;
	vec3_t	vec_maxs;
	float	radius;
};

typedef unsigned short ModelInstanceHandle_t;
struct model_render_info_t {
	vec3_t                  origin;
	ang_t                  angles;
	void *pRenderable;
	const model_t *pModel;
	const matrix_t *pModelToWorld;
	const matrix_t *pLightingOffset;
	const vec3_t *pLightingOrigin;
	int                     flags;
	int                     entity_index;
	int                     skin;
	int                     body;
	int                     hitboxset;
	ModelInstanceHandle_t   instance;

	model_render_info_t( ) {
		pModelToWorld = NULL;
		pLightingOffset = NULL;
		pLightingOrigin = NULL;
	}
};

class i_mdl_cache {
public:
	void begin_lock() {
		using original_fn = void( __thiscall * )( i_mdl_cache * );
		return ( *( original_fn ** )this )[ 32 ]( this );
	}
	void end_lock() {
		using original_fn = void(__thiscall*)(i_mdl_cache*);
		return (*(original_fn**)this)[33](this);
	}
	void begin_coarse_lock() {
		using original_fn = void(__thiscall*)(i_mdl_cache*);
		return (*(original_fn**)this)[34](this);
	}
	void end_coarse_lock() {
		using original_fn = void(__thiscall*)(i_mdl_cache*);
		return (*(original_fn**)this)[35](this);
	}
};

class studio_hdr_t;

class model_info_t {
public:
	model_t *get_model( int index ) {
		using original_fn = model_t * ( __thiscall * )( model_info_t *, int );
		return ( *( original_fn ** )this )[ 1 ]( this, index );
	}
	int get_model_index( const char *filename ) {
		using original_fn = int( __thiscall * )( model_info_t *, const char * );
		return ( *( original_fn ** )this )[ 2 ]( this, filename );
	}
	const char *get_model_name( const model_t *model ) {
		using original_fn = const char *( __thiscall * )( model_info_t *, const model_t * );
		return ( *( original_fn ** )this )[ 3 ]( this, model );
	}
	studio_hdr_t *get_studio_model( const model_t *model ) {
		using original_fn = studio_hdr_t * ( __thiscall * )( model_info_t *, const model_t * );
		return ( *( original_fn ** )this )[ 30 ]( this, model );
	}
};