# Test targets

TESTS = $(patsubst tests/%.cpp,bin/%,$(wildcard tests/*.cpp))

bin/test_%: $(OBJECTS) tests/test_%.cpp
	$(CXX) -g $(INCLUDEDIRS) $(CFLAGS) \
		$^ -o $@ \
		$(LIBDIRS) $(LIBS)

tests: $(TESTS)

