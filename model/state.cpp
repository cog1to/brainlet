#include <string>
#include <unordered_map>

#include "model/thought.h"
#include "model/state.h"

State::State(Thought *center, std::unordered_map<ThoughtId, Thought*> *thoughts) {
	m_centralThought = center;
	m_thoughts = thoughts;
}

State::~State() {
	if (m_centralThought) {
		delete m_centralThought;
	}

	if (m_thoughts) {
		std::unordered_map<ThoughtId, Thought*>::iterator it;
		for (it = m_thoughts->begin(); it != m_thoughts->end(); it++) {
			delete it->second;
		}

		m_thoughts->clear();
		delete m_thoughts;
	}
}
