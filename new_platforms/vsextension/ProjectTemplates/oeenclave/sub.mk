# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.
include $(NEW_PLATFORMS_PATH)oe_sub.mk

global-incdirs-y += $(NEW_PLATFORMS_PATH)Inc
global-incdirs-y += optee

# Add any additional include directories here
#global-incdirs-y += ...

../$projectname$_t.c: ../../$projectname$.edl
	$(OEEDGER8R) --trusted --trusted-dir .. --search-path "$(NEW_PLATFORMS_PATH)Inc$(OEPATHSEP)$(OE_SDK_ROOT_PATH)include" ../../$projectname$.edl

../$projectname$_t.h: ../../$projectname$.edl
	$(OEEDGER8R) --trusted --trusted-dir .. --search-path "$(NEW_PLATFORMS_PATH)Inc$(OEPATHSEP)$(OE_SDK_ROOT_PATH)include" ../../$projectname$.edl

# Add the c file generated from your EDL file here
srcs-y             += ../$projectname$_t.c

# Add additional sources here
srcs-y             += ../enc.c

# Add additional libraries here
libdirs            += $(NEW_PLATFORMS_PATH)bin/optee/new_platforms
libnames           += oestdio_enc
libdeps            += $(NEW_PLATFORMS_PATH)bin/optee/new_platforms/liboestdio_enc.a
#
# libnames         += ...
# libdirs          += ...
# libdeps          += ...
