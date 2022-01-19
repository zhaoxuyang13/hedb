ENC_OPS := $(shell find ops -name '*.c')
TOOLS := $(shell find ../tools -name "*.c")
global-incdirs-y += include ../include  
srcs-y += ops_ta.c $(ENC_OPS) $(TOOLS) 
# libdirs += ../mbedtls/lib
# libnames += mbedcrypto
# To remove a certain compiler flag, add a line like this
#cflags-template_ta.c-y += -Wno-strict-prototypes
# cflags-ops_ta.c-y += -pg
# link math
# libnames += timer stdc++
# libdirs += ../host 
libnames += mbedtls
