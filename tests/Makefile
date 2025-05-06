# Tests
tests: bin test_anchor test_thought test_resize test_edit test_base \
	test_memory test_presenter test_markdown test_text_presenter \
	test_brain_presenter test_conn test_brain_item \
	test_brain_list test_brain_list_presenter test_tabs \
	test_graph_repo test_text_repo test_tabs_with_repo \
	test_tabs_with_config

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

test_tabs: $(OBJECTS) tests/test_tabs.cpp
	$(CXX) -g $(INCLUDEDIRS) $(CFLAGS) \
		$(OBJECTS) \
		tests/test_tabs.cpp \
		-o bin/test_tabs \
		$(LIBDIRS) $(LIBS)

test_brain_list_folder: $(OBJECTS) tests/test_brain_list_folder.cpp
	$(CXX) -g $(INCLUDEDIRS) $(CFLAGS) \
		$(OBJECTS) \
		tests/test_brain_list_folder.cpp \
		-o bin/test_brain_list_folder \
		$(LIBDIRS) $(LIBS)

test_database_brain_load: $(OBJECTS) tests/test_database_brain_load.cpp
	$(CXX) -g $(INCLUDEDIRS) $(CFLAGS) \
		$(OBJECTS) \
		tests/test_database_brain_load.cpp \
		-o bin/test_database_brain_load \
		$(LIBDIRS) $(LIBS)

test_graph_repo: $(OBJECTS) tests/test_graph_repository.cpp
	$(CXX) -g $(INCLUDEDIRS) $(CFLAGS) \
		$(OBJECTS) \
		tests/test_graph_repository.cpp \
		-o bin/test_graph_repository \
		$(LIBDIRS) $(LIBS)

test_text_repo: $(OBJECTS) tests/test_text_repository.cpp
	$(CXX) -g $(INCLUDEDIRS) $(CFLAGS) \
		$(OBJECTS) \
		tests/test_text_repository.cpp \
		-o bin/test_text_repository \
		$(LIBDIRS) $(LIBS)

test_tabs_with_repo: $(OBJECTS) tests/test_tabs_with_repo.cpp
	$(CXX) -g $(INCLUDEDIRS) $(CFLAGS) \
		$(OBJECTS) \
		tests/test_tabs_with_repo.cpp \
		-o bin/test_tabs_with_repo \
		$(LIBDIRS) $(LIBS)

test_tabs_with_config: $(OBJECTS) tests/test_tabs_with_config.cpp
	$(CXX) -g $(INCLUDEDIRS) $(CFLAGS) \
		$(OBJECTS) \
		tests/test_tabs_with_config.cpp \
		-o bin/test_tabs_with_config \
		$(LIBDIRS) $(LIBS)

