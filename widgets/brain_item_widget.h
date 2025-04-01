#ifndef H_BRAIN_ITEM_WIDGET
#define H_BRAIN_ITEM_WIDGET

#include <QWidget>
#include <QPushButton>
#include <QEvent>
#include <QResizeEvent>
#include <QEnterEvent>

#include "widgets/style.h"

class BrainItemWidget: public QPushButton {
	Q_OBJECT

public:
	BrainItemWidget(QWidget*, Style*, QString, QString);
	const QString id() const;
	const QString name() const;
	void setName(QString);

protected:
	// Event overrides.
	void resizeEvent(QResizeEvent*) override;
	void enterEvent(QEnterEvent*) override;
	void leaveEvent(QEvent*) override;

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
	// TODO: Button idea: hold-to-activate widget that fills with
	// color left-to-right while pressing it, and sends the signal
	// when filled completely.
	QPushButton m_deleteButton;
};

#endif

