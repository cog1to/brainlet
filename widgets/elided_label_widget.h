#ifndef H_ELIDED_LABEL_WIDGET
#define H_ELIDED_LABEL_WIDGET

#include <QFrame>
#include <QString>
#include <QMargins>
#include <QPaintEvent>

class ElidedLabelWidget: public QFrame {
	Q_OBJECT

public:
	ElidedLabelWidget(QWidget*, QString);
	QSize sizeHint() const override;
	// State.
	void setText(QString);

protected:
	void paintEvent(QPaintEvent*) override;

private:
	QString m_text;
};

#endif
