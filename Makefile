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
# Layouts
LAYOUTS := $(shell ls layout/*.cpp)
LAYOUTS_O = $(patsubst %.cpp,obj/%.o,$(LAYOUTS))
# Models
MODELS := $(shell ls model/*.cpp)
MODELS_O = $(patsubst %.cpp,obj/%.o,$(MODELS))
# Entity
REPO := $(shell ls entity/*.cpp)
REPO_O = $(patsubst %.cpp,obj/%.o,$(REPO))
# Widgets
WIDGETS_C := $(shell ls widgets/*widget.cpp) widgets/style.cpp
WIDGETS_H = $(wildcard widgets/*widget.h) widgets/style.h
WIDGETS_O = $(patsubst %.cpp,obj/%.o$(WIDGETS_C))
WIDGETS_MOCS_C = $(patsubst widgets/%.cpp,mocs/%.cpp,$(WIDGETS_H:.h=.moc.cpp))
WIDGETS_MOCS_O = $(patsubst %.cpp,obj/%.o,$(WIDGETS_MOCS_C))
# Presenters
PRESENTERS_C := $(shell ls presenters/*.cpp)
PRESENTERS_H = $(wildcard presenters/*.h)
PRESENTERS_O = $(patsubst %.cpp,obj/%.o,$(PRESENTERS_C))
PRESENTERS_MOCS_C = $(patsubst presenters/%.cpp,mocs/%.cpp,$(PRESENTERS_H:.h=.moc.cpp))
PRESENTERS_MOCS_O = $(patsubst %.cpp,obj/%.o,$(PRESENTERS_MOCS_C))
# All object files
SOURCES = $(shell find . \( -path ./tests -prune -o -path ./mocs -prune \) -o -name "*.cpp" -print | sed -e 's/\.\///') $(WIDGETS_MOCS_C) $(PRESENTERS_MOCS_C)
OBJECTS = $(patsubst %.cpp,obj/%.o,$(SOURCES))

# Main target
all: tests

# Clean
clean:
	rm -rf mocs/* bin/* obj/*

# Directories
bin:
	mkdir -p bin

moc: mocs
	mkdir -p mocs

# Tests with debug graphics
debug: CFLAGS += -DDEBUG_GUI=1
debug: tests

# MOCs
mocfiles: moc $(WIDGETS_MOCS_C) $(PRESENTERS_MOCS_C)

mocs/%widget.moc.cpp: widgets/%widget.h
	$(MOC) $< -o $@

mocs/%presenter.moc.cpp: presenters/%presenter.h
	$(MOC) $< -o $@

mocs/style.moc.cpp: widgets/style.h
	$(MOC) $< -o $@

# Object rules
obj/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) -c $< -g $(INCLUDEDIRS) $(CFLAGS) \
		-o $@

# Tests
tests: bin test_anchor test_thought test_resize test_edit test_base \
	test_memory test_presenter test_markdown test_text_presenter \
	test_brain_presenter test_conn test_brain_item \
	test_brain_list test_brain_list_presenter

test_anchor: $(OBJECTS) tests/test_anchor.cpp
	$(CXX) -g $(INCLUDEDIRS) $(CFLAGS) \
		$(OBJECTS) \
		tests/test_anchor.cpp \
		-o bin/test_anchor \
		$(LIBDIRS) $(LIBS)

test_thought: $(OBJECTS) tests/test_thought.cpp
	$(CXX) -g $(INCLUDEDIRS) $(CFLAGS) \
		$(OBJECTS) \
		tests/test_thought.cpp \
		-o bin/test_thought \
		$(LIBDIRS) $(LIBS)

test_resize: $(OBJECTS) tests/test_resize_on_hover.cpp
	$(CXX) -g $(INCLUDEDIRS) $(CFLAGS) \
		$(OBJECTS) \
		tests/test_resize_on_hover.cpp \
		-o bin/test_resize \
		$(LIBDIRS) $(LIBS)

test_edit: $(OBJECTS) tests/test_resize_on_hover.cpp
	$(CXX) -g $(INCLUDEDIRS) $(CFLAGS) \
		$(OBJECTS) \
		tests/test_edit.cpp \
		-o bin/test_edit \
		$(LIBDIRS) $(LIBS)

test_base: $(OBJECTS) tests/test_base_canvas.cpp
	$(CXX) -g $(INCLUDEDIRS) $(CFLAGS) \
		$(OBJECTS) \
		tests/test_base_canvas.cpp \
		-o bin/test_base \
		$(LIBDIRS) $(LIBS)

test_memory: $(OBJECTS) tests/test_memory_repository.cpp
	$(CXX) -g $(INCLUDEDIRS) $(CFLAGS) \
		$(OBJECTS) \
		tests/test_memory_repository.cpp \
		-o bin/test_memory \
		$(LIBDIRS) $(LIBS)

test_presenter: $(OBJECTS) tests/test_presenter.cpp
	$(CXX) -g $(INCLUDEDIRS) $(CFLAGS) \
		$(OBJECTS) \
		tests/test_presenter.cpp \
		-o bin/test_presenter \
		$(LIBDIRS) $(LIBS)

test_markdown: $(OBJECTS) tests/test_markdown.cpp
	$(CXX) -g $(INCLUDEDIRS) $(CFLAGS) \
		$(OBJECTS) \
		tests/test_markdown.cpp \
		-o bin/test_markdown \
		$(LIBDIRS) $(LIBS)

test_text_presenter: $(OBJECTS) tests/test_text_presenter.cpp
	$(CXX) -g $(INCLUDEDIRS) $(CFLAGS) \
		$(OBJECTS) \
		tests/test_text_presenter.cpp \
		-o bin/test_text_presenter \
		$(LIBDIRS) $(LIBS)

test_brain_presenter: $(OBJECTS) tests/test_brain_presenter.cpp
	$(CXX) -g $(INCLUDEDIRS) $(CFLAGS) \
		$(OBJECTS) \
		tests/test_brain_presenter.cpp \
		-o bin/test_brain_presenter \
		$(LIBDIRS) $(LIBS)

test_conn: $(OBJECTS) tests/test_conn_list.cpp
	$(CXX) -g $(INCLUDEDIRS) $(CFLAGS) \
		$(OBJECTS) \
		tests/test_conn_list.cpp \
		-o bin/test_conn_list \
		$(LIBDIRS) $(LIBS)

test_brain_item: $(OBJECTS) tests/test_brain_item.cpp
	$(CXX) -g $(INCLUDEDIRS) $(CFLAGS) \
		$(OBJECTS) \
		tests/test_brain_item.cpp \
		-o bin/test_brain_item \
		$(LIBDIRS) $(LIBS)

test_brain_list: $(OBJECTS) tests/test_brain_list.cpp
	$(CXX) -g $(INCLUDEDIRS) $(CFLAGS) \
		$(OBJECTS) \
		tests/test_brain_list.cpp \
		-o bin/test_brain_list \
		$(LIBDIRS) $(LIBS)

test_brain_list_presenter: $(OBJECTS) tests/test_brain_list_presenter.cpp
	$(CXX) -g $(INCLUDEDIRS) $(CFLAGS) \
		$(OBJECTS) \
		tests/test_brain_list_presenter.cpp \
		-o bin/test_brain_list_presenter \
		$(LIBDIRS) $(LIBS)

