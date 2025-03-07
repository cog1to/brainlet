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
	GetResult(TextRepositoryError _err, std::string _res): error(_err), result(_res) {};
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
};

#endif
