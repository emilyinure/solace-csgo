#include "predictioncopy.h"

#include "datamap.h"
#include "utils.h"

CPredictionCopy::CPredictionCopy( int type, byte *dest, bool dest_packed, const byte *src, bool src_packed,
                                  optype_t opType, FN_FIELD_COMPARE func /*= NULL*/ ) {
	m_OpType = opType;
	m_nType = type;
	m_pDest = dest;
	m_pSrc = src;
	m_nDestOffsetIndex = dest_packed ? TD_OFFSET_PACKED : TD_OFFSET_NORMAL;
	m_nSrcOffsetIndex = src_packed ? TD_OFFSET_PACKED : TD_OFFSET_NORMAL;

	m_nErrorCount = 0;
	m_nEntIndex = -1;

	m_pWatchField = nullptr;
	m_FieldCompareFunc = func;
}

int CPredictionCopy::TransferData( const char *operation, int entindex, datamap_t *dmap ) {
	static auto pat = util::find( "client.dll", "55 8B EC 8B 45 10 53 56 8B F1 57" );
	using PrepDataMap = int( __thiscall * )( CPredictionCopy *, const char *, int, datamap_t * );
	return reinterpret_cast< PrepDataMap >( pat )( this, operation, entindex, dmap );
}

bool CPredictionCopy::PrepareDataMap( datamap_t *dmap ) {
	dmap->m_packed_size = 0;
	static auto pat = util::find( "client.dll", "55 8B EC 83 EC ? 57 8B F9 89 7D ? 83 7F ? ?" );
	using PrepDataMap = bool( __thiscall * )( datamap_t * );
	return reinterpret_cast< PrepDataMap >( pat )( dmap );
}
