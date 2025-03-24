#ifndef H_BASE_CANVAS_WIDGET
#define H_BASE_CANVAS_WIDGET

#include <unordered_map>
#include <vector>

#include <QWidget>
#include <QResizeEvent>
#include <QWheelEvent>
#include <QPainterPath>
#include <QMouseEvent>

#include "layout/base_layout.h"
#include "widgets/style.h"
#include "widgets/base_widget.h"
#include "widgets/anchor_highlight_widget.h"
#include "widgets/thought_widget.h"
#include "widgets/scroll_area_widget.h"
#include "widgets/connection_list_widget.h"

struct AnchorSource {
	ThoughtWidget *widget;
	AnchorType type;
	AnchorSource(ThoughtWidget *w, AnchorType t): widget(w), type(t) {}
};

struct Path {
	ThoughtWidget *from, *to;
	QPen pen;
	QPainterPath path;
	Path(ThoughtWidget *_l, ThoughtWidget *_r, QPen& _pen, QPainterPath& _path)
		: from(_l), to(_r), pen(_pen), path(_path) {}
};

class CanvasWidget: public BaseWidget {
	Q_OBJECT

public:
	CanvasWidget(QWidget*, Style*, BaseLayout*);
	~CanvasWidget();
	QSize sizeHint() const override;
	// Suggestions.
	void showSuggestions(std::vector<ConnectionItem>);
	void hideSuggestions();

signals:
	void textChanged(ThoughtId, QString, std::function<void(bool)>);
	void thoughtSelected(ThoughtId);
	void thoughtCreated(ThoughtId, ConnectionType, bool, QString, std::function<void(bool, ThoughtId)>);
	void thoughtConnected(ThoughtId, ThoughtId, ConnectionType, std::function<void(bool)>);
	void thoughtDeleted(ThoughtId);
	void thoughtsDisconnected(ThoughtId, ThoughtId);
	void shown();
	void newThoughtTextChanged(QString);

protected:
	// Event overrides.
	void paintEvent(QPaintEvent *) override;
	void resizeEvent(QResizeEvent *) override;
	void showEvent(QShowEvent *) override;
	void mouseMoveEvent(QMouseEvent *) override;
	void mousePressEvent(QMouseEvent *) override;

private:
	BaseLayout *m_layout = nullptr;
	// Main content widgets.
	std::unordered_map<ThoughtId, ThoughtWidget*> m_widgets;
	std::unordered_map<unsigned int, ScrollAreaWidget*> m_scrollAreas;
	// Connections.
	std::vector<Path> m_paths;
	// Anchor highlight.
	AnchorHighlightWidget m_anchorHighlight;
	// Thought/connection creation.
	AnchorSource *m_anchorSource = nullptr;
	ThoughtWidget *m_newThought = nullptr;
	ThoughtWidget *m_overThought = nullptr;
	ThoughtWidget *m_menuThought = nullptr;
	QWidget m_overlay;
	std::optional<Path> m_pathHighlight;
	// Suggestions.
	ConnectionListWidget *m_suggestions = nullptr;	
	QFrame *m_suggestionsContainer = nullptr;
	void layoutSuggestions();
	// Layout.
	void updateLayout();
	void updatePaths();
	Path makePath(ThoughtWidget*, ThoughtWidget*, AnchorPoint, AnchorPoint, QPen&);
	void layoutScrollAreas();
	void drawAnchorConnection(QPainter&);
	void drawNewThoughtConnection(QPainter& painter);
	void drawOverThoughtConnection(QPainter& painter);
	void drawConnection(QPainter& painter, Path&);
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
	void updateConnection();
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
	void onMenuRequested(ThoughtWidget*, const QPoint&);
	void onDeleteThought();
	void onDisconnect();
	void onNewThoughtTextChanged(ThoughtWidget*);
	void onSuggestionSelected(ThoughtId, QString);
	void onNextSuggestion();
	void onPrevSuggestion();
	void onMenuHide();
};

#endif

