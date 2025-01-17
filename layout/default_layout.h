#ifndef H_DEFAULT_LAYOUT
#define H_DEFAULT_LAYOUT

#include "layout/base_layout.h"

class DefaultLayout: public BaseLayout {
public:
	DefaultLayout(Style*);
	void reload() override;
	void setSize(QSize) override;
	const std::unordered_map<ThoughtId, ItemLayout>* items() const override;

private:
	// Helpers.
	void updateWidgets();
	void layoutHorizontalSide(const std::vector<ThoughtId>&, QRect);
	void layoutVerticalSide(const std::vector<ThoughtId>&, QRect);
	// Sizing helpers.
	QSize widgetSize(std::string text, int);
	// State.
	std::unordered_map<ThoughtId, ItemLayout> m_layout;
	// Layout settings.
	int m_widgetWidth = 0;
	int m_widgetHeight = 0;
	int m_widgetSpacing = 0;
	int m_sidePadding = 10;
};

#endif

