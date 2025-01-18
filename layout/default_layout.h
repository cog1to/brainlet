#ifndef H_DEFAULT_LAYOUT
#define H_DEFAULT_LAYOUT

#include "layout/base_layout.h"

class DefaultLayout: public BaseLayout {
public:
	DefaultLayout(Style*);
	void reload() override;
	void setSize(QSize) override;
	void setState(State*) override;
	const std::unordered_map<ThoughtId, ItemLayout>* items() const override;

private:
	// Helpers.
	static inline bool compareThoughts(Thought*, Thought*);
	static inline void sortNodes(
		std::vector<Thought*>&,
		const std::vector<ThoughtId>&,
		const std::unordered_map<ThoughtId, Thought*>*
	);
	void updateWidgets();
	void loadSiblings();
	void layoutHorizontalSide(const std::vector<Thought*>&, QRect);
	void layoutVerticalSide(const std::vector<Thought*>&, QRect);
	// Sizing helpers.
	QSize widgetSize(std::string text, int);
	// State.
	std::vector<Thought*> m_siblings;
	std::vector<Thought*> m_children;
	std::vector<Thought*> m_parents;
	std::vector<Thought*> m_links;
	std::unordered_map<ThoughtId, ItemLayout> m_layout;
	// Layout settings.
	int m_widgetWidth = 0;
	int m_widgetHeight = 0;
	int m_widgetSpacing = 0;
	int m_sidePadding = 10;
};

#endif

