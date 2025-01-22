# Basic config
INCLUDEDIRS = -I${QTDIR}/include \
							-I${QTDIR}/include/QtCore \
							-I${QTDIR}/include/QtWidgets \
							-I${QTDIR}/include/QtGui \
							-I.
LIBDIRS = -L${QTDIR}/lib
LIBS = -lQt6Core -lQt6Widgets -lQt6Gui -lQt6DBus
CFLAGS = ${FLAGS}
# Utils
MOC = ${QTDIR}/libexec/moc
# Files
HEADERS := $(shell ls **/*.h)
LAYOUTS := $(shell ls layout/*.cpp)
MODELS := $(shell ls model/*.cpp)
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
debug: CFLAGS += -DDEBUG_GUI=1
debug: tests

# Tests
tests: opts bin test_anchor test_thought test_resize test_edit test_base

test_anchor: mocs $(HEADERS) $(WIDGETS) tests/test_anchor.cpp
	$(CXX) -g $(INCLUDEDIRS) $(CFLAGS) \
		$(WIDGETS) $(MOCS_O) tests/test_anchor.cpp \
		-o bin/test_anchor $(LIBDIRS) $(LIBS)

test_thought: mocs $(HEADERS) $(WIDGETS) tests/test_thought.cpp
	$(CXX) -g $(INCLUDEDIRS) $(CFLAGS) \
		$(WIDGETS) $(MOCS_O) tests/test_thought.cpp \
		-o bin/test_thought $(LIBDIRS) $(LIBS)

test_resize: mocs $(HEADERS) $(WIDGETS) tests/test_resize_on_hover.cpp
	$(CXX) -g $(INCLUDEDIRS) $(CFLAGS) \
		$(WIDGETS) $(MOCS_O) tests/test_resize_on_hover.cpp \
		-o bin/test_resize $(LIBDIRS) $(LIBS)

test_edit: mocs $(HEADERS) $(WIDGETS) tests/test_resize_on_hover.cpp
	$(CXX) -g $(INCLUDEDIRS) $(CFLAGS) \
		$(WIDGETS) $(MOCS_O) tests/test_edit.cpp \
		-o bin/test_edit $(LIBDIRS) $(LIBS)

test_base: mocs $(HEADERS) $(WIDGETS) tests/test_base_canvas.cpp
	$(CXX) -g $(INCLUDEDIRS) $(CFLAGS) \
		$(MODELS) $(LAYOUTS) $(WIDGETS) $(MOCS_O) tests/test_base_canvas.cpp \
		-o bin/test_base $(LIBDIRS) $(LIBS)

# MOCs
mocs: moc $(MOCS_O)

mocs/%.moc.cpp: widgets/%.h
	$(MOC) $< -o $@
