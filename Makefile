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
WIDGETS_C := $(shell ls widgets/*widget.cpp) widgets/style.cpp
WIDGETS_H = $(wildcard widgets/*widget.h) widgets/style.h
WIDGETS_MOCS_C = $(patsubst widgets/%.cpp,mocs/%.cpp,$(WIDGETS_H:.h=.moc.cpp))
# Presenters
PRESENTERS_C := $(shell ls presenters/*.cpp)
PRESENTERS_H = $(wildcard presenters/*.h)
PRESENTERS_MOCS_C = $(patsubst presenters/%.cpp,mocs/%.cpp,$(PRESENTERS_H:.h=.moc.cpp))

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
	test_memory test_presenter test_markdown test_text_presenter \
	test_brain_presenter

test_anchor: mocs $(HEADERS) $(WIDGETS_C) tests/test_anchor.cpp
	$(CXX) -g $(INCLUDEDIRS) $(CFLAGS) \
		widgets/style.cpp mocs/style.moc.cpp \
		widgets/base_widget.cpp mocs/base_widget.moc.cpp \
		widgets/anchor_widget.cpp mocs/anchor_widget.moc.cpp tests/test_anchor.cpp \
		-o bin/test_anchor $(LIBDIRS) $(LIBS)

test_thought: mocs $(HEADERS) $(WIDGETS_C) tests/test_thought.cpp
	$(CXX) -g $(INCLUDEDIRS) $(CFLAGS) \
		widgets/style.cpp mocs/style.moc.cpp \
		widgets/base_widget.cpp mocs/base_widget.moc.cpp \
		widgets/anchor_widget.cpp mocs/anchor_widget.moc.cpp \
		widgets/thought_widget.cpp mocs/thought_widget.moc.cpp \
		widgets/thought_edit_widget.cpp mocs/thought_edit_widget.moc.cpp \
		tests/test_thought.cpp \
		-o bin/test_thought $(LIBDIRS) $(LIBS)

test_resize: mocs $(HEADERS) $(WIDGETS_C) tests/test_resize_on_hover.cpp
	$(CXX) -g $(INCLUDEDIRS) $(CFLAGS) \
		widgets/style.cpp mocs/style.moc.cpp \
		widgets/base_widget.cpp mocs/base_widget.moc.cpp \
		widgets/anchor_widget.cpp mocs/anchor_widget.moc.cpp \
		widgets/thought_widget.cpp mocs/thought_widget.moc.cpp \
		widgets/thought_edit_widget.cpp mocs/thought_edit_widget.moc.cpp \
		tests/test_resize_on_hover.cpp \
		-o bin/test_resize $(LIBDIRS) $(LIBS)

test_edit: mocs $(HEADERS) $(WIDGETS_C) tests/test_resize_on_hover.cpp
	$(CXX) -g $(INCLUDEDIRS) $(CFLAGS) \
		$(WIDGETS_C) \
		$(WIDGETS_MOCS_C) \
		$(MODELS) \
		tests/test_edit.cpp \
		-o bin/test_edit $(LIBDIRS) $(LIBS)

test_base: mocs $(HEADERS) $(WIDGETS_C) tests/test_base_canvas.cpp
	$(CXX) -g $(INCLUDEDIRS) $(CFLAGS) \
		$(MODELS) $(LAYOUTS) $(WIDGETS_C) $(WIDGETS_MOCS_C) \
		tests/test_base_canvas.cpp \
		-o bin/test_base $(LIBDIRS) $(LIBS)

test_memory: $(HEADERS) $(REPO) tests/test_memory_repository.cpp
	$(CXX) -g $(INCLUDEDIRS) $(CFLAGS) \
		$(MODELS) $(LAYOUTS) $(WIDGETS_C) $(WIDGETS_MOCS_C) \
		$(REPO) tests/test_memory_repository.cpp \
		-o bin/test_memory $(LIBDIRS) $(LIBS)

test_presenter: mocs $(HEADERS) $(REPO) tests/test_presenter.cpp
	$(CXX) -g $(INCLUDEDIRS) $(CFLAGS) \
		$(MODELS) $(REPO) $(WIDGETS_C) $(PRESENTERS_C) $(LAYOUTS) \
		$(WIDGETS_MOCS_C) $(PRESENTERS_MOCS_C) \
		tests/test_presenter.cpp \
		-o bin/test_presenter $(LIBDIRS) $(LIBS)

test_markdown: mocs $(HEADERS) $(WIDGETS_C) tests/test_markdown.cpp
	$(CXX) -g $(INCLUDEDIRS) $(CFLAGS) \
		$(MODELS) $(LAYOUTS) $(WIDGETS_C) $(WIDGETS_MOCS_C) tests/test_markdown.cpp \
		-o bin/test_markdown $(LIBDIRS) $(LIBS)

test_text_presenter: mocs $(HEADERS) $(REPO) tests/test_text_presenter.cpp
	$(CXX) -g $(INCLUDEDIRS) $(CFLAGS) \
		$(MODELS) $(REPO) $(WIDGETS_C) $(PRESENTERS_C) $(LAYOUTS) \
		$(WIDGETS_MOCS_C) $(PRESENTERS_MOCS_C) \
		tests/test_text_presenter.cpp \
		-o bin/test_text_presenter $(LIBDIRS) $(LIBS)

test_brain_presenter: mocs $(HEADERS) $(REPO) tests/test_brain_presenter.cpp
	$(CXX) -g $(INCLUDEDIRS) $(CFLAGS) \
		$(MODELS) $(REPO) $(WIDGETS_C) $(PRESENTERS_C) $(LAYOUTS) \
		$(WIDGETS_MOCS_C) $(PRESENTERS_MOCS_C) \
		tests/test_brain_presenter.cpp \
		-o bin/test_brain_presenter $(LIBDIRS) $(LIBS)

# MOCs
mocs: moc $(WIDGETS_MOCS_C) $(PRESENTERS_MOCS_C)

mocs/%widget.moc.cpp: widgets/%widget.h
	$(MOC) $< -o $@

mocs/%presenter.moc.cpp: presenters/%presenter.h
	$(MOC) $< -o $@

