################################################################
#
# $Id:$
#
# $Log:$
#

ifeq ($(V),1)
	VB=''
else
	VB=@
endif

ifdef OVERDRIVE
CFLAGS	:=	-Wall -g -O3 \
	-std=gnu11 \
	-fno-unroll-loops \
	-fstrict-aliasing \
	-DVB_OVERDRIVE \
	-DVB_DOUBLE_BUFFER \
  -DDOOMGENERIC_RESX=384 -DDOOMGENERIC_RESY=224

else # !OVERDRIVE

ifdef USE_CLANG
CLANG_ROOT := $(HOME)/gits/llvm-v810/build/bin/
CXX     := $(CLANG_ROOT)/clang++
CC     := $(CLANG_ROOT)/clang
AS     := $(CLANG_ROOT)/llvm-mc
AR     := $(CLANG_ROOT)/llvm-ar
LD     := $(CLANG_ROOT)/ld.lld
OBJCOPY     := $(CLANG_ROOT)/llvm-objcopy
else
CXX     := ~/bin/v810-gcc/bin/v810-g++
CC     := ~/bin/v810-gcc/bin/v810-gcc
AS     := ~/bin/v810-gcc/bin/v810-as
AR     := ~/bin/v810-gcc/bin/v810-ar
LD     := ~/bin/v810-gcc/bin/v810-ld
OBJCOPY     := ~/bin/v810-gcc/bin/v810-objcopy
endif

CFLAGS	:=	-Wall -Wno-sign-compare -g -O3 -ffast-math -fno-builtin \
	-std=gnu11 \
	-fno-unroll-loops \
	-fomit-frame-pointer \
	-fstrict-aliasing \
	-isystem $(HOME)/gits/v810-gcc-newlib-for-llvm/v810-gcc/v810/include \
	-Werror=return-type \
	-D__VALIST=char* \
	-DFIXED_REALS \
	-DNO_IOSTREAM \
	-DHWINFO_DEBUG \
	-DVB_DOUBLE_BUFFER \
  -DDOOMGENERIC_RESX=384 -DDOOMGENERIC_RESY=224


LDFLAGS:=	-Tvb.ld -g -nostartfiles -specs=nosys.specs -lnosys -nostartfiles -fno-builtin

ifdef USE_CLANG
CFLAGS += -mcpu=vb -mapp-regs #-flto
ASFLAGS	:=	--triple v810-unknown-vb --filetype=obj
LIBDIRS := $(HOME)/gits/v810-gcc-newlib-for-llvm/v810-gcc/v810
else
LIBS += -lgcc
LIBDIRS	:=	$(HOME)/bin/v810-gcc/v810
CFLAGS += -fno-leading-underscore
endif

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)

ifndef USE_CLANG
LIBPATHS += -L$(HOME)/bin/v810-gcc/lib/gcc/v810/4.9.4
endif

endif # !OVERDRIVE

# subdirectory for objects
OBJDIR=build
OUTPUT=doomgeneric

SRC_DOOM = dummy.o am_map.o doomdef.o doomstat.o dstrings.o d_event.o d_items.o d_iwad.o d_loop.o d_main.o d_mode.o d_net.o f_finale.o f_wipe.o g_game.o hu_lib.o hu_stuff.o info.o i_cdmus.o i_endoom.o i_joystick.o i_scale.o i_sound.o i_system.o i_timer.o memio.o m_argv.o m_bbox.o m_cheat.o m_config.o m_controls.o m_fixed.o m_menu.o m_misc.o m_random.o p_ceilng.o p_doors.o p_enemy.o p_floor.o p_inter.o p_lights.o p_map.o p_maputl.o p_mobj.o p_plats.o p_pspr.o p_saveg.o p_setup.o p_sight.o p_spec.o p_switch.o p_telept.o p_tick.o p_user.o r_bsp.o r_data.o r_draw.o r_main.o r_plane.o r_segs.o r_sky.o r_things.o sha1.o sounds.o statdump.o st_lib.o st_stuff.o s_sound.o tables.o v_video.o wi_stuff.o w_checksum.o w_file.o w_main.o w_wad.o z_zone.o w_file_stdc.o i_input.o i_video.o doomgeneric.o doomgeneric_vb.o vb_utils.o rom_header.o doom1_wad.o

ifndef OVERDRIVE
	SRC_DOOM += crt0.o
endif


OBJS += $(addprefix $(OBJDIR)/, $(SRC_DOOM))

all:	 $(OUTPUT).vb

clean:
	rm -rf $(OBJDIR)
	rm -f $(OUTPUT).vb
	rm -f $(OUTPUT).elf
	rm -f doom1_wad.h
	rm -f doom1_wad.c

$(OUTPUT).vb	:	$(OUTPUT).elf
	$(OBJCOPY) -O binary $(OUTPUT).elf $(OUTPUT).vb

$(OUTPUT).elf	:	$(OBJS)
	$(CC) $(CFLAGS) $(OBJS) \
 	$(LIBPATHS) \
	$(LDFLAGS) \
	-o $(OUTPUT).elf  -fdata-sections -ffunction-sections -Wl,--gc-sections \

doom1_wad.c: DOOM1.WAD
	echo -n "const " >> doom1_wad.c
	xxd -i DOOM1.WAD >> doom1_wad.c

doom1_wad.h: doom1_wad.c
	echo "#pragma once" >> doom1_wad.h
	echo -n "extern " >> doom1_wad.h
	head -n 1 doom1_wad.c | sed 's/\].*$$//g' | tr '\n' ' ' >> doom1_wad.h
	tail -n 1 doom1_wad.c | sed 's/^unsigned int DOOM1_WAD_len = \([0-9]*\);/\1/g' | tr '\n' ' ' >> doom1_wad.h
	echo "];" >> doom1_wad.h


$(OBJS): | $(OBJDIR) doom1_wad.h

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.o:	%.c
	@echo [Compiling $<]
	$(VB)$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o:	%.s
	@echo [Compiling $<]
	$(VB)$(CC) $(CFLAGS) -c $< -o $@

print:
	@echo OBJS: $(OBJS)

