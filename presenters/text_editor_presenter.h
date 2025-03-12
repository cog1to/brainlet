#ifndef H_TEXT_EDITOR_PRESENTER
#define H_TEXT_EDITOR_PRESENTER

#include <QObject>
#include <QString>

#include "model/thought.h"
#include "entity/text_repository.h"
#include "widgets/markdown_widget.h"

class TextEditorPresenter: public QObject {
	Q_OBJECT

public:
	TextEditorPresenter(TextRepository*, MarkdownWidget*);
	void setThought(ThoughtId);

signals:
	void textError(MarkdownError);

private slots:
	void onTextChanged(QString&);

private:
	ThoughtId m_id = InvalidThoughtId;
	TextRepository *m_repository = nullptr;
	MarkdownWidget *m_view = nullptr;
};

#endif
