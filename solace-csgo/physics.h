#pragma once
#include "includes.h"

struct surfacephysicsparams_t {
	float m_friction;
	float m_elasticity;
	float m_density;
	float m_thickness;
	float m_dampening;
};

struct surfaceaudioparams_t {
	float m_audio_reflectivity;
	float m_audio_hardness_factor;
	float m_audio_roughness_factor;
	float m_scrape_rough_threshold;
	float m_impact_hard_threshold;
	float m_audio_hard_min_velocity;
	float m_high_pitch_occlusion;
	float m_mid_pitch_occlusion;
	float m_low_pitch_occlusion;
};

struct surfacegameprops_t {
	float    m_max_speed_factor;
	float    m_jump_factor;
	float    m_penetration_modifier;
	float    m_damage_modifier;
	uint16_t m_material;
	uint8_t  m_climbable;
};

struct surfacesoundnames_t {
	short m_walk_left;
	short m_walk_right;
	short m_run_left;
	short m_run_right;
	short m_impact_soft;
	short m_impact_hard;
	short m_scrape_smooth;
	short m_scrape_rough;
	short m_bullet_impact;
	short m_rolling;
	short m_break_sound;
	short m_strain;
};

struct surfacedata_t {
    surfacephysicsparams_t physics;
    surfaceaudioparams_t audio;
    surfacesoundnames_t sounds;
    surfacegameprops_t game;
    char pad[ 48 ];
};

class phys_surface_props_t {
public:
    virtual ~phys_surface_props_t( void ) { }
    // parses a text file containing surface prop keys
    virtual int ParseSurfaceData( const char *pFilename, const char *pTextfile ) = 0;
    // current number of entries in the database
    virtual int SurfacePropCount( void ) const = 0;

    virtual int GetSurfaceIndex( const char *pSurfacePropName ) const = 0;
    virtual void GetPhysicsProperties( int surfaceDataIndex, float *density, float *thickness, float *friction,
                                       float *elasticity ) const = 0;

    virtual surfacedata_t *GetSurfaceData( int surfaceDataIndex ) = 0;
    virtual const char *GetString( unsigned short stringTableIndex ) const = 0;


    virtual const char *GetPropName( int surfaceDataIndex ) const = 0;

    // sets the global index table for world materials
    // UNDONE: Make this per-CPhysCollide
    virtual void SetWorldMaterialIndexTable( int *pMapArray, int mapSize ) = 0;

    // NOTE: Same as GetPhysicsProperties, but maybe more convenient
    virtual void GetPhysicsParameters( int surfaceDataIndex, surfacephysicsparams_t *pParamsOut ) const = 0;
};
