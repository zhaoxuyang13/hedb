#pragma once

extern "C" {
#include <defs.h>
#include <stdafx.h>
#include <c.h>
}


#include "enc_ops.hpp"


#define DatumGetEncInt(X)  ((int *) DatumGetPointer(X))
#define PG_GETARG_ENCINT(n) DatumGetEncInt(PG_GETARG_DATUM(n))

#define DatumGetEncFloat(X)  ((float *) DatumGetPointer(X))
#define PG_GETARG_ENCFLOAT(n) DatumGetEncFloat(PG_GETARG_DATUM(n))

//variable length text. 
typedef struct varlena EncText;
#define PG_GETARG_ENCTEXT_P(n) ((EncText *)PG_DETOAST_DATUM(PG_GETARG_DATUM(n)))
