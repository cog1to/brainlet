# Install prefix
PREFIX=/usr/local

# Basic config
INCLUDEDIRS = `pkg-config --cflags Qt6Core Qt6Widgets Qt6Gui Qt6Sql Qt6DBus` -I.
LIBDIRS = `pkg-config --libs-only-L Qt6Core Qt6Widgets Qt6Gui Qt6Sql Qt6DBus`
LIBS = `pkg-config --libs-only-l Qt6Core Qt6Widgets Qt6Gui Qt6Sql Qt6DBus`
CFLAGS = ${FLAGS} -fPIC
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
	LIBS = -framework QtWidgets \
				 -framework QtCore \
				 -framework QtGui \
				 -framework QtDBus \
				 -framework QtSql
	MOC = ${QTDIR}/share/qt/libexec/moc
	RCC = ${QTDIR}/share/qt/libexec/rcc
	INCLUDEDIRS = -I. \
								-I${QTDIR}/include \
								-I${QTDIR}/include/QtCore \
								-I${QTDIR}/include/QtWidgets \
								-I${QTDIR}/include/QtGui \
								-I${QTDIR}/include/QtSql \
								-I${QTDIR}/include/QtDBus
	CFLAGS += -DDARWIN=1
endif

# Debug flags
ifdef DEBUG
	CFLAGS += -DDEBUG_GUI=1 -g -fsanitize=address,undefined,leak
else
	CFLAGS += -O2 -DQT_NO_DEBUG_OUTPUT=1
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
SOURCES = $(shell find . \( -path ./resources -prune -o -path ./tests -prune -o -path ./mocs -prune -o -path ./main.cpp -prune \) -o -name "*.cpp" -print | sed -e 's/\.\///') $(WIDGETS_MOCS_C) $(PRESENTERS_MOCS_C) $(RESOURCES_C)
OBJECTS = $(patsubst %.cpp,obj/%.o,$(SOURCES))
# All moc files
MOCS = $(WIDGETS_MOCS_C) $(PRESENTERS_MOCS_C)

# Don't delete obj and moc files.
.PRECIOUS: $(OBJECTS) $(MOCS)

# Main target
default: app

# Clean
clean:
	rm -rf mocs/* bin/* obj/* build/*

# Directories
bin:
	mkdir -p bin

# Tests with debug graphics
debug: CFLAGS += -DDEBUG_GUI=1

# Resources
$(RESOURCES_C): $(RESOURCES) $(wildcard resources/icons/*)
	$(RCC) -name resources $(RESOURCES) -o $(RESOURCES_C)

# MOCs
mocs/%widget.moc.cpp: widgets/%widget.h
	@mkdir -p $(@D)
	$(MOC) $< -o $@

mocs/%presenter.moc.cpp: presenters/%presenter.h
	@mkdir -p $(@D)
	$(MOC) $< -o $@

mocs/style.moc.cpp: widgets/style.h
	@mkdir -p $(@D)
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

# App target
app: build/brainlet build/brainlet.desktop build/brainlet.png

build/brainlet: $(OBJECTS) main.cpp
	@mkdir -p $(@D)
	$(CXX) $(INCLUDEDIRS) $(CFLAGS) \
		$^ -o $@ \
		$(LIBDIRS) $(LIBS)

build/brainlet.desktop: brainlet.desktop.in
	@mkdir -p $(@D)
	cp $< $@
	sed "s|PREFIX|$(PREFIX)|" <$< >$@

build/brainlet.png: resources/icons/icon.png
	@mkdir -p $(@D)
	cp $< $@

# Install target
install: build/brainlet build/brainlet.desktop build/brainlet.png
	mkdir -p $(PREFIX)/share/applications/ && \
		cp build/brainlet.desktop $(PREFIX)/share/applications/
	mkdir -p $(PREFIX)/share/icons/hicolor/256x256/apps/ && \
		cp build/brainlet.png $(PREFIX)/share/icons/hicolor/256x256/apps/
	mkdir -p $(PREFIX)/bin && \
		cp build/brainlet $(PREFIX)/bin/

# Experimental MacOS target.
mac: build/brainlet build/brainlet.png
	mkdir -p build/Brainlet.app/Contents/MacOS
	cp build/brainlet build/Brainlet.app/Contents/MacOS/brainlet
	cp resources/mac/Info.plist build/Brainlet.app/Contents/Info.plist
	mkdir -p build/Brainlet.app/Contents/Resources
	cp resources/icons/* build/Brainlet.app/Contents/Resources/
	# Hacky as hell. I have no idea why libpath doesn't work properly,
	# But we have to execute this command twice to make a somewhat working
	# build.
	macdeployqt build/Brainlet.app -libpath=${QTDIR}/lib -libpath=/opt/homebrew/lib \
		-always-overwrite -no-strip -codesign=${CODESIGN}
	macdeployqt build/Brainlet.app -libpath=${QTDIR}/lib -libpath=/opt/homebrew/lib \
		-always-overwrite -no-strip -codesign=${CODESIGN}
