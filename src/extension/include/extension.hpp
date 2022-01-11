#pragma once
#include <defs.h>
#include <stdafx.hpp>
#include <enc_types.h>

#define DatumGetEncInt(X)  ((EncInt *) DatumGetPointer(X))
#define PG_GETARG_ENCINT(n) DatumGetEncInt(PG_GETARG_DATUM(n))

#define DatumGetEncFloat(X)  ((EncFloat *) DatumGetPointer(X))
#define PG_GETARG_ENCFlOAT(n) DatumGetEncFloat(PG_GETARG_DATUM(n))

#define DatumGetEncTimestamp(X)  ((EncTimestamp *) DatumGetPointer(X))
#define PG_GETARG_ENCTimestamp(n) DatumGetEncTimestamp(PG_GETARG_DATUM(n))

#define DatumGetEncStr(X)  ((EncStr *) DatumGetPointer(X))
#define PG_GETARG_ENCStr(n) DatumGetEncStr(PG_GETARG_DATUM(n))

//variable length text. 
typedef struct varlena EncText;
