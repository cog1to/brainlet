#ifndef H_ANCHOR_WIDGET
#define H_ANCHOR_WIDGET

#include <QObject>
#include <QWidget>
#include <QBrush>

#include "widgets/style.h"
#include "widgets/base_widget.h"

class AnchorWidget: public BaseWidget {
	Q_OBJECT

public:
	// Constructor.
	AnchorWidget(QWidget*, Style*, bool);
	~AnchorWidget();
	// Properties.
	const bool active() const;
	void setActive(bool);
	// Method overrides.
	QSize sizeHint() const override;
	// Size.
	static constexpr QSize defaultSize = QSize(24, 24);

protected:
	// Event overrides.
	void paintEvent(QPaintEvent *event) override;
	// Members.
	bool m_active;
};

#endif

