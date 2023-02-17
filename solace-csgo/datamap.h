#include <string.h>

namespace std {
	enum class byte : unsigned char;
}

class quaternion_t;
class vec3_t;
class color;
// pre-declare.
class datamap_t;

// prototype.
using inputfunc_t = void( __cdecl * )( void *data );


typedef enum _fieldtypes {
	FIELD_VOID = 0,			// No type or value
	FIELD_FLOAT,			// Any floating point value
	FIELD_STRING,			// A string ID (return from ALLOC_STRING)
	FIELD_VECTOR,			// Any vector, QAngle, or AngularImpulse
	FIELD_QUATERNION,		// A quaternion
	FIELD_INTEGER,			// Any integer or enum
	FIELD_BOOLEAN,			// boolean, implemented as an int, I may use this as a hint for compression
	FIELD_SHORT,			// 2 byte integer
	FIELD_CHARACTER,		// a byte
	FIELD_COLOR32,			// 8-bit per channel r,g,b,a (32bit color)
	FIELD_EMBEDDED,			// an embedded object with a datadesc, recursively traverse and embedded class/structure based on an additional typedescription
	FIELD_CUSTOM,			// special type that contains function pointers to it's read/write/parse functions

	FIELD_CLASSPTR,			// CBaseEntity *
	FIELD_EHANDLE,			// Entity handle
	FIELD_EDICT,			// edict_t *

	FIELD_POSITION_VECTOR,	// A world coordinate (these are fixed up across level transitions automagically)
	FIELD_TIME,				// a floating point time (these are fixed up automatically too!)
	FIELD_TICK,				// an integer tick count( fixed up similarly to time)
	FIELD_MODELNAME,		// Engine string that is a model name (needs precache)
	FIELD_SOUNDNAME,		// Engine string that is a sound name (needs precache)

	FIELD_INPUT,			// a list of inputed data fields (all derived from CMultiInputVar)
	FIELD_FUNCTION,			// A class function pointer (Think, Use, etc)

	FIELD_VMATRIX,			// a vmatrix (output coords are NOT worldspace)

	// NOTE: Use float arrays for local transformations that don't need to be fixed up.
	FIELD_VMATRIX_WORLDSPACE,// A VMatrix that maps some local space to world space (translation is fixed up on level transitions)
	FIELD_MATRIX3X4_WORLDSPACE,	// matrix3x4_t that maps some local space to world space (translation is fixed up on level transitions)

	FIELD_INTERVAL,			// a start and range floating point interval ( e.g., 3.2->3.6 == 3.2 and 0.4 )
	FIELD_MODELINDEX,		// a model index
	FIELD_MATERIALINDEX,	// a material index (using the material precache string table)

	FIELD_VECTOR2D,			// 2 floats
	FIELD_INTEGER64,		// 64bit integer

	FIELD_VECTOR4D,			// 4 floats

	FIELD_TYPECOUNT,		// MUST BE LAST
} fieldtype_t;

enum {
	TD_OFFSET_NORMAL = 0,
	TD_OFFSET_PACKED = 1,
	TD_OFFSET_COUNT,
};

enum : int {
	PC_NON_NETWORKED_ONLY = 0,
	PC_NETWORKED_ONLY,

	PC_COPYTYPE_COUNT,
	PC_EVERYTHING = PC_COPYTYPE_COUNT,
};

class typedescription_t {
public:
	fieldtype_t				m_type;
	const char *m_name;
	int					fieldOffset;
	unsigned short			m_size;
	short					m_flags;
	const char *m_ext_name;
	void *m_save_restore_ops;
	inputfunc_t				m_input_func;
	datamap_t * m_td;
	int						m_bytes;
	typedescription_t *m_override_field;
	int						m_override_count;
	float					m_tolerance;
	int					flatOffset[ TD_OFFSET_COUNT ];
	unsigned short		flatGroup;
	typedescription_t( ) { };
	typedescription_t( fieldtype_t type, const char *name,
					   int						offset,
					   unsigned short			size,
					   short					flags,
					   const char *ext_name,
					   int						bytes,
					   float					tolerance ) : m_type( type ), m_name( name ), m_size( size ), m_flags( flags ),
		m_ext_name( ext_name ), m_save_restore_ops( nullptr ),
		m_input_func( nullptr ),
		m_bytes( bytes ),
		m_override_field( nullptr ),
		m_override_count( 0 ),
		m_tolerance( tolerance ) {
		fieldOffset = ( offset );
	}
};

struct optimized_datamap_t;
class datamap_t {
public:
	typedescription_t *m_desc;
	int					m_num_fields;
	char const *m_name;
	datamap_t *m_base;

	int					m_packed_size;
	optimized_datamap_t *m_pOptimizedDataMap;
	typedescription_t *find_var( const char *name ) const {
		if ( !m_num_fields || m_num_fields > 200 || !m_desc || !m_name )
			return nullptr;
		if ( m_num_fields > 0) {
			for ( int i{}; i < m_num_fields; ++i ) {
				typedescription_t *var = &m_desc[ i ];
				if ( !var->m_name )
					continue;
				if ( var->m_type == FIELD_EMBEDDED ) {
					if ( var->m_td ) {
						typedescription_t *offset;

						if ( ( offset = var->m_td->find_var( name )) != nullptr )
							return offset;
					}
				}
				else if ( strcmp( name, var->m_name ) == 0 ) {
					return var;
				}
			}
		}
		if ( m_base )
			return m_base->find_var( name );
		return nullptr;
	}
};


#define _FIELD(name,fieldtype,count,flags,mapname,tolerance, offset, size)		{ fieldtype, #name, offset, count, flags, mapname, NULL, NULL, NULL, size, NULL, 0, tolerance }
#define DEFINE_PRED_FIELD(name,fieldtype, flags, offset)			_FIELD(name, fieldtype, 1,  flags, NULL, 0.0f, offset, size )	
#define DECLARE_PREDICTABLE()											\
	public:																\
		static typedescription_t m_PredDesc[];							\
		static datamap_t m_PredMap;										\
		virtual datamap_t *GetPredDescMap( void );						\
		template <typename T> friend datamap_t *PredMapInit(T *)
#define BEGIN_PREDICTION_DATA( className ) \
	datamap_t className::m_PredMap = { 0, 0, #className, &BaseClass::m_PredMap }; \
	datamap_t *className::GetPredDescMap( void ) { return &m_PredMap; } \
	BEGIN_PREDICTION_DATA_GUTS( className )

#define BEGIN_PREDICTION_DATA_NO_BASE( className ) \
	datamap_t className::m_PredMap = { 0, 0, #className, NULL }; \
	datamap_t *className::GetPredDescMap( void ) { return &m_PredMap; } \
	BEGIN_PREDICTION_DATA_GUTS( className )

#define BEGIN_PREDICTION_DATA_GUTS( className ) \
	template <typename T> datamap_t *PredMapInit(T *); \
	template <> datamap_t *PredMapInit<className>( className * ); \
	namespace className##_PredDataDescInit \
	{ \
		datamap_t *g_PredMapHolder = PredMapInit( (className *)NULL ); /* This can/will be used for some clean up duties later */ \
	} \
	\
	template <> datamap_t *PredMapInit<className>( className * ) \
	{ \
		typedef className classNameTypedef; \
		static typedescription_t predDesc[] = \
		{ \
		{ FIELD_VOID,0,0,0,0,0,0,0,0}, /* so you can define "empty" tables */

#define END_PREDICTION_DATA() \
		}; \
		\
		if ( sizeof( predDesc ) > sizeof( predDesc[0] ) ) \
		{ \
			classNameTypedef::m_PredMap.dataNumFields = ARRAYSIZE( predDesc ) - 1; \
			classNameTypedef::m_PredMap.dataDesc 	  = &predDesc[1]; \
		} \
		else \
		{ \
			classNameTypedef::m_PredMap.dataNumFields = 1; \
			classNameTypedef::m_PredMap.dataDesc 	  = predDesc; \
		} \
		return &classNameTypedef::m_PredMap; \
	}
//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//



// Each datamap_t is broken down into two flattened arrays of fields, 
//  one for PC_NETWORKED_DATA and one for PC_NON_NETWORKED_ONLY (optimized_datamap_t::datamapinfo_t::flattenedoffsets_t)
// Each flattened array is sorted by offset for better cache performance
// Finally, contiguous "runs" off offsets are precomputed (optimized_datamap_t::datamapinfo_t::datacopyruns_t) for fast copy operations

// A data run is a set of DEFINE_PRED_FIELD fields in a c++ object which are contiguous and can be processing
//  using a single memcpy operation
//  

