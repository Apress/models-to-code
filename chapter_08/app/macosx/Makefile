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

PLATFORM = macosx

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
	../common\
	../../../chapter_06/model\
	../../../chapter_07/model\
	../../model\
	$(NULL)

PROG	=\
	lube_sio\
	$(NULL)

AWEB	=\
	lube.aweb\
	sio.aweb\
	$(NULL)

TACKFILE =\
	alsharness.tack\
	$(NULL)

PYCCAFILE =\
	$(patsubst %.aweb,%.pycca,$(AWEB))\
	$(NULL)

PYFLAGS	=\
	-trim\
	-noline\
	-save\
	-dataportal\
	-instrument '*'\
	$(NULL)

SRCS	=\
	$(patsubst %.tack,%.c,$(TACKFILE))\
	$(patsubst %.pycca,%.c,$(PYCCAFILE))\
	platform.c\
	lube_sio_bridge.c\
	sio_eop.c\
	$(NULL)


OBJS	=\
	$(patsubst %.c,%.o,$(SRCS))\
	$(NULL)

DEPFILES = $(patsubst %.o,%.d,$(OBJS))

LIBS	=\
	tack\
	$(NULL)

LIBFILES =\
	 $(TACKDIR)/libtack.a\
	 $(NULL)

TFLAGS	=\
	-save\
	$(NULL)

CPPFLAGS=\
	-DTACK\
	-I.\
	-I$(TACKDIR)\
	-I../common\
	$(NULL)

CFLAGS	=\
	-std=c99\
	-g3\
	-Wall\
	$(NULL)

.PHONY : all

all : $(PROG) $(LIBFILES)

$(LIBFILES) :
	$(MAKE) -C $(TACKDIR)

$(PROG) : $(OBJS) $(TACKDIR)/libtack.a
	cc -o $@ $(OBJS) $(TACKDIR)/libtack.a

lube.o : lube.c lube.h

lube.c lube.h lube.ral : lube.pycca

lube.pycca : lube.aweb

alsharness.c alsharness.h : alsharness.tack lube.ral sio.ral

sio.o : sio.c sio.h

sio.c sio.h sio.ral : sio.pycca

sio.pycca : sio.aweb

sio_eop.o : sio_eop.c sio.h

sio_eop.c : sio.aweb
	$(ATANGLE) -root $(notdir $@) -output $(notdir $@) $<

lube_sio_bridge.o : lube_sio_bridge.c lube.h sio.h

lube_sio_bridge.c : lube_sio_bridge.aweb

platform.c : harness.h alsharness.h lube.h sio.h

alsharness.c alsharness.h : alsharness.tack lube.ral sio.ral
	$(TACK) $(TFLAGS) $<

CLEANFILES	=\
		$(PROG)\
		$(OBJS)\
		lube_sio_bridge.c\
		sio_eop.c\
		$(patsubst %.aweb,%.pycca,$(AWEB))\
		$(patsubst %.pycca,%.c,$(PYCCAFILE))\
		$(patsubst %.pycca,%.h,$(PYCCAFILE))\
		$(patsubst %.c,%.d,$(SRCS))\
		$(patsubst %.tack,%.c,$(TACKFILE))\
		$(patsubst %.tack,%.h,$(TACKFILE))\
		*.d.*\
		*.ral\
		$(DEPFILES)\
		*.gcov\
		*.gcno\
		*.gcda\
		$(NULL)

.PHONY : clean
clean :
	-$(RM) $(LIB) $(CLEANFILES)

%.c %.h %.ral : %.pycca
	$(PYCCA) $(PYFLAGS) $<

%.pycca : %.aweb
	$(ATANGLE) -root $(notdir $@) -output $(notdir $@) $<

%.c : %.aweb
	$(ATANGLE) -root $(notdir $@) -output $(notdir $@) $<

# vim:set ts=8 sw=8 sts=8 noexpandtab:
