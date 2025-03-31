#ifndef H_BRAIN_ENTITY
#define H_BRAIN_ENTITY

#include <string>

using namespace std;

class BrainEntity {
public:
	BrainEntity(string _id, string _name, uint64_t _timestamp)
		: id(_id), name(_name), timestamp(_timestamp) {};
	string id;
	string name;	
	uint64_t timestamp;
};

#endif

