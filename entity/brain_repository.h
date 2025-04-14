#ifndef H_BRAIN_REPOSITORY
#define H_BRAIN_REPOSITORY

#include "entity/graph_repository.h"
#include "entity/search_repository.h"
#include "entity/text_repository.h"

class BrainRepository:
	public GraphRepository,
	public SearchRepository,
	public TextRepository
{
protected:
	BrainRepository();
};

#endif
