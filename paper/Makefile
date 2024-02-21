TARGET    := main.pdf

IMAGE_DIR := images
TABLE_DIR := tables
CODE_DIR  := code
BUILD_DIR := build

IMAGES := $(shell find $(IMAGE_DIR) -name "*.tex")
TABLES := $(shell find $(TABLE_DIR) -name "*.tex")
CODES  := $(shell find $(CODE_DIR) -name "*.tex")
SOURCES   := deps/pkgs.tex deps/macros.tex main.tex $(CODES) $(TABLES) $(IMAGES)
RASTERIMG := $(wildcard images/*.png)
VECTORIMG := $(wildcard images/*.svg)


LATEX   = pdflatex -shell-escape -file-line-error -interaction=nonstopmode
BIBTEX  = bibtex

.PHONY: all debug clean distclean .dirs

all: $(TARGET)

$(TARGET): .dirs $(SOURCES) $(RASTERIMG) $(patsubst %.svg,%.pdf,$(VECTORIMG))

.dirs:
	mkdir -p $(BUILD_DIR) $(IMAGE_DIR) $(TABLE_DIR) $(CODE_DIR)

%.pdf: %.tex %.bib
	($(LATEX) $<; \
	$(BIBTEX) $* > $(BIBTEX)_out.log; \
	$(LATEX) $<; \
	$(LATEX) $<;)||true
	while ( grep -q '^LaTeX Warning: Label(s) may have changed' $*.log) \
	do $(LATEX) $<; done
	mv -f *.aux *.idx *.ind *.out *.toc *.log *.bbl *.blg *.brf $(BUILD_DIR) 2> /dev/null ||true

debug:
	-grep Warning *.log

clean:
	$(RM) *.aux *.idx *.ind *.out *.toc *.log *.bbl *.blg *.brf
	$(RM) $(BUILD_DIR) -r

distclean: clean
	$(RM) *.pdf
