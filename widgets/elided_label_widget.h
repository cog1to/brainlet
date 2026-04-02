#ifndef H_ELIDED_LABEL_WIDGET
#define H_ELIDED_LABEL_WIDGET

#include <QFrame>
#include <QString>
#include <QMargins>
#include <QPaintEvent>
#include <QTextLayout>

class ElidedLabelWidget: public QFrame {
	Q_OBJECT

public:
	ElidedLabelWidget(QWidget*, QString, bool);
	QSize sizeHint() const override;
	int heightForWidth(int) const override;
	bool hasHeightForWidth() const override;
	// State.
	void setText(QString);

protected:
	void paintEvent(QPaintEvent*) override;

private:
	QString m_text;
	bool m_multiline;
};

#endif
