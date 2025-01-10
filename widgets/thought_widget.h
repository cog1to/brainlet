#ifndef H_THOUGHT_WIDGET
#define H_THOUGHT_WIDGET

#include <string>
#include <QObject>
#include <QWidget>
#include <QResizeEvent>
#include <QTextEdit>

#include "widgets/style.h"
#include "widgets/base_widget.h"
#include "widgets/anchor_widget.h"
#include "widgets/thought_edit_widget.h"

class ThoughtWidget: public BaseWidget {
	Q_OBJECT

public:
	// Constructor and destructor.
	ThoughtWidget(QWidget*, Style*, std::string, bool, bool, bool);
	~ThoughtWidget();
	// Properties.
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

signals:
	void textMouseEnter(ThoughtWidget*);
	void textMouseLeave(ThoughtWidget*);

protected slots:
	void onTextEnter();
	void onTextLeave();

protected:
	// Event overrides.
	void paintEvent(QPaintEvent *) override;
	void resizeEvent(QResizeEvent *) override;
	// Children.
	ThoughtEditWidget m_textEdit;
	AnchorWidget m_anchorLink;
	AnchorWidget m_anchorParent;
	AnchorWidget m_anchorChild;
	// Size constants.
	static constexpr QSize padding = QSize(10, 2);

private:
	// State.
	bool m_hover = false;
	QString m_text;
	// Helpers.
	void updateText();
};

#endif
