#ifndef H_ANCHOR_WIDGET
#define H_ANCHOR_WIDGET

#include <QObject>
#include <QWidget>
#include <QBrush>
#include <QEnterEvent>
#include <QMouseEvent>

#include "widgets/style.h"
#include "widgets/base_widget.h"

enum AnchorType { AnchorLink, AnchorParent, AnchorChild };

class AnchorWidget: public BaseWidget {
	Q_OBJECT

public:
	// Constructor.
	AnchorWidget(QWidget*, Style*, AnchorType, bool);
	~AnchorWidget();
	// Properties.
	const bool active() const;
	void setActive(bool);
	const AnchorType type() const;
	void setEnabled(bool);
	// Method overrides.
	QSize sizeHint() const override;
	// Size.
	static constexpr QSize defaultSize = QSize(24, 24);

signals:
	void mouseEnter(AnchorWidget*);
	void mouseLeave(AnchorWidget*);
	void mouseMove(AnchorWidget*, QPoint);
	void mouseRelease(AnchorWidget*, QPoint);
	void mouseCancel(AnchorWidget*);

protected:
	// Event overrides.
	void paintEvent(QPaintEvent*) override;
	void enterEvent(QEnterEvent*) override;
	void leaveEvent(QEvent*) override;
	void mousePressEvent(QMouseEvent*) override;
	void mouseReleaseEvent(QMouseEvent*) override;
	void mouseMoveEvent(QMouseEvent*) override;
	void keyPressEvent(QKeyEvent *) override;

private:
	// Members.
	AnchorType m_type;
	bool m_active;
	bool m_pressed;
	QPoint m_dragStart;
};

#endif

