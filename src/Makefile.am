# $Id$

# This file is part of bidilink.
#
# bidilink is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# bidilink is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with bidilink; if not, write to the Free Software Foundation,
# Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

bin_PROGRAMS = bidilink

bidilink_SOURCES = bidilink.c client-tty.h exec.h server-tty.h std.h \
	client-tcp.c client-unix.c server-tcp.c server-unix.c stream.c \
	client-tcp.h client-unix.h server-tcp.h server-unix.h stream.h \
	client-tty.c exec.c server-tty.c std.c

bidilink_CFLAGS = -DLOCKDIR="\"$(LOCKDIR)\""

