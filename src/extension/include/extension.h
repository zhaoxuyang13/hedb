#pragma once
#include <defs.h>
#include <stdafx.h>
#include <enc_types.h>

#define DatumGetEncInt(X)  ((EncInt *) DatumGetPointer(X))
#define PG_GETARG_ENCINT(n) DatumGetEncInt(PG_GETARG_DATUM(n))
//variable length text. 
typedef struct varlena EncText;
