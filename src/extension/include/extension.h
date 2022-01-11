#pragma once
#include <defs.h>
#include <stdafx.h>
#include <enc_types.h>
#include <c.h>
// #include <extension_helper.h>
// #define DECLARE_PG_FUNC(func) extern "C" { PG_FUNCTION_INFO_V1(func); }

#define DatumGetEncInt(X)  ((EncInt *) DatumGetPointer(X))
#define PG_GETARG_ENCINT(n) DatumGetEncInt(PG_GETARG_DATUM(n))




//variable length text. 
typedef struct varlena EncText;
