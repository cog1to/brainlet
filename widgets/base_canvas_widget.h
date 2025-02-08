#ifndef H_BASE_CANVAS_WIDGET
#define H_BASE_CANVAS_WIDGET

#include <unordered_map>
#include <vector>

#include <QWidget>
#include <QResizeEvent>
#include <QWheelEvent>

#include "layout/base_layout.h"
#include "widgets/style.h"
#include "widgets/base_widget.h"
#include "widgets/anchor_highlight_widget.h"
#include "widgets/thought_widget.h"
#include "widgets/scroll_area_widget.h"

struct AnchorSource {
	ThoughtWidget *widget;
	AnchorType type;
	AnchorSource(ThoughtWidget *w, AnchorType t): widget(w), type(t) {}
};

class BaseCanvasWidget: public BaseWidget {
	Q_OBJECT

public:
	BaseCanvasWidget(QWidget*, Style*, BaseLayout*);
	~BaseCanvasWidget();

signals:
	void textChanged(ThoughtId, QString, std::function<void(bool)>);
	void thoughtSelected(ThoughtId);
	void thoughtCreated(ThoughtId, ConnectionType, bool, QString, std::function<void(bool, ThoughtId)>);
	void onShown();

protected:
	// Event overrides.
	void paintEvent(QPaintEvent *) override;
	void resizeEvent(QResizeEvent *) override;
	void showEvent(QShowEvent *) override;

private:
	BaseLayout *m_layout = nullptr;
	// Main content widgets.
	std::unordered_map<ThoughtId, ThoughtWidget*> m_widgets;
	std::unordered_map<unsigned int, ScrollAreaWidget*> m_scrollAreas;
	// Anchor highlight.
	AnchorHighlightWidget m_anchorHighlight;
	// Thought/connection creation.
	AnchorSource *m_anchorSource = nullptr;
	ThoughtWidget *m_newThought = nullptr;
	ThoughtWidget *m_overThought = nullptr;
	QWidget m_overlay;
	// Layout.
	void updateLayout();
	void layoutScrollAreas();
	void drawAnchorConnection(QPainter&);
	void drawNewThoughtConnection(QPainter& painter);
	void drawOverThoughtConnection(QPainter& painter);
	void drawConnection(
		QPainter& painter,
		AnchorPoint outgoing, AnchorPoint incoming,
		QPen& pen
	);
	ThoughtWidget *cachedWidget(ThoughtId id);
	ThoughtWidget *createWidget(const ItemLayout&, bool);
	void connectWidget(ThoughtWidget*);
	ScrollAreaWidget *cachedScrollArea(unsigned int id);
	ScrollAreaWidget *createScrollArea(unsigned int id, ScrollBarPos);
	// Helpers.
	void clearAnchor();
	AnchorType reverseAnchorType(AnchorType type);
	ThoughtWidget *widgetUnder(QPoint);
	void setupNewThought();
	// Layout constants.
	static constexpr qreal controlPointRatio = 0.5;
	static constexpr int minAnchorDistance = 25;

private slots:
	void onWidgetClicked(ThoughtWidget*);
	void onWidgetActivated(ThoughtWidget*);
	void onWidgetDeactivated(ThoughtWidget*);
	void onWidgetScroll(ThoughtWidget*, QWheelEvent*);
	void onScrollAreaScroll(unsigned int, int);
	void onAnchorEntered(ThoughtWidget*, AnchorType, QPoint);
	void onAnchorLeft();
	void onAnchorMoved(QPoint);
	void onAnchorReleased(QPoint);
	void onAnchorCanceled();
	void onTextConfirmed(ThoughtWidget*, QString, std::function<void(bool)>);
	void onCreateCanceled(ThoughtWidget*);
	void onCreateConfirmed(ThoughtWidget*, QString, std::function<void(bool)>);
};

#endif

