#ifndef H_DEFAULT_LAYOUT
#define H_DEFAULT_LAYOUT

#include "layout/base_layout.h"
#include "layout/item_layout.h"
#include "layout/scroll_area_layout.h"
#include "layout/item_connection.h"

class DefaultLayout: public BaseLayout {
	enum LayoutConnectionType { child, parent, link };

public:
	DefaultLayout(Style*);
	void reload() override;
	void setSize(QSize) override;
	void setState(const State*) override;
	const ThoughtId* rootId() const override;
	const std::unordered_map<ThoughtId, ItemLayout>* items() const override;
	const std::unordered_map<unsigned int, ScrollAreaLayout>* scrollAreas() const override;
	const std::vector<ItemConnection>* connections() const override;
	const std::vector<ItemConnection>* subconnections() const override;
	void onScroll(unsigned int, int) override;

private:
	// Helpers.
	static inline bool compareThoughts(Thought*, Thought*);
	static inline void sortNodes(
		// Data to sort nodes.
		std::vector<Thought*>&,
		const std::vector<ThoughtId>&,
		const std::unordered_map<ThoughtId, Thought*>*,
		// Data to fill connections:
		std::vector<ItemConnection>*,
		ThoughtId,
		LayoutConnectionType
	);
	static inline bool listContains(std::vector<Thought*>&, ThoughtId);
	void updateWidgets();
	void loadSiblings();
	void layoutHorizontalSide(const std::vector<Thought*>&, QRect, ScrollBarPos);
	void layoutVerticalSide(const std::vector<Thought*>&, QRect, ScrollBarPos);
	// Sizing helpers.
	QSize widgetSize(std::string text, int);
	// State.
	std::vector<Thought*> m_siblings;
	std::vector<Thought*> m_children;
	std::vector<Thought*> m_parents;
	std::vector<Thought*> m_links;
	std::unordered_map<ThoughtId, ItemLayout> m_layout;
	std::unordered_map<unsigned int, ScrollAreaLayout> m_scrollAreas;
	std::unordered_map<unsigned int, int> m_offsets;
	std::vector<ItemConnection> m_connections;
	std::vector<ItemConnection> m_subconnections;
	// Layout settings.
	int m_verticalWidgetWidth = 0;
	int m_widgetHeight = 0;
	int m_widgetSpacing = 0;
	int m_sidePadding = 10;
	int m_topSideHeight = 0;
	int m_leftSideWidth = 0;
	int m_minWidgetWidth = 40;
	// Ratio.
	static constexpr float s_sideRatio = 0.2;
};

#endif

