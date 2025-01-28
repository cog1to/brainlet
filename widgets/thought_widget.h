#ifndef H_THOUGHT_WIDGET
#define H_THOUGHT_WIDGET

#include <string>

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
	ThoughtWidget(QWidget*, Style*, ThoughtId, bool, std::string, bool, bool, bool);
	~ThoughtWidget();
	// Id.
	const ThoughtId id() const;
	// Properties.
	const bool readOnly() const;
	void setReadOnly(bool);
	const bool hasParent() const;
	void setHasParent(bool);
	const bool hasChild() const;
	void sethHasChild(bool);
	const bool hasLink() const;
	void setHasLink(bool);
	const std::string text() const;
	void setText(std::string);
	// Method override.
	QSize sizeHint() const override;
	// Calculates bounding rect for given width without height restriction.
	QSize sizeForWidth(int width) const;
	// Current state.
	const bool isActive() const;
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
	void textConfirmed(ThoughtWidget*, QString, std::function<void(bool)>);

protected slots:
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
	QString m_text;
	QString m_originalText;
	QTextCursor m_cursor;
	// Helpers.
	void updateText();
};

#endif
