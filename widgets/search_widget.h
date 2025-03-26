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
	SearchWidget(QWidget*, Style*, bool, QString, bool dark);
	// Updates.
	void setItems(std::vector<ConnectionItem>);
	// Removes text and frees focus.
	void clear();
	// Removes text but keeps focus.
	void reset();
	// State.
	bool isActive() const;

signals:
	void searchActivated(SearchWidget*);
	void searchCanceled(SearchWidget*);
	void textChanged(SearchWidget*, QString);
	void thoughtSelected(SearchWidget*, ThoughtId, QString);
	void connectionSelected(SearchWidget*, ThoughtId, QString, ConnectionType, bool);
	void updated(SearchWidget*);

private slots:
	// Text editing.
	void onTextChanged();
	void onTextEdit();
	void onTextCanceled();
	void onTextConfirmed(std::function<void(bool)>);
	void onFocusLost();
	// List selection.
	void onThoughtSelected(ThoughtId, QString);
	void onConnectionSelected(ThoughtId, QString, ConnectionType, bool);
	// List navigation.
	void onNextSuggestion();
	void onPrevSuggestion();

private:
	// Subviews.
	QHBoxLayout *m_input_layout = nullptr;
	QLabel *m_icon = nullptr;
	ThoughtEditWidget *m_edit = nullptr;
	ConnectionListWidget *m_list = nullptr;
	QVBoxLayout m_layout;
	QWidget m_separator;
	// State.
	bool m_showButtons = false;
	bool m_active = false;
	Style *m_style = nullptr;
	// Helpers.
	void restyleIcon(bool);
};

#endif

