#*++
# Copyright 2017 by Leon Starr, Andrew Mangogna and Stephen Mellor
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# Project:
#   Models to Code Book
#
# Module:
#   Makefile for Lubrication domain translation
#*--

PLATFORM = windows

TOOLDIR = ../../../tools/$(PLATFORM)

ATANGLE =\
	$(TOOLDIR)/atangle\
	$(NULL)

PYCCA	=\
	$(TOOLDIR)/pycca\
	$(NULL)

TACK	=\
	$(TOOLDIR)/tack\
	$(NULL)

TACKDIR	= ../../../libtack

VPATH	=\
	$(TACKDIR)\
	../../../chapter_06/code/$(PLATFORM)\
	../../../chapter_07/code/$(PLATFORM)\
	../../model\
	$(NULL)

SRCS	=\
	lube_sio_bridge.c\
	$(NULL)

AWEB	=\
	lube_sio_bridge.aweb\
	$(NULL)

OBJS	=\
	$(patsubst %.c,%.o,$(SRCS))\
	$(NULL)

DEPFILES = $(patsubst %.o,%.d,$(OBJS))

CPPFLAGS=\
	-I.\
	$(patsubst %,-I%,$(VPATH))\
	$(NULL)

CFLAGS	=\
	-std=c99\
	-g3\
	-Wall\
	-Og\
	$(NULL)

.PHONY : all clean

all : $(OBJS)

lube_sio_bridge.o : sio.h lube.h pycca_portal.h harness.h

lube_sio_bridge.c : $(AWEB)
	$(ATANGLE) -root $@ -output $@ $<

CLEANFILES	=\
		$(SRCS)\
		$(OBJS)\
		$(DEPFILES)\
		$(NULL)

clean :
	-$(RM) $(LIB) $(CLEANFILES)

# vim:set ts=8 sw=8 sts=8 noexpandtab:
