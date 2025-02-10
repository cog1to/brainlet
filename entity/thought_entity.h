#ifndef H_THOUGHT_ENTITY
#define H_THOUGHT_ENTITY

#include <string>

#include "model/thought.h"

class ThoughtEntity {
public:
	ThoughtEntity(ThoughtId _id, std::string _name): id(_id), name(_name) {};
	ThoughtId id;
	std::string name;
};

#endif

