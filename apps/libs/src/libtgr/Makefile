# Makefile
# vim:syntax=make noexpandtab:
#
# Authors:
#		EQware various
#
# May use variables
# 	TGR_TYPE:	Whether to build xe or ce portion of the runtime.
# 			May be one of: tg-xe, tg-ce
# 			(Required, No Default)
#
# 	TG_ROOT:	Where the root (tg/tg) director of the tg tree is.
# 			(Default: `pwd`/../../../../../tg/tg)
#
# 	TG_INSTALL:	Where the install director of the tg tree is.
# 			(Default: $TG_ROOT/install)
#

#
# Supported TGR_TYPE values
#
TGR_TYPES = tg-xe tg-ce

ifndef TGR_TYPE
$(error Need to specify TGR_TYPE! )
endif

ifeq (,$(findstring $(TGR_TYPE), $(TGR_TYPES)))
$(error Unknown/unsupported  TGR_TYPE ($(TGR_TYPE)) ! )
endif

default: all
all:
	$(MAKE) -C $(TGR_TYPE) all

install:
	$(MAKE) -C $(TGR_TYPE) install

clean:
	$(MAKE) -C $(TGR_TYPE) clean

.PHONY: default all install clean
