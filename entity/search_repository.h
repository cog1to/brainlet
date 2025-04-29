#ifndef H_SEARCH_REPOSITORY
#define H_SEARCH_REPOSITORY

#include <string>

#include "model/thought.h"

enum SearchError {
	SearchErrorNone,
	SearchErrorIO
};

struct SearchItem {
	ThoughtId id;
	std::string name;
};

struct SearchResult {
	SearchError error;
	std::vector<SearchItem> items;
};

class SearchRepository {
public:
	virtual SearchResult search(std::string) = 0;
};

#endif
