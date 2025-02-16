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
# MacOS overrides
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	CXX = /opt/homebrew/opt/llvm/bin/clang++
	LIBDIRS = -F${QTDIR}/lib
	LIBS = -framework QtWidgets -framework QtCore -framework QtGui -framework QtDBus
	MOC = ${QTDIR}/share/qt/libexec/moc
	CFLAGS += -DDARWIN=1
endif
# Files
HEADERS := $(shell ls **/*.h)
LAYOUTS := $(shell ls layout/*.cpp)
MODELS := $(shell ls model/*.cpp)
REPO := $(shell ls entity/*.cpp)
# Widgets
WIDGETS := $(shell ls widgets/*.cpp)
MOCS_H = $(wildcard widgets/*.h)
MOCS_O = $(patsubst widgets/%.cpp,mocs/%.cpp,$(MOCS_H:.h=.moc.cpp))
# Presenters
PRESENTERS := $(shell ls presenters/*.cpp)
PRESENTERS_MOCS_H = $(wildcard presenters/*.h)
PRESENTERS_MOCS_O = $(patsubst presenters/%.cpp,mocs/%.cpp,$(PRESENTERS_MOCS_H:.h=.moc.cpp))

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
tests: bin test_anchor test_thought test_resize test_edit test_base \
	test_memory test_presenter test_markdown

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

test_memory: $(HEADERS) $(REPO) tests/test_memory_repository.cpp
	$(CXX) -g $(INCLUDEDIRS) $(CFLAGS) \
		$(MODELS) $(REPO) tests/test_memory_repository.cpp \
		-o bin/test_memory

test_presenter: mocs $(HEADERS) $(REPO) tests/test_presenter.cpp
	$(CXX) -g $(INCLUDEDIRS) $(CFLAGS) \
		$(MODELS) $(REPO) $(WIDGETS) $(PRESENTERS) $(LAYOUTS) \
		$(MOCS_O) $(PRESENTERS_MOCS_O) \
		tests/test_presenter.cpp \
		-o bin/test_presenter $(LIBDIRS) $(LIBS)

test_markdown: mocs $(HEADERS) $(WIDGETS) tests/test_markdown.cpp
	$(CXX) -g $(INCLUDEDIRS) $(CFLAGS) \
		$(MODELS) $(LAYOUTS) $(WIDGETS) $(MOCS_O) tests/test_markdown.cpp \
		-o bin/test_markdown $(LIBDIRS) $(LIBS)

# MOCs
mocs: moc $(MOCS_O) $(PRESENTERS_MOCS_O)

mocs/%widget.moc.cpp: widgets/%widget.h
	$(MOC) $< -o $@

mocs/%presenter.moc.cpp: presenters/%presenter.h
	$(MOC) $< -o $@

