# Basic config
INCLUDEDIRS = `pkg-config --cflags Qt6Core Qt6Widgets Qt6Gui Qt6Sql Qt6DBus` -I.
LIBDIRS = `pkg-config --libs-only-L Qt6Core Qt6Widgets Qt6Gui Qt6Sql Qt6DBus`
LIBS = `pkg-config --libs-only-l Qt6Core Qt6Widgets Qt6Gui Qt6Sql Qt6DBus`
CFLAGS = ${FLAGS} -fPIC -fsanitize=address,undefined,leak

# Utils
QTLIBEXEC = `pkg-config --variable=libexecdir Qt6Core`
MOC = ${QTLIBEXEC}/moc
RCC = ${QTLIBEXEC}/rcc

# MacOS overrides
# TODO: Is there a way to query install paths from homebrew?
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	CXX = /opt/homebrew/opt/llvm/bin/clang++
	LIBDIRS = -F${QTDIR}/lib
	LIBS = -framework QtWidgets -framework QtCore -framework QtGui -framework QtDBus -framework QtSql
	MOC = ${QTDIR}/share/qt/libexec/moc
	RCC = ${QTDIR}/share/qt/libexec/rcc
	CFLAGS += -DDARWIN=1
endif

# Widgets
WIDGETS_H = $(wildcard widgets/*.h)
WIDGETS_MOCS_C = $(patsubst widgets/%.cpp,mocs/%.cpp,$(WIDGETS_H:.h=.moc.cpp))
# Presenters
PRESENTERS_H = $(wildcard presenters/*.h)
PRESENTERS_MOCS_C = $(patsubst presenters/%.cpp,mocs/%.cpp,$(PRESENTERS_H:.h=.moc.cpp))
# Resources
RESOURCES = resources/resources.qrc
RESOURCES_C = resources/resources.cpp
# All object files
SOURCES = $(shell find . \( -path ./resources -prune -o -path ./tests -prune -o -path ./mocs -prune \) -o -name "*.cpp" -print | sed -e 's/\.\///') $(WIDGETS_MOCS_C) $(PRESENTERS_MOCS_C) $(RESOURCES_C)
OBJECTS = $(patsubst %.cpp,obj/%.o,$(SOURCES))

.PRECIOUS: $(OBJECTS)

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

# Resources
$(RESOURCES_C): $(RESOURCES) $(wildcard resources/icons/*)
	$(RCC) -name resources $(RESOURCES) -o $(RESOURCES_C)

# MOCs
mocfiles: moc $(WIDGETS_MOCS_C) $(PRESENTERS_MOCS_C)

mocs/%widget.moc.cpp: widgets/%widget.h
	$(MOC) $< -o $@

mocs/%presenter.moc.cpp: presenters/%presenter.h
	$(MOC) $< -o $@

mocs/style.moc.cpp: widgets/style.h
	$(MOC) $< -o $@

# Object rules
obj/resources/resources.o: $(RESOURCES_C)
	@mkdir -p $(@D)
	$(CXX) -c $< -g $(INCLUDEDIRS) $(CFLAGS) \
		-o $@

obj/%.o: %.cpp %.h
	@mkdir -p $(@D)
	$(CXX) -c $< -g $(INCLUDEDIRS) $(CFLAGS) \
		-o $@

obj/mocs/%.o: mocs/%.cpp
	@mkdir -p $(@D)
	$(CXX) -c $< -g $(INCLUDEDIRS) $(CFLAGS) \
		-o $@

# Test targets
TESTS = $(patsubst tests/%.cpp,bin/%,$(wildcard tests/*.cpp))

bin/test_%: $(OBJECTS) tests/test_%.cpp
	$(CXX) -g $(INCLUDEDIRS) $(CFLAGS) \
		$^ -o $@ \
		$(LIBDIRS) $(LIBS)

tests: $(TESTS)

