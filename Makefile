BAK = backup`date +%s`.tar.gz

CPP := g++

EXE := histviewer

.PHONY: all clean backup

all: $(EXE)

histviewer: main.o HistViewer.o HistViewerDict.o HistFileReader.o
	$(CPP) -g -Wall -O $(filter %.o,$^) -o $@ $(shell root-config --glibs)

%Dict.cxx: %.h %LinkDef.h
	rootcint -f $@ -c $(filter %.h,$^)

%.o: %.cxx
	$(CPP) -g -Wall -O -c $(shell root-config --cflags) $< -o $@

HistViewer.o HistFileReader.o : %.o: %.h
%Dict.o: %.h %LinkDef.h
main.o: HistViewer.h
HistViewer.o: HistFileReader.h

clean:
	rm -f $(EXE) *.o *Dict.cxx *Dict.h

backup:
	@echo Creating $(BAK)
	@mkdir -p BAK
	@tar cvzfh BAK/$(BAK) $(wildcard *.cxx) $(wildcard *.h) Makefile
