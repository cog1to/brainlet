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
#include <QLabel>

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
	BrainItemWidget(QWidget*, Style*, QString, QString, uint64_t);
	const QString id() const;
	const QString name() const;
	void setName(QString);
	const uint64_t brainSize() const;
	void setBrainSize(uint64_t);
	QSize sizeHint() const override;
	// Metrics.
	static const int PADDING = 8;
	static const int SPACING = 8;
	static const int MAX_WIDTH = 220;

protected:
	// Event overrides.
	void enterEvent(QEnterEvent*) override;
	void leaveEvent(QEvent*) override;
	void mousePressEvent(QMouseEvent*) override;
	void mouseReleaseEvent(QMouseEvent*) override;

signals:
	void deleteClicked(BrainItemWidget*);
	void renameClicked(BrainItemWidget*);
	void buttonClicked(BrainItemWidget*);

private slots:
	void onClick();
	void onDeleteClick();
	void onRenameClick();

private:
	Style *m_style;
	QString m_id;
	QString m_name;
	uint64_t m_size;
	// Contents
	QVBoxLayout m_layout;
	ElidedLabelWidget *m_label = nullptr;
	QLabel *m_infoLabel;
	// TODO: Button idea: hold-to-activate widget that fills with
	// color left-to-right while pressing it, and sends the signal
	// when filled completely.
	QPushButton *m_deleteButton = nullptr;
	QPushButton *m_renameButton = nullptr;
	QHBoxLayout *m_buttonsLayout = nullptr;
	// Helpers.
	static inline QString getStyle(Style *style, Status status);
	static inline QString humanReadableSize(uint64_t size);
};

#endif

