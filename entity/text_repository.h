#ifndef H_TEXT_REPOSITORY
#define H_TEXT_REPOSITORY

#include <string>

#include "model/model.h"

enum TextRepositoryError {
	TextRepositoryNone,
	TextRepositoryIOError
};

struct GetResult {
	TextRepositoryError error;
	std::string result;	
public:
	GetResult(TextRepositoryError _err, std::string _res)
		: error(_err), result(_res) {};
};

struct SaveResult {
	TextRepositoryError error;
public:
	SaveResult(TextRepositoryError _err): error(_err) {};
};

class TextRepository {
public:
	virtual GetResult getText(ThoughtId) = 0;
	virtual SaveResult saveText(ThoughtId, std::string) = 0;
	// This method copies the method from GraphRepository. I don't know
	// if this is the "correct" way, but it feels right in terms of
	// separation of data access interfaces for separate logical/UI
	// components. But it also increases cognitive load, because we have
	// to remember that there are two places to modify the method. I
	// might revert this change later, I don't know...
	virtual bool connectThoughts(ThoughtId, ThoughtId, ConnectionType) = 0;
};

#endif
