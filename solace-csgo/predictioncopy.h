#pragma once
#include "Windows.h"

#include "memory_shit.h"
class datamap_t;
class typedescription_t;

typedef void ( *FN_FIELD_COMPARE )( const char *classname, const char *fieldname, const char *fieldtype,
									bool networked, bool noterrorchecked, bool differs, bool withintolerance, const char *value );

class CPredictionCopy {
public:
	typedef enum {
		DIFFERS = 0,
		IDENTICAL,
		WITHINTOLERANCE,
	} difftype_t;

	typedef enum {
		TRANSFERDATA_COPYONLY = 0,  // Data copying only (uses runs)
		TRANSFERDATA_ERRORCHECK_NOSPEW, // Checks for errors, returns after first error found
		TRANSFERDATA_ERRORCHECK_SPEW,   // checks for errors, reports all errors to console
		TRANSFERDATA_ERRORCHECK_DESCRIBE, // used by hud_pdump, dumps values, etc, for all fields
	} optype_t;

	CPredictionCopy( int type, byte *dest, bool dest_packed, const byte *src, bool src_packed,
					 optype_t opType, FN_FIELD_COMPARE func = nullptr );

	int		TransferData( const char *operation, int entindex, datamap_t *dmap );

	static bool PrepareDataMap( datamap_t *dmap );

private:

	optype_t		m_OpType;
	int				m_nType;
	byte *m_pDest;
	const byte *m_pSrc;
	int				m_nDestOffsetIndex;
	int				m_nSrcOffsetIndex;
	int				m_nErrorCount;
	int				m_nEntIndex;

	FN_FIELD_COMPARE	m_FieldCompareFunc;

	const typedescription_t *m_pWatchField;
	char const *m_pOperation;

	CUtlStack< const typedescription_t * > m_FieldStack;
};

#undef CUtilStack
#undef CUtlMemory