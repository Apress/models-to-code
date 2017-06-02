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
#   Makefile for Signal I/O Domain Workbook
#*--

DOC	=\
	lube_sio_bridge.aweb\
	$(NULL)

PDF	= $(patsubst %.aweb,%.pdf,$(DOC))

IMAGES	=\
	$(NULL)

IMAGEPDF =\
	$(patsubst %.uxf,%.pdf,$(IMAGES))\
	$(NULL)

DBLATEX_PARAMS	=\
		doc.publisher.show=0\
		glossary.numbered=0\
		index.numbered=0\
		doc.lot.show=figure,table\
		toc.section.depth=3\
		doc.section.depth=0\
		$(NULL)

# Add this to the above options to get rid of the revision history section
# latex.output.revhistory=0

A2X_OPTS	=\
		--verbose\
		--doctype=article\
		--format=pdf\
		$(NULL)

ASCIIDOC_ATTRS	=\
		docinfo1\
		$(NULL)

DBLATEX_OPTS	=\
		--dblatex-opts="$(patsubst %,--param=%,$(DBLATEX_PARAMS))"\
		$(NULL)

ASCIIDOC_OPTS	=\
		$(patsubst %,--attribute=%,$(ASCIIDOC_ATTRS))\
		$(NULL)

CLEANFILES	=\
		$(PDF)\
		$(IMAGEPDF)\
		$(NULL)

all : $(PDF)

$(PDF) : $(DOC) $(IMAGEPDF) docinfo.xml

%.pdf : %.aweb
	a2x $(A2X_OPTS) $(ASCIIDOC_OPTS) $(DBLATEX_OPTS) $<

%.pdf : %.uxf
	umlet -action=convert -format=pdf -filename=$< -output=$(basename $@)

sync :
	umletsync -report atctrl.aweb

clean :
	$(RM) $(CLEANFILES)
