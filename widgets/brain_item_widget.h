#ifndef H_BRAIN_ITEM_WIDGET
#define H_BRAIN_ITEM_WIDGET

#include <QWidget>
#include <QFrame>
#include <QPushButton>
#include <QEvent>
#include <QResizeEvent>
#include <QEnterEvent>
#include <QMouseEvent>
#include <QHBoxLayout>

#include "widgets/style.h"
#include "widgets/base_widget.h"
#include "widgets/elided_label_widget.h"

class BrainItemWidget: public QFrame {
	Q_OBJECT

	enum Status {
		StatusNormal,
		StatusHover,
		StatusPressed
	};

public:
	BrainItemWidget(QWidget*, Style*, QString, QString);
	const QString id() const;
	const QString name() const;
	void setName(QString);
	QSize sizeHint() const override;

protected:
	// Event overrides.
	void enterEvent(QEnterEvent*) override;
	void leaveEvent(QEvent*) override;
	void mousePressEvent(QMouseEvent*) override;
	void mouseReleaseEvent(QMouseEvent*) override;

signals:
	void deleteClicked(BrainItemWidget*);
	void buttonClicked(BrainItemWidget*);

private slots:
	void onClick();
	void onDeleteClick();

private:
	Style *m_style;
	QString m_id;
	QString m_name;
	// Contents
	QHBoxLayout m_layout;
	ElidedLabelWidget *m_label = nullptr;
	// TODO: Button idea: hold-to-activate widget that fills with
	// color left-to-right while pressing it, and sends the signal
	// when filled completely.
	QPushButton *m_deleteButton = nullptr;
	// Helpers.
	static inline QString getStyle(Style *style, Status status);
};

#endif

