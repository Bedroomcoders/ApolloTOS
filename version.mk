#
# version.mk - Makefile fragment for version numbers
#
# Copyright (C) 2017-2025 The EmuTOS development team.
#
# Authors:
#  VRI      Vincent Rivière
#  RFB      Roger Burrows
#
# This file is distributed under the GPL, version 2 or at your
# option any later version.  See doc/license.txt for details.
#

#
# ApolloTOS and EmuCON2 version management
#

#
# the following values are stored in the ApolloTOS 'extended os header',
# and are used to create the VERSION string for official releases.
#
MAJOR_VERSION = 1
MINOR_VERSION = 0
FIX_VERSION = 0

empty =
space = $(empty) $(empty)
ifeq ($(FIX_VERSION),0)
tempversion = $(MAJOR_VERSION) $(MINOR_VERSION)
else
tempversion = $(MAJOR_VERSION) $(MINOR_VERSION) $(FIX_VERSION)
endif

#
# this is the version string displayed on the welcome screen, and in
# EmuDesk's 'About' dialog.
#
VERSION = 1.0.0-ApolloTOS

#
# this is stored in the OS 'extended os header'.  a zero value
# indicates an official release; a non-zero value indicates any other
# kind of release (snapshot or custom).
#
UNOFFICIAL = 0
