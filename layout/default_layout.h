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
	void layoutVerticalSide(const std::vector<ThoughtId>&, QRect);
	// Sizing helpers.
	QSize widgetSize(std::string text, float);
	// State.
	std::unordered_map<ThoughtId, ItemLayout> m_layout;
	// Layout settings.
	static constexpr float s_centerWidgetSize = 0.3;
	static constexpr float s_widgetSize = 0.25;
	static constexpr float s_sidePanelWidth = 0.3;
	static constexpr float s_minVerticalSpacing = 0.05;
};

#endif

