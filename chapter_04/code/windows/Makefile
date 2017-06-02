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
#    Makefile for ATC domain translation
#*--

TOOLDIR = ../../../tools/windows

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
	../../model\
	../common\
	$(NULL)

PROG	=\
	atc\
	$(NULL)

TACKFILE =\
	atcharness.tack\
	$(NULL)

AWEB	=\
	atctrl.aweb\
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
	platform.c\
	$(patsubst %.tack,%.c,$(TACKFILE))\
	$(patsubst %.pycca,%.c,$(PYCCAFILE))\
	$(NULL)


OBJS	=\
	$(patsubst %.c,%.o,$(SRCS))\
	$(NULL)

DEPFILES = $(patsubst %.o,%.d,$(OBJS))

LIBS	=\
	tack\
	$(NULL)

LIBFILES = $(patsubst %,lib%.a,$(LIBS))

TFLAGS	=\
	-save\
	$(NULL)

CPPFLAGS=\
	-DTACK\
	-I.\
	$(patsubst %,-I%,$(VPATH))\
	$(NULL)

CFLAGS	=\
	-std=c99\
	-g3\
	-Wall\
	$(NULL)

all : $(LIBFILES) $(PROG)

$(PROG) : $(OBJS) $(TACKDIR)/libtack.a
	cc $^ -lgcov -o $@

platform.o : platform.c harness.h mechs.h atcharness.h

atctrl.pycca : atctrl.aweb
	$(ATANGLE) -root $@ -output $@ $<

atctrl.c atctrl.h atctrl.ral : atctrl.pycca
	$(PYCCA) $(PYFLAGS) $<

atcharness.c atcharness.h : atcharness.tack atctrl.ral
	$(TACK) $(TFLAGS) $<

.PHONY : libs

libs : $(LIBFILES)

$(TACKDIR)/libtack.a :
	$(MAKE) -C $(TACKDIR)

CLEANFILES	=\
		$(PROG)\
		$(OBJS)\
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

# vim:set ts=8 sw=8 sts=8 noexpandtab:
