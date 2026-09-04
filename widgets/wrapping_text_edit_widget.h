#ifndef H_WRAPPING_TEXT_EDIT_WIDGET
#define H_WRAPPING_TEXT_EDIT_WIDGET

#include <QTextEdit>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QFocusEvent>
#include <QWidget>
#include <QString>
#include <QMimeData>

class WrappingTextEditWidget: public QTextEdit {
	Q_OBJECT

public:
	WrappingTextEditWidget(
		QWidget* parent
	);

	void setTitle(const QString& title);
	QString& getTitle();
	int heightForWidth(int width) const override;
	bool hasHeightForWidth() const override;

signals:
	void editConfirmed(QString&, std::function<void(bool)>);

protected:
	void enterEvent(QEnterEvent *) override;
	void leaveEvent(QEvent *) override;
	void keyPressEvent(QKeyEvent *) override;
	void mousePressEvent(QMouseEvent *) override;
	void focusOutEvent(QFocusEvent *) override;
	void insertFromMimeData(const QMimeData *) override;

private:
	void cancel();

	QString m_title;

	static const int maxLength = 200; // Max thought title length.
};

#endif
