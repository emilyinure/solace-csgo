#pragma once
#include "utils.h"

class mem_alloc_t {
public:
    //virtual ~IMemAlloc();

    VFUNC( alloc( size_t nSize ), 1, void *( __thiscall * )( decltype( this ), size_t ), nSize );

    VFUNC( re_alloc( void *pMem, size_t nSize ), 3, void *( __thiscall * )( decltype( this ), size_t, void * ), nSize, pMem );

    VFUNC( free( void *pMem ), 5, void( __thiscall * )( decltype( this ), void * ), pMem );
};
