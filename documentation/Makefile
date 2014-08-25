# Makefile for generating published alhUserGuide files

HTMLDOC = "/home/phoebus/ANJ/bin/solaris-sparc/htmldoc"
WEBSITE = /net/epics/Public/epics/EpicsDocumentation/ExtensionsManuals/AlarmHandler

VERSION = 1.2.34

# Content options:

BOOK    = --book
TITLE   = --title --titlefile ALH.title.html
TOC     = --toclevels 3
#LOGO    = --logoimage alh.gif
TEXT    = --bodyfont Times --fontsize 11.0 --fontspacing 1.2
HDTEXT  = --headingfont Times
#HDTEXT  = --headingfont Helvetica
BODY    = --textcolor black

# .ps & .pdf options:

PAGE    = --duplex --size Universal --portrait --color
MARGINS = --left 1.0in --right 0.5in --top 0.5in --bottom 0.5in
HEADER  = --header .t. --tocheader .t.
FOOTER  = --footer c.1 --tocfooter ..i
HFTEXT  = --headfootfont Times --headfootsize 11.0

OPTS = $(BOOK) $(TITLE) $(TOC) $(LOGO) $(TEXT) $(HDTEXT) $(BODY)
PAGEOPTS = $(OPTS) $(PAGE) $(MARGINS) $(HEADER) $(FOOTER) $(HFTEXT)

PSOPTS   = -t ps $(PAGEOPTS)
PDFOPTS  = -t pdf --firstpage c1 $(PAGEOPTS)
HTMLOPTS = -t html $(OPTS)


FILES  = ALH.html


# ALHUserGuide.html is name of file in alh/Makefile.Host

all: alhUserGuide-$(VERSION).ps.gz alhUserGuide-$(VERSION).pdf ALHUserGuide-$(VERSION).html

alhUserGuide-$(VERSION).ps: $(FILES) Makefile  
	$(HTMLDOC) $(PSOPTS) -f $@ $(FILES)

alhUserGuide-$(VERSION).pdf: $(FILES) Makefile
	$(HTMLDOC) $(PDFOPTS) -f $@ $(FILES)

ALHUserGuide-$(VERSION).html: $(FILES) Makefile
	rm -rf $@
	$(HTMLDOC) $(HTMLOPTS) -f $@ $(FILES)

%.gz: %
	rm -f $@
	gzip $<

install: all
	/bin/cp -f alhUserGuide-$(VERSION).ps.gz $(WEBSITE)/alhUserGuide-$(VERSION).ps.gz
	/bin/cp -f alhUserGuide-$(VERSION).pdf $(WEBSITE)/alhUserGuide-$(VERSION).pdf
	/bin/cp -rf alhUserGuide $(WEBSITE)/alhUserGuide-$(VERSION)
	rm -rf $(WEBSITE)/alhUserGuide-$(VERSION)
	mkdir $(WEBSITE)/alhUserGuide-$(VERSION)
	/bin/cp -rf . $(WEBSITE)/alhUserGuide-$(VERSION)

clean:
	rm alhUserGuide-$(VERSION).ps alhUserGuide-$(VERSION).pdf ALHUserGuide-$(VERSION).html \
	 alhUserGuide-$(VERSION).ps.gz alhUserGuide-$(VERSION).pdf.gz
