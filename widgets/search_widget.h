#ifndef H_SEARCH_WIDGET
#define H_SEARCH_WIDGET

#include <vector>

#include <QString>
#include <QLabel>
#include <QWidget>
#include <QFrame>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "model/thought.h"
#include "widgets/style.h"
#include "widgets/base_widget.h"
#include "widgets/connection_list_widget.h"
#include "widgets/thought_edit_widget.h"

class SearchWidget: public QFrame {
	Q_OBJECT

public:
	SearchWidget(QWidget*, Style*, bool);
	// Updates.
	void setItems(std::vector<ConnectionItem>);
	void clear();
	// Size.
	QSize sizeHint() const override;
	// State.
	bool isActive() const;

signals:
	void searchActivated(SearchWidget*);
	void searchCanceled(SearchWidget*);
	void textChanged(SearchWidget*, QString);
	void thoughtSelected(SearchWidget*, ThoughtId);
	void connectionSelected(SearchWidget*, ThoughtId, ConnectionType, bool);

private slots:
	// Text editing.
	void onTextChanged();
	void onTextEdit();
	void onTextCancel();
	// List selection.
	void onThoughtSelected(ThoughtId);
	void onConnectionSelected(ThoughtId, ConnectionType, bool);

private:
	// Subviews.
	QHBoxLayout *m_input_layout = nullptr;
	QLabel *m_icon = nullptr;
	ThoughtEditWidget *m_edit = nullptr;
	QVBoxLayout m_layout;
	ConnectionListWidget m_list;
	QWidget m_separator;
	// State.
	bool m_showButtons = false;
	Style *m_style = nullptr;
	// Helpers.
	void restyleIcon(bool);
};

#endif

