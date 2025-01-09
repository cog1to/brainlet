# Basic config
INCLUDEDIRS = -I. \
							-I${QTDIR}/include \
							-I${QTDIR}/include/QtCore \
							-I${QTDIR}/include/QtWidgets \
							-I${QTDIR}/include/QtGui
LIBDIRS = -L${QTDIR}/lib
LIBS = -lQt6Core -lQt6Widgets -lQt6Gui -lQt6DBus
FLAGS = 
# Utils
MOC = ${QTDIR}/libexec/moc
# Files
HEADERS := $(shell ls **/*.h)
WIDGETS := $(shell ls widgets/*.cpp)
MOCS_H = $(wildcard widgets/*.h)
MOCS_O = $(patsubst widgets/%.cpp,mocs/%.cpp,$(MOCS_H:.h=.moc.cpp))

# Main target
all: tests

# Clean
clean:
	rm -f mocs/* bin/*

# Directories
bin:
	mkdir -p bin

moc:
	mkdir -p mocs

# Tests with debug graphics
debug: FLAGS += -DDEBUG_GUI=1
debug: tests

# Tests
tests: bin test_anchor test_thought

test_anchor: mocs $(HEADERS) $(WIDGETS) tests/test_anchor.cpp
	$(CXX) -g $(INCLUDEDIRS) $(FLAGS) \
		$(WIDGETS) $(MOCS_O) tests/test_anchor.cpp \
		-o bin/test_anchor $(LIBDIRS) $(LIBS) 

test_thought: mocs $(HEADERS) $(WIDGETS) tests/test_thought.cpp
	$(CXX) -g $(INCLUDEDIRS) $(FLAGS) \
		$(WIDGETS) $(MOCS_O) tests/test_thought.cpp \
		-o bin/test_thought $(LIBDIRS) $(LIBS)

# MOCs
mocs: moc $(MOCS_O)

mocs/%.moc.cpp: widgets/%.h
	$(MOC) $< -o $@
