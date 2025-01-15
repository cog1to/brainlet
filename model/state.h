#ifndef H_STATE_MODEL
#define H_STATE_MODEL

#include <unordered_map>

#include "model/thought.h"

/**
 * State holds currently loaded chunk of a brain.
 * When constructing State with a central Thought and peripheral Thoughs map,
 * it assumes ownerwhip of those objects, and deletes them when destroyed.
 */
class State {
public:
	// Constructor and destructor.
	State(Thought*, std::unordered_map<ThoughtId, Thought*>*);
	~State();
	// Properties.
	const Thought *centralThought() const { return m_centralThought; }
	const std::unordered_map<ThoughtId, Thought*>* thoughts() const { return m_thoughts; }

private:
	Thought *m_centralThought = nullptr;
	std::unordered_map<ThoughtId, Thought*> *m_thoughts = nullptr;
};

#endif
