#ifndef H_THOUGHT_WIDGET
#define H_THOUGHT_WIDGET

#include <QObject>
#include <QWidget>
#include <QResizeEvent>
#include <QTextEdit>

#include "model/thought.h"
#include "layout/item_connection.h"
#include "widgets/style.h"
#include "widgets/base_widget.h"
#include "widgets/anchor_widget.h"
#include "widgets/thought_edit_widget.h"

typedef struct {
	qreal x, y;
	qreal dx, dy;
} AnchorPoint;

class ThoughtWidget: public BaseWidget {
	Q_OBJECT

public:
	// Constructor and destructor.
	ThoughtWidget(
		QWidget *parent,
		Style *style,
		ThoughtId id,
		bool readOnly,
		QString name,
		bool hasParent, bool hasChild, bool hasLink,
		bool rightSideLink = false,
		bool canDelete = true,
		bool anchorsActive = true
	);
	~ThoughtWidget();
	// Id.
	const ThoughtId id() const;
	void setId(ThoughtId);
	// Properties.
	const bool readOnly() const;
	void setReadOnly(bool);
	const bool hasParent() const;
	void setHasParent(bool);
	const bool hasChild() const;
	void setHasChild(bool);
	const bool hasLink() const;
	void setHasLink(bool);
	const QString text() const;
	void setText(QString);
	const bool rightSideLink() const;
	void setRightSideLink(bool);
	const bool canDelete() const;
	void setCanDelete(bool);
	const bool anchorsActive() const;
	void setAnchorsActive(bool);
	// Flag for setting currently focused widget. This doesn't affect
	// it's input focus state, but highlights it.
	void setFocused(bool);
	// Method override.
	QSize sizeHint() const override;
	// Calculates bounding rect for given width without height restriction.
	QSize sizeForWidth(int width) const;
	// Current state.
	const bool isActive() const;
	void setHighlight(bool);
	void activate();
	void removeFocus();
	// Anchor ponts for connections.
	AnchorPoint getAnchorFrom(ConnectionType);
	AnchorPoint getAnchorTo(ConnectionType);
	AnchorPoint getAnchorFrom(AnchorType);

signals:
	void activated(ThoughtWidget*);
	void deactivated(ThoughtWidget*);
	void textChanged(ThoughtWidget*);
	void mouseScroll(ThoughtWidget*, QWheelEvent*);
	void anchorEntered(ThoughtWidget*, AnchorType, QPoint);
	void anchorLeft();
	void anchorMoved(QPoint);
	void anchorReleased(QPoint);
	void anchorCanceled();
	void textConfirmed(ThoughtWidget*, QString, std::function<void(bool)>);
	void clicked(ThoughtWidget*);
	void textCanceled(ThoughtWidget*);
	void menuRequested(ThoughtWidget*, const QPoint&);
	void nextSuggestion();
	void prevSuggestion();

protected slots:
	void onClick();
	void onTextEnter();
	void onTextLeave();
	void onTextClearFocus();
	void onTextChanged();
	void onTextEdit();
	void onTextCancel();
	void onTextConfirmed(std::function<void(bool)>);
	void onAnchorEntered(AnchorWidget*);
	void onAnchorLeft(AnchorWidget*);
	void onAnchorMove(AnchorWidget*, QPoint);
	void onAnchorRelease(AnchorWidget*, QPoint);
	void onAnchorCanceled(AnchorWidget*);
	void onMenuRequested(const QPoint&);

protected:
	// Event overrides.
	void paintEvent(QPaintEvent *) override;
	void resizeEvent(QResizeEvent *) override;
	void wheelEvent(QWheelEvent *) override;
	// Children.
	ThoughtEditWidget m_textEdit;
	AnchorWidget m_anchorLink;
	AnchorWidget m_anchorParent;
	AnchorWidget m_anchorChild;
	// Size constants.
	static constexpr QSize padding = QSize(10, 2);
	static constexpr qreal parentRightOffset = 0.3;
	static constexpr qreal childLeftOffset = 0.3;

private:
	// State.
	ThoughtId m_id;
	bool m_hover = false;
	bool m_highlight = false;
	bool m_rightSideLink = false;
	bool m_canDelete = true;
	bool m_anchorsActive = true;
	bool m_focused = false;
	QString m_text;
	QString m_originalText;
	QTextCursor m_cursor;
	// Helpers.
	void updateText();
	void updateLayout(QSize);
};

#endif
