#* @(#) pSOSystem PowerPC/V2.5.0: drivers/slip/rules_ppc.mk 4.3 98/10/08 11:16:36 *
#*********************************************************************** 
#*                                                                     * 
#*   MODULE:  drivers/slip/rules.mk                                    * 
#*   DATE:    98/10/08                                                 * 
#*   PURPOSE: Rules for making slip driver                             * 
#*                                                                     * 
#*---------------------------------------------------------------------* 
#*                                                                     * 
#*        Copyright 1991 - 1998, Integrated Systems, Inc.              * 
#*                      ALL RIGHTS RESERVED                            * 
#*                                                                     * 
#*   Permission is hereby granted to licensees of Integrated Systems,  * 
#*   Inc. products to use or abstract this computer program for the    * 
#*   sole purpose of implementing a product based on Integrated        * 
#*   Systems, Inc. products.   No other rights to reproduce, use,      * 
#*   or disseminate this computer program, whether in part or in       * 
#*   whole, are granted.                                               * 
#*                                                                     * 
#*   Integrated Systems, Inc. makes no representation or warranties    * 
#*   with respect to the performance of this computer program, and     * 
#*   specifically disclaims any responsibility for any damages,        * 
#*   special or consequential, connected with the use of this program. * 
#*                                                                     * 
#*---------------------------------------------------------------------* 
#*                                                                     * 
#*                                                                     * 
#*                                                                     * 
#*********************************************************************** 
LIB_H = include
LIB_H_R = include
LIB_INC   = $(PSS_ROOT)/$(LIB_H)
LIB_INC_R   = $(PSS_ROOT)/$(LIB_H_R)

OBJ_DIR = o
LIB_DIR=rip
LIB_NAME = librip
LIB_NAME_VRF = librip_vrf
LIB_TARGET=librip.a
LIB_TARGET_VRF=mpls_rip.a

S         = $(PSS_ROOT)/sys/$(LIB_DIR)
APP_DIR   = $(PSS_ROOT)/apps/router

# long word alignment for compression algorithm
#CFLAGS  = -Xstruct-best-align=0 -Xstrict-ansi -Xforce-prototypes -Xlint \
#	-Xinit-locals -Xinit-value=165 -DROUTING_MODULE
# CFLAGS  = -Xstruct-best-align=0

CFLAGS  = -Xstruct-best-align=0 -Xkeywords=0xC \
			-DKERNEL -DNBNS -DROUTING_MODULE -DRSVP_MODULE -DSOCKBUF_DEBUG \
			-Xstrict-ansi -Xforce-prototypes -Xstack-probe \
			-Xno-optimized-debug -Xinit-locals -Xinit-value=165 \
			-I$(PSS_ROOT)/include

CFLAGS_VRF  = -Xstruct-best-align=0 -Xkeywords=0xC \
			-DKERNEL -DNBNS -DROUTING_MODULE -DRSVP_MODULE -DSOCKBUF_DEBUG -DVRF_MODULE \
			-Xstrict-ansi -Xforce-prototypes -Xstack-probe \
			-Xno-optimized-debug -Xinit-locals -Xinit-value=165 \
			-I$(PSS_ROOT)/include


RT_INCLUDES =  $(LIB_INC_R)/routing
RIP_INCLUDES =  $(LIB_INC_R)/rip

INCLUDES =  -DROUTING_MODULE \
	-I$(LIB_INC) \
	-I$(LIB_INC)/ip \
	-I$(LIB_INC_R) \
	-I$(RT_INCLUDES) \
	-I$(RIP_INCLUDES) 
	

SOPTS   = -@$(APP_DIR)/c.opt $(INCLUDES) -@$(S)/$(OBJ_DIR)/$(LIB_DIR).opt
SOPTS_VRF   = -@$(APP_DIR)/c.opt $(INCLUDES) -@$(S)/$(OBJ_DIR)/$(LIB_DIR)VRF.opt

# exported

#------------------------------------------------------------------------
# all compilation time defines
#------------------------------------------------------------------------

LIBRIP_VRF_OBJS = $(OBJ_DIR)/rip_main_vrf.o \
	$(OBJ_DIR)/rip_init_vrf.o \
	$(OBJ_DIR)/rip_cmd_vrf.o \
	$(OBJ_DIR)/rip_debug_vrf.o \
	$(OBJ_DIR)/rip_show_vrf.o \
	$(OBJ_DIR)/rip_intf_vrf.o \
	$(OBJ_DIR)/rip_packet_vrf.o \
	$(OBJ_DIR)/rip_timer_vrf.o \
	$(OBJ_DIR)/rip_redis_vrf.o \
	$(OBJ_DIR)/rip_mib_vrf.o \
	$(OBJ_DIR)/rip_version_vrf.o \

LIBRIP_OBJS = $(OBJ_DIR)/rip_main.o \
	$(OBJ_DIR)/rip_init.o \
	$(OBJ_DIR)/rip_cmd.o \
	$(OBJ_DIR)/rip_debug.o \
	$(OBJ_DIR)/rip_show.o \
	$(OBJ_DIR)/rip_intf.o \
	$(OBJ_DIR)/rip_packet.o \
	$(OBJ_DIR)/rip_timer.o \
	$(OBJ_DIR)/rip_redis.o \
	$(OBJ_DIR)/rip_mib.o \
	$(OBJ_DIR)/rip_version.o \

all: $(LIB_TARGET) $(LIB_TARGET_VRF)
#all: $(LIB_TARGET)
#all: $(LIB_TARGET_VRF)

$(LIB_TARGET): $(OBJ_DIR)/$(LIB_DIR).opt \
		$(LIBRIP_OBJS) \
        rules.mk \
        makefile
		@rm -f $@
		$(LIB) $(LIBOPTS) $@ $(LIBRIP_OBJS)
		$(LIB) $(LIBOPTS_SORT) $@
		@cp $@ $(APP_DIR)/lib/librip.a

$(LIB_TARGET_VRF): $(OBJ_DIR)/$(LIB_DIR)VRF.opt \
		$(LIBRIP_VRF_OBJS) \
        rules.mk \
        makefile
		@rm -f $@
		$(LIB) $(LIBOPTS) $@ $(LIBRIP_VRF_OBJS)
		$(LIB) $(LIBOPTS_SORT) $@
		@cp $@ $(APP_DIR)/lib/mpls_rip.a

$(OBJ_DIR)/rip_main.o: rip_main.c \
	makefile $(S)/rules.mk
	$(CC) $(SOPTS) -o $@ $<

$(OBJ_DIR)/rip_init.o: rip_init.c \
	makefile $(S)/rules.mk
	$(CC) $(SOPTS) -o $@ $<

$(OBJ_DIR)/rip_cmd.o: rip_cmd.c \
	makefile $(S)/rules.mk
	$(CC) $(SOPTS) -o $@ $<

$(OBJ_DIR)/rip_debug.o: rip_debug.c \
	makefile $(S)/rules.mk
	$(CC) $(SOPTS) -o $@ $<

$(OBJ_DIR)/rip_show.o: rip_show.c \
	makefile $(S)/rules.mk
	$(CC) $(SOPTS) -o $@ $<

$(OBJ_DIR)/rip_intf.o: rip_intf.c \
	makefile $(S)/rules.mk
	$(CC) $(SOPTS) -o $@ $<

$(OBJ_DIR)/rip_packet.o: rip_packet.c \
	makefile $(S)/rules.mk
	$(CC) $(SOPTS) -o $@ $<

$(OBJ_DIR)/rip_timer.o: rip_timer.c \
	makefile $(S)/rules.mk
	$(CC) $(SOPTS) -o $@ $<

$(OBJ_DIR)/rip_redis.o: rip_redis.c \
	makefile $(S)/rules.mk
	$(CC) $(SOPTS) -o $@ $<

$(OBJ_DIR)/rip_mib.o: rip_mib.c \
	makefile $(S)/rules.mk
	$(CC) $(SOPTS) -o $@ $<
	
$(OBJ_DIR)/rip_version.o: rip_version.c \
	makefile $(S)/rules.mk
	$(CC) $(SOPTS) -o $@ $<

$(OBJ_DIR)/$(LIB_DIR).opt: $(APP_DIR)/c.opt
	@echo $(CFLAGS) > $@
	
$(OBJ_DIR)/rip_main_vrf.o: rip_main.c \
	makefile $(S)/rules.mk
	$(CC) $(SOPTS_VRF) -o $@ $<

$(OBJ_DIR)/rip_init_vrf.o: rip_init.c \
	makefile $(S)/rules.mk
	$(CC) $(SOPTS_VRF) -o $@ $<

$(OBJ_DIR)/rip_cmd_vrf.o: rip_cmd.c \
	makefile $(S)/rules.mk
	$(CC) $(SOPTS_VRF) -o $@ $<

$(OBJ_DIR)/rip_debug_vrf.o: rip_debug.c \
	makefile $(S)/rules.mk
	$(CC) $(SOPTS_VRF) -o $@ $<

$(OBJ_DIR)/rip_show_vrf.o: rip_show.c \
	makefile $(S)/rules.mk
	$(CC) $(SOPTS_VRF) -o $@ $<
	
$(OBJ_DIR)/rip_intf_vrf.o: rip_intf.c \
	makefile $(S)/rules.mk
	$(CC) $(SOPTS_VRF) -o $@ $<

$(OBJ_DIR)/rip_packet_vrf.o: rip_packet.c \
	makefile $(S)/rules.mk
	$(CC) $(SOPTS_VRF) -o $@ $<

$(OBJ_DIR)/rip_timer_vrf.o: rip_timer.c \
	makefile $(S)/rules.mk
	$(CC) $(SOPTS_VRF) -o $@ $<

$(OBJ_DIR)/rip_redis_vrf.o: rip_redis.c \
	makefile $(S)/rules.mk
	$(CC) $(SOPTS_VRF) -o $@ $<

$(OBJ_DIR)/rip_mib_vrf.o: rip_mib.c \
	makefile $(S)/rules.mk
	$(CC) $(SOPTS_VRF) -o $@ $<
$(OBJ_DIR)/rip_version_vrf.o: rip_version.c \
	makefile $(S)/rules.mk
	$(CC) $(SOPTS) -o $@ $<
	

$(OBJ_DIR)/$(LIB_DIR)VRF.opt: $(APP_DIR)/c.opt
	@echo $(CFLAGS_VRF) > $@

$(OBJ_DIR):
	@mkdir $(OBJ_DIR)

#------------------------------------------------------------------------
# Following target is used in psosmake to ensure that .opt files exist
# when target for a single file is made. 
#------------------------------------------------------------------------
create_opt_files: $(OBJ_DIR) $(OBJ_DIR)/$(LIB_DIR).opt $(OBJ_DIR)/$(LIB_DIR)VRF.opt 

