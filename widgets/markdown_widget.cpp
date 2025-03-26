#include <QObject>
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QTextBlock>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QMimeData>
#include <QRegularExpression>
#include <QKeySequence>
#include <QMouseEvent>
#include <QDesktopServices>
#include <QMenu>

#include "model/text_model.h"
#include "widgets/markdown_widget.h"

#include <QDebug>

MarkdownWidget::MarkdownWidget(QWidget *parent, Style *style)
	: QTextEdit(parent)
{
	m_style = style;

	setWordWrapMode(QTextOption::WordWrap);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setAcceptRichText(false);
	setPlaceholderText(tr("Start typing here..."));

	// Apply style.
	setStyleSheet(
		QString("border: 0px; background-color: %1; color: %2; font: %3 %4px \"%5\"")
		.arg(style->background().name(QColor::HexRgb))
		.arg(style->textEditColor().name(QColor::HexRgb))
		.arg(style->textEditFont().bold() ? "bold" : "")
		.arg(style->textEditFont().pixelSize())
		.arg(style->textEditFont().family())
	);

	// Install highlighter.
	m_highlighter = new MarkdownHighlighter(style, document());
}

MarkdownWidget::~MarkdownWidget() {
	delete m_highlighter;
}

void MarkdownWidget::load(QString data) {
	qDebug() << "++ loading ++";
	qDebug() << data;

	// Delete timer.
	if (m_saveTimer != nullptr) {
		delete m_saveTimer;
		m_saveTimer = nullptr;
	}

	// Set new model.
	QRegularExpression splitExp("(\\n|\r\\n)", QRegularExpression::MultilineOption);
	QStringList lines = data.split(splitExp);
	m_model = TextModel(lines);
	// There has to be at least one line in the document.
	if (m_model.lines()->size() == 0) {
		QString emptyString = QString();
		Line line = Line(emptyString);
		std::vector<Line> lines = {line};
		m_model.setLines(lines);
	}
	m_highlighter->setModel(&m_model);

	// Clear current text.
	disconnect(this, SIGNAL(cursorPositionChanged()), this, SLOT(onCursorMoved()));
	QTextCursor tmp = QTextCursor(document());
	tmp.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
	tmp.removeSelectedText();

	// Normal paragraph format.
	QTextBlockFormat blockFormat;
	blockFormat.setBottomMargin(ParagraphMargin);
	textCursor().setBlockFormat(blockFormat);

	// Append model to the document.
	QTextCursor cursor = append(m_model, QTextCursor(document()));

	// Reset cursor.
	m_prevBlock = cursor.block().blockNumber();
	if (m_prevBlock == 0) {
		m_prevBlock = -1;
	}
	cursor.setPosition(0);
	m_highlighter->onActiveBlockChanged(cursor.block().blockNumber());
	setTextCursor(cursor);
}

QTextCursor MarkdownWidget::append(
	TextModel model,
	QTextCursor cursor
) {
	QTextDocument *doc = document();

	//-- Default formats. --//

	// Normal paragraph format.
	QTextBlockFormat blockFormat;
	blockFormat.setBottomMargin(ParagraphMargin);
	cursor.mergeBlockFormat(blockFormat);

	// Inter-list format.
	QTextBlockFormat listFormat;
	listFormat.setBottomMargin(0);

	// Code frame format.
	QTextFrameFormat codeFormat;
	codeFormat.setBottomMargin(ParagraphMargin);
	codeFormat.setTopMargin(ParagraphMargin);
	codeFormat.setPadding(ParagraphMargin);
	codeFormat.setBackground(m_style->codeBackground());

	//-- Content building --//

	// Suspend cursor changes while we're editing.
	disconnect(this, SIGNAL(cursorPositionChanged()), this, SLOT(onCursorMoved()));

	std::vector<Line> *mlines = model.lines();
	std::vector<Line>::iterator it;

	QTextList *list = nullptr;
	QTextFrame *code = nullptr;

	bool inCode = false;
	if (auto block = cursor.block().blockNumber(); block != -1) {
		auto lines = m_model.lines();
		if (lines->size() > block && (*lines)[block].isCodeBlock) {
			code = cursor.currentFrame();
			cursor.setBlockFormat(listFormat);
			inCode = true;
		} else if (lines->size() > block && (*lines)[block].isListItem()) {
			list = cursor.currentList();
		}
	}

	// Append paragraphs.
	for (it = mlines->begin(); it != mlines->end(); it++) {
		if ((*it).isListItem()) {
			// TODO: Levels support.
			// Create a new list if needed.
			if (list == nullptr) {
				cursor.deletePreviousChar();
				list = cursor.insertList(
					(*it).list.listType == ListBullet
						? QTextListFormat::ListDisc
						: QTextListFormat::ListDecimal
				);
				cursor.mergeBlockFormat(listFormat);
			}
		} else if ((*it).isCodeBlock) {
			// Create code frame.
			if (code == nullptr) {
				cursor.deletePreviousChar();
				code = cursor.insertFrame(codeFormat);
				cursor.setBlockFormat(listFormat);
			}
		} else {
			if (list != nullptr) {
				if (it != mlines->begin() && mlines->size() > 1) {
					list = nullptr;
					cursor.setBlockFormat(blockFormat);
				}
			} else if (!inCode && code != nullptr) {
				code = nullptr;
			}
		}

		// Add paragraph's content.
		cursor.insertText((*it).folded);
		if ((it + 1) != mlines->end()) {
			// Add margin at the end of the list.
			if ((*it).isListItem() && (*(it + 1)).isListItem() == false) {
				cursor.mergeBlockFormat(blockFormat);
				cursor.insertBlock();
			// Close code frame.
			} else if (
				!inCode &&
				((*it).isCodeBlock == true && (*(it + 1)).isCodeBlock == false)
			) {
				cursor.movePosition(QTextCursor::NextBlock);
				cursor.insertBlock();
				cursor.mergeBlockFormat(blockFormat);
				cursor.deletePreviousChar();
			} else {
				cursor.insertBlock();
			}
		}
	}

	// Reconnect cursor change signal.
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(onCursorMoved()));

	// Position at the end of editing.
	return cursor;
}

void MarkdownWidget::insertFromMimeData(const QMimeData *source) {
	if (!source->hasText())
		return;

	QString text = source->text();
	if (text.isEmpty())
		return;

	// Create model from pastebin.
	QRegularExpression splitExp("(\\n|\\r\\n)", QRegularExpression::MultilineOption);
	QStringList dataLines = text.split(splitExp);
	TextModel model = TextModel(dataLines);
	std::vector<Line> *mlines = model.lines();

	// Current document config.
	QTextCursor cursor = textCursor();
	std::vector<Line> *lines = m_model.lines();
	int block = cursor.block().blockNumber();
	auto current = lines->begin() + block;
	int pos = cursor.positionInBlock();

	// Clear current selection first.
	if (
		cursor.block().blockNumber() != -1 &&
		cursor.selectionStart() != cursor.selectionEnd()
	) {
		deleteSelection(&cursor, &current, &pos);
	}

	QString original = (*current).text;
	QString prefix = original.left(pos);
	QString suffix = original.right(original.size() - pos);

	// Add lines before cursor.
	std::vector<Line> newModel;
	for (auto it = lines->begin(); it != current; it++) {
		newModel.push_back(*it);
	}

	bool isCode = (*current).isCodeBlock;

	// Add pasted lines.
	std::vector<Line>::iterator it;
	for (it = mlines->begin(); it != mlines->end(); it++) {
		if (it == mlines->begin()) {
			QString newText = prefix + (*it).text;
			// Add suffix if we're inserting a single line without paragraph brakes.
			// This case is a simple in-paragraph paste.
			if ((it + 1) == mlines->end()) {
				newText = newText + suffix;
			}

			if (isCode) {
				newModel.push_back(Line::codeLine(newText));
			} else {
				// Copy list settings from current line.
				Line newLine = Line(newText, (*current).list);
				newModel.push_back(newLine);
			}
		} else if ((it + 1) == mlines->end()) {
			QString lastText = (*it).text + suffix;
			if (isCode) {
				Line line = Line::codeLine(lastText);
				newModel.push_back(line);
			} else {
				Line line = Line(lastText);
				newModel.push_back(line);
			}
		} else {
			if (isCode) {
				Line line = Line::codeLine((*it).text);
				newModel.push_back(line);
			} else {
				newModel.push_back(*it);
			}
		}
	}

	// Add lines after cursor.
	current += 1;
	for (; current != lines->end(); current++) {
		newModel.push_back(*current);
	}

	// Save new model.
	m_model.setLines(newModel);

	// Append to document.
	QTextCursor updated = append(model, textCursor());
	// Reset prev block for cases when paste was within a single line.
	// That way we unfold the current line.
	if (mlines->size() == 1) {
		m_prevBlock = -1;
	}
	setTextCursor(updated);
	onCursorMoved();
	// Rehighlight because document configuration has changed.
	m_highlighter->rehighlight();
}

QMimeData *MarkdownWidget::createMimeDataFromSelection() const {
	QTextCursor cursor = textCursor();
	QString selection = getSelection(cursor);
	QMimeData *data = new QMimeData();
	data->setText(selection);
	return data;
}

QString MarkdownWidget::getSelection(QTextCursor cursor) const {
	const std::vector<Line> *lines = m_model.const_lines();

	if (cursor.selectionStart() == cursor.selectionEnd()) {
		return nullptr;
	}

	int startBlock = document()->findBlock(cursor.selectionStart()).blockNumber();
	int endBlock = document()->findBlock(cursor.selectionEnd()).blockNumber();
	int selStart = cursor.selectionStart();

	if (startBlock == endBlock) {
		QString text = (*(lines->begin() + startBlock)).text;
		int length = cursor.selectionEnd() - cursor.selectionStart();
		// If selection is done forward, i.e. a->bcd|ef, then start position is
		// end of selection minus length of selection.
		// If selection is done backwards, i.e. a|bcd<-e, then the start position is
		// current cursor position.
		int start = (cursor.position() > cursor.selectionStart())
			? (cursor.positionInBlock() - length)
			: (cursor.positionInBlock());

		QString selection = text.mid(start, length);
		return selection;
	} else {
		QStringList list;
		const Line *prevLine = nullptr;
		const Line *currentLine = nullptr;

		currentLine = &(*(lines->begin() + startBlock));
		if (currentLine->isCodeBlock)
			list.push_back("```");
		prevLine = currentLine;

		// For first block, push text from selection start.
		QString firstText = currentLine->text;

		// Relative position inside start block.
		int startPos = cursor.selectionStart()
			- document()->findBlock(cursor.selectionStart()).position();
		// If cursor is not on the block, we need to adjust position to
		// take into account all of the folded formatting.
		if (cursor.position() == cursor.selectionStart() && startBlock != endBlock) {
			startPos += adjustForUnfolding(
				&currentLine->folded,
				&currentLine->foldedFormats,
				startPos
			);
		}

		// Get the text from position to the end of the block.
		QString remainingFirstText = firstText.right(firstText.size() - startPos);
		// Add list prefix, if any.
		QString prefix = currentLine->list.value;
		list.push_back(prefix + remainingFirstText);

		// Push every line before last.
		for (
			auto it = lines->begin() + startBlock + 1;
			it <= (lines->begin() + endBlock);
			it++
		) {
			currentLine = &(*it);

			// Separate blocks.
			if (prevLine->isCodeBlock && !currentLine->isCodeBlock) {
				list.push_back("```");
				list.push_back("");
			} else if (!prevLine->isCodeBlock && currentLine->isCodeBlock) {
				list.push_back("");
				list.push_back("```");
			} else if (prevLine->list.listType != currentLine->list.listType) {
				list.push_back("");
			} else if (
				prevLine->list.listType == ListNone &&
				currentLine->list.listType == ListNone
			) {
				list.push_back("");
			}

			if (it == (lines->begin() + endBlock))
				break;

			QString prefix = currentLine->list.value;
			list.push_back(prefix + (*it).text);
			prevLine = currentLine;
		}

		// For last block, push text up to selection end.
		currentLine = &(*(lines->begin() + endBlock));
		// Position inside block.
		int endPos = cursor.selectionEnd()
			- document()->findBlock(cursor.selectionEnd()).position();
		// If cursor is not on the block, we need to adjust position to
		// take into account all of the folded formatting.
		if (cursor.position() == cursor.selectionEnd() && startBlock != endBlock) {
			endPos += adjustForUnfolding(
				&currentLine->folded,
				&currentLine->foldedFormats,
				endPos
			);
		}

		QString lastText = currentLine->text;
		QString startingLastText = lastText.left(endPos);
		list.push_back(currentLine->list.value + startingLastText);

		// Close code block.
		if (currentLine->isCodeBlock)
			list.push_back("```");

		QString joined = list.join("\n");
		return joined;
	}

	return nullptr;
}

bool MarkdownWidget::isDirty() const {
	return m_isDirty;
}

QString MarkdownWidget::text() const {
	// Select all text.
	QTextCursor cursor = QTextCursor(document());
	cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
	QString txt = getSelection(cursor);
	// Return all text.
	return txt;
}

Style *MarkdownWidget::style() {
	return m_style;
}

// Search.

void MarkdownWidget::showSearchWidget(QWidget *widget, QPoint belowPoint) {
	assert(widget != nullptr);
	m_search = widget;

	// Save current cursor position.
	m_menuCursor = textCursor();

	// Calculate position.
	QSize hint = widget->sizeHint();
	QSize cur = size();
	int width = std::min(cur.width(), 300);
	int x = std::max(belowPoint.x() - width / 2, 0);

	widget->setGeometry(
		x, belowPoint.y() + 5,
		width, hint.height()
	);

	widget->setParent(this);
	widget->show();
	widget->setFocus();
}

void MarkdownWidget::hideSearchWidget() {
	if (m_search == nullptr)
		return;

	m_search->deleteLater();
	m_search = nullptr;

	// Restore focus and cursor.
	setFocus();
}

void MarkdownWidget::insertNodeLink(ThoughtId id, QString title) {
	QMimeData *data = new QMimeData();
	data->setText(
		QString("[%1](node://%2)")
		.arg(title)
		.arg(id)
	);
	insertFromMimeData(data);
	delete data;

	// Schedule save.
	m_isDirty = true;
	throttleSave();

	// Rehighlight.
	m_highlighter->onActiveBlockChanged(textCursor().block().blockNumber());
	m_highlighter->rehighlight();
}

// Control events.

void MarkdownWidget::onError(MarkdownError error) {
	// TODO: Show error popup/toast.
}

// UI Events.

void MarkdownWidget::resizeEvent(QResizeEvent* event) {
	QTextEdit::resizeEvent(event);
}

void MarkdownWidget::keyPressEvent(QKeyEvent *event) {
	static QString emptyString = "";

	QTextCursor cursor = textCursor();
	std::vector<Line> *lines = m_model.lines();
	int block = cursor.block().blockNumber();
	auto current = lines->begin() + block;
	int pos = cursor.positionInBlock();
	// Flag indicating whether backspace handling block should be
	// entered when we press delete. In some cases the behavior for
	// delete and backspace is equivalent.
	bool deleteAsBackspace = false;

	// Inter-list format.
	QTextBlockFormat listFormat;
	listFormat.setBottomMargin(0);

	// Normal paragraph format.
	QTextBlockFormat blockFormat;
	blockFormat.setBottomMargin(ParagraphMargin);

	// Mark dirty. Not ideal, because we trigger save even on cursor move.
	// TODO: Wrap the event handler into a function returning a bool value
	// indicating whether there was a change in the text.
	m_isDirty = true;
	throttleSave();

	if (event->matches(QKeySequence::Cut)) {
		copy();
		deleteSelection(&cursor, &current, &pos);
		return;
	}

	if (
		cursor.block().blockNumber() != -1 &&
		isControlKey(event) &&
		cursor.selectionStart() != cursor.selectionEnd()
	) {
		deleteSelection(&cursor, &current, &pos);
		if (!isNewlineKey(event)) {
			return;
		}
	}

	if (isNewlineKey(event)) {
		if (cursor.block().blockNumber() == -1)
			return;

		QString original = (*current).text;
		QString prefix = original.left(pos);
		QString suffix = original.right(original.size() - pos);

		bool isLastCode = isLastCodeBlock(&current);
		bool isFirstCode = isFirstCodeBlock(&current);
		bool isLastList = isLastListItem(&current);
		bool isFirstList = isFirstListItem(&current);

		if (
			prefix.size() == 0 && suffix.size() == 0 &&
			(event->modifiers() & Qt::ShiftModifier) == 0
		) {
			if (isLastCode || isLastList) {
				// End code block.
				endListOrCode(&cursor, current);
				(*current).isCodeBlock = false;
				(*current).list = ListItem();
				(*current).setText(prefix);
				// No more actions needed.
				return;
			} else if (isFirstCode || isFirstList) {
				// End code block.
				deleteListOrCode(&cursor);
				(*current).isCodeBlock = false;
				(*current).list = ListItem();
				(*current).setText(prefix);
				// No more actions needed.
				return;
			}
		}

		if ((*current).isCodeBlock) {
			(*current).setText(prefix);
			lines->insert(current + 1, Line::codeLine(suffix));
			cursor.insertBlock();
			return;
		} else if ((*current).isListItem()) {
			(*current).setText(prefix);
			lines->insert(current + 1, Line(suffix, (*current).list));
			cursor.mergeBlockFormat(listFormat);
			cursor.insertBlock();

			auto next = current + 1;
			if (isLastListItem(&next))
				cursor.mergeBlockFormat(blockFormat);
			return;
		} else {
			(*current).setText(prefix);
			lines->insert(current + 1, Line(suffix));
			cursor.insertBlock();
			return;
		}
	}

	if (event->key() == Qt::Key_Delete) {
		if (cursor.atBlockEnd()) {
			if (cursor.atBlockStart()) {
				deleteAsBackspace = true;
			} else if (isLastCodeBlock(&current)) {
				if (
					isFirstCodeBlock(&current) &&
					(*current).text.isEmpty()
				) {
					// Delete code block as if we pressed backspace.
					cursor.movePosition(QTextCursor::NextCharacter);
					deleteAsBackspace = true;
				} else {
					// Don't do anything if we're at the border of the code block.
					return;
				}
			} else if (current != lines->end() && (*(current+1)).isCodeBlock) {
				// Don't do anything if we're at the border of the code block.
				return;
			} else {
				// Merge with next item.
				if ((current + 1) == lines->end()) {
					if ((*current).text.isEmpty()) {
						lines->erase(current);
						cursor.deletePreviousChar();
					}
				} else {
					QString text = (*current).text + (*(current + 1)).text;
					(*(current)).setText(text);
					lines->erase(current + 1);

					if (current != lines->begin()) {
						cursor.deleteChar();
					}

					// Restore margins if needed.
					if (isLastListItem(&current)) {
						cursor.mergeBlockFormat(blockFormat);
					}
				}

				m_highlighter->onActiveBlockChanged(cursor.block().blockNumber());
				m_highlighter->rehighlight();
				return;
			}
		}
	}

	if (event->key() == Qt::Key_Backspace || deleteAsBackspace) {
		if (cursor.atBlockStart()) {
			// If at beginning or end of list item, make it not list.
			if (isLastListItem(&current)) {
				if ((*current).text.isEmpty()) {
					endListOrCode(&cursor, current);
					(*current).list = ListItem();
				} else {
					// Merge with previous item.
					QString text = (*(current - 1)).text + (*current).text;
					(*(current - 1)).setText(text);
					lines->erase(current);

					if (current != lines->begin()) {
						cursor.deletePreviousChar();
					}

					// Set margins for end of the list.
					cursor.mergeBlockFormat(blockFormat);

					m_highlighter->onActiveBlockChanged(cursor.block().blockNumber());
					m_highlighter->rehighlight();
				}
				return;
			} else if (isFirstListItem(&current)) {
				// Remove list formatting from current block.
				(*current).list = ListItem();
				cursor.setBlockFormat(blockFormat);
				return;
			} else if (isFirstCodeBlock(&current)) {
				// Delete code block if it's empty.
				//
				// TODO: This is not ideal behavior. We should probably remove current
				// line from the code block while retaining the block itself. Instead
				// we just ignore that case, and only delete the whole code block if
				// it's empty.
				// But QTextEdit is extremely anal with editing frames, so this has to
				// do for now.
				if (isLastCodeBlock(&current) && (*current).text.isEmpty()) {
					lines->erase(current);

					disconnect(this, SIGNAL(cursorPositionChanged()), this, SLOT(onCursorMoved()));

					cursor.movePosition(QTextCursor::PreviousBlock);
					cursor.movePosition(QTextCursor::EndOfBlock);
					cursor.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor);
					cursor.removeSelectedText();
					cursor.insertBlock();
					cursor.movePosition(QTextCursor::PreviousCharacter);

					connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(onCursorMoved()));

					// Restore cursor.
					setTextCursor(cursor);
				}
				return;
			} else {
				if (current != lines->begin()) {
					if (
						!(*current).isCodeBlock &&
						(*(current - 1)).isCodeBlock &&
						(current + 1) == lines->end() || (*(current + 1)).isCodeBlock
					) {
						// TODO: Bug. Don't allow delete lines between code blocks or at the end
						// of the document after a code block.
						return;
					}

					// Merge with previous item.
					int prevLength = (*(current - 1)).text.size();
					bool isCodeBlock = (*current).isCodeBlock;
					bool isEmpty = (*current).text.isEmpty();
					QString text = (*(current - 1)).text + (*current).text;
					(*(current - 1)).setText(text);
					lines->erase(current);

					if (!isCodeBlock && (*(current - 1)).isCodeBlock) {
						cursor.beginEditBlock();
						cursor.select(QTextCursor::LineUnderCursor);
						cursor.removeSelectedText();
						cursor.deleteChar();
						cursor.movePosition(QTextCursor::PreviousBlock);
						cursor.movePosition(
							QTextCursor::NextCharacter, QTextCursor::MoveAnchor, prevLength
						);
						m_prevBlock = cursor.block().blockNumber();
						cursor.endEditBlock();
						setTextCursor(cursor);
					} else {
						disconnect(this, SIGNAL(cursorPositionChanged()), this, SLOT(onCursorMoved()));

						cursor.deletePreviousChar();
						m_prevBlock = 0;
						setTextCursor(cursor);
						onCursorMoved();

						connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(onCursorMoved()));
					}

					// Rehighlight.
					m_highlighter->onActiveBlockChanged(cursor.block().blockNumber());
					m_highlighter->rehighlight();
				}
				return;
			}
		}
	}

	if (event->key() == Qt::Key_Tab) {
		// TODO: Support list levels.
	}

	// Normal key press.
	QTextEdit::keyPressEvent(event);

	if (
		event->matches(QKeySequence::Paste) ||
		event->matches(QKeySequence::Copy) ||
		event->matches(QKeySequence::Cut)
	) {
		return;
	}

	if (
		textCursor().block() == cursor.block() &&
		// Fix to prevent removing formatting when modifier key is pressed
		// while selection is active.
		!cursor.hasSelection()
	) {
		// Update text.
		QString text = cursor.block().text();
		ListItem prevList = (*current).list;
		bool prevCode = (*current).isCodeBlock;
		(*current).setText(text);

		if (prevList.listType == ListNone && (*current).list.listType == ListBullet) {
			cursor.createList(QTextListFormat::ListDisc);
			cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor, 2);
			cursor.removeSelectedText();
		} else if (prevList.listType == ListNone && (*current).list.listType == ListNumeric) {
			cursor.createList(QTextListFormat::ListDecimal);
			cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor, 3);
			cursor.removeSelectedText();
		} else if (
			text == "```" &&
			current != lines->begin() &&
			(*(current - 1)).isCodeBlock &&
			(!(*current).isCodeBlock)
		) {
			disconnect(this, SIGNAL(cursorPositionChanged()), this, SLOT(onCursorMoved()));

			// TODO: QTextEdit is buggy. We can't insert two code blocks into the
			// document one after another, so we solve it either by merging current
			// line into previous code block, or inserting a new line into previous
			// block.
			if ((*(current + 1)).isCodeBlock || (current + 1) == lines->end()) {
				QString newEmptyString = QString();
				(*current).setText(newEmptyString);
				QString emptyString = QString();
				Line newLine = Line::codeLine(emptyString);
				lines->insert(current, newLine);

				cursor.beginEditBlock();

				// Delete current text.
				cursor.deletePreviousChar();
				cursor.deletePreviousChar();
				cursor.deletePreviousChar();

				// Append code to the previous code frame.
				cursor.movePosition(QTextCursor::PreviousBlock);
				cursor.movePosition(QTextCursor::EndOfBlock);
				cursor.insertBlock();

				cursor.endEditBlock();
			} else {
				// Clear text.
				QString emptyString = QString();
				(*current).setText(emptyString);
				(*current).isCodeBlock = true;

				cursor.beginEditBlock();

				// Delete current text.
				cursor.deletePreviousChar();
				cursor.deletePreviousChar();
				cursor.deletePreviousChar();
				// Delete current block.
				cursor.deleteChar();

				// Append code to the previous code frame.
				cursor.movePosition(QTextCursor::PreviousBlock);
				cursor.movePosition(QTextCursor::EndOfBlock);
				cursor.insertBlock();

				cursor.endEditBlock();
			}

			connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(onCursorMoved()));

			// Update cursor position.
			setTextCursor(cursor);
			onCursorMoved();
		} else if (prevCode == false && text == "```") {
			QString firstEmptyLine = QString();
			(*current) = Line::codeLine(firstEmptyLine);
			QString secondEmptyLine = QString();
			lines->insert(current + 1, Line(secondEmptyLine));

			disconnect(this, SIGNAL(cursorPositionChanged()), this, SLOT(onCursorMoved()));
			cursor.beginEditBlock();

			// Delete current text.
			cursor.deletePreviousChar();
			cursor.deletePreviousChar();
			cursor.deletePreviousChar();
			cursor.deletePreviousChar();

			// Insert code frame.
			QTextFrameFormat codeFormat;
			codeFormat.setBottomMargin(ParagraphMargin);
			codeFormat.setTopMargin(ParagraphMargin);
			codeFormat.setPadding(ParagraphMargin);
			codeFormat.setBackground(m_style->codeBackground());
			cursor.insertFrame(codeFormat);

			// Set code font.
			QTextCharFormat fmt;
			fmt.setFont(m_style->codeFont());
			cursor.mergeCharFormat(fmt);

			// Add lower margin to next block.
			cursor.movePosition(QTextCursor::NextBlock);
			cursor.mergeBlockFormat(blockFormat);

			cursor.endEditBlock();
			connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(onCursorMoved()));

			// Update cursor.
			setTextCursor(cursor);
			onCursorMoved();
		}

		// Update highlighting after we've saved to the model.
		m_highlighter->rehighlight();
	}
}

// Link handling.

void MarkdownWidget::mousePressEvent(QMouseEvent *event) {
	if (event->button() == Qt::LeftButton) {
		QString anchor;
		// Check for URL at current position. We have to do this, because
		// using setAnchor from QSyntaxHighlighter only applies the style,
		// but doesn't actually make the text a link. So we have to manually
		// search for a link under the click position and activate it...
		QTextCursor cursor = cursorForPosition(event->pos());
		std::vector<Line>* lines = m_model.lines();
		int blockNumber = cursor.block().blockNumber();
		Line &line = *(lines->begin() + blockNumber);

		int position = cursor.positionInBlock();
		QString text = line.text;
		std::vector<FormatRange> formats = line.formats;
		// If we're not on active block, use folded text instead of originial.
		if (textCursor().block().blockNumber() != blockNumber) {
			text = line.folded;
			formats = line.foldedFormats;
		}

		// Go through formats and detect links that envelop the position.
		for (auto& fmt: formats) {
			if (fmt.format != Link && fmt.format != PlainLink && fmt.format != NodeLink)
				continue;
			if (!(fmt.from < position && fmt.to > position))
				continue;
			if (fmt.link.target.isEmpty())
				continue;
			anchor = fmt.link.target;
			break;
		}

		if (!anchor.isEmpty()) {
			m_anchor = anchor;
			return;
		}
	}

	QTextEdit::mousePressEvent(event);
}

void MarkdownWidget::mouseReleaseEvent(QMouseEvent *event) {
	static QRegularExpression expr("(.+?)://(.+?)");

	if (!m_anchor.isEmpty()) {
		QRegularExpressionMatch match = expr.match(m_anchor);
		if (match.hasMatch()) {
			QString scheme = match.captured(1);
			if (scheme == "node") {
				QString value = match.captured(2);
				bool success = false;
				ThoughtId thoughtId = value.toInt(&success);
				if (success) {
					emit nodeLinkSelected(thoughtId);
				}
			} else {
				QDesktopServices::openUrl(QUrl(m_anchor));
			}
		}

		m_anchor = "";
		return;
	}

	QTextEdit::mouseReleaseEvent(event);
}

void MarkdownWidget::contextMenuEvent(QContextMenuEvent *event) {
	QMenu *menu = createStandardContextMenu();

	// Separator before the first action.
	QAction *separator = menu->insertSeparator(menu->actions().first());

	// Custom node link action before separator.
	QString connectMenu = tr("Connect thought...");
	QAction *action = new QAction(connectMenu, this);
	connect(action, SIGNAL(triggered()), this, SLOT(onInsertNodeLink()));
	menu->insertAction(separator, action);

	// Show the menu.
	menu->exec(mapToGlobal(event->pos()));
}

// Linking nodes.

void MarkdownWidget::onInsertNodeLink() {
	QRect rect = cursorRect();
	emit nodeInsertionActivated(rect.bottomLeft());
}

// Cursor.

void MarkdownWidget::onCursorMoved() {
	std::vector<Line> *lines = m_model.lines();
	QTextCursor cursor = textCursor();
	int posInBlock = cursor.positionInBlock();

	if (
		m_prevBlock != -1 &&
		m_prevBlock == cursor.block().blockNumber()
	) {
		return;
	}

	QTextBlock block = cursor.block();
	int number = block.blockNumber();

	if (
		number >= 0 &&
		m_prevBlock >= 0 &&
		cursor.hasSelection() &&
		(*lines)[m_prevBlock].isCodeBlock == (*lines)[number].isCodeBlock
	) {
		return;
	}

	m_highlighter->onActiveBlockChanged(number);

	// Suspend cursor changes while we're editing.
	disconnect(this, SIGNAL(cursorPositionChanged()), this, SLOT(onCursorMoved()));

	// Change previous block.
	if (m_prevBlock != -1) {
		QTextBlock prevBlock = document()->findBlockByNumber(m_prevBlock);
		Line prev = (*m_model.lines())[m_prevBlock];
		formatBlock(prevBlock, &prev.folded, &prev.foldedFormats);
	}

	// Save prev cursor.
	m_prevBlock = cursor.block().blockNumber();

	// Check if we're not outside of the bounds.
	if (number >= (*m_model.lines()).size())
		return;

	// Update current block.
	if (number >= 0) {
		Line line = (*m_model.lines())[number];
		formatBlock(cursor.block(), &line.text, &line.formats);
		posInBlock += adjustForUnfolding(&line.folded, &line.foldedFormats, posInBlock);
	}

	// Restore cursor.
	QTextCursor newCursor = QTextCursor(cursor.block());
	newCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, posInBlock);
	if (newCursor.position() >= 0) {
		this->setTextCursor(newCursor);
	}

	// Resume cursor updates.
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(onCursorMoved()));
}

int MarkdownWidget::adjustForUnfolding(
	const QString *text,
	const std::vector<FormatRange> *ranges,
	int positionInBlock
) const {
	assert(text != nullptr);
	assert(ranges != nullptr);

	int offset = 0;
	for (auto& range: *ranges) {
		if (range.to < positionInBlock) {
			offset += range.endOffset();
		}
		if (range.from < positionInBlock) {
			offset += range.startOffset();
		}
	}

	return offset;
}

void MarkdownWidget::formatBlock(
	QTextBlock block,
	QString *text,
	std::vector<FormatRange> *formats
) {
	assert(text != nullptr);
	assert(formats != nullptr);

	QTextCursor edit(block);
	int size = text->size();

	edit.beginEditBlock();
	edit.select(QTextCursor::LineUnderCursor);
	edit.insertText(*text, QTextCharFormat());
	edit.endEditBlock();
}

bool MarkdownWidget::isControlKey(QKeyEvent* event) {
	return (
		event->key() == Qt::Key_Return ||
		event->key() == Qt::Key_Enter ||
		event->key() == Qt::Key_Delete ||
		event->key() == Qt::Key_Backspace
	);
}

bool MarkdownWidget::isNewlineKey(QKeyEvent* event) {
	return (
		event->key() == Qt::Key_Return ||
		event->key() == Qt::Key_Enter
	);
}

void MarkdownWidget::deleteSelection(
	QTextCursor *cursor,
	std::vector<Line>::iterator *current,
	int *pos
) {
	std::vector<Line> *lines = m_model.lines();

	if (cursor->selectionStart() == cursor->selectionEnd()) {
		return;
	}

	int startBlock = document()->findBlock(cursor->selectionStart()).blockNumber();
	int endBlock = document()->findBlock(cursor->selectionEnd()).blockNumber();
	int selStart = cursor->selectionStart();

	// Delete selected text in first and last blocks.
	if (startBlock != endBlock) {
		// For last block, delete from start of the block to the end of
		// selection.

		Line *lastLine = &(*(lines->begin() + endBlock));
		// Position inside last block.
		int endPos = cursor->selectionEnd()
			- document()->findBlock(cursor->selectionEnd()).position();
		// If selection starts not on the last block, we need to
		// take into account all of the folded formatting.
		if (cursor->position() == cursor->selectionEnd()) {
			endPos += adjustForUnfolding(
				&lastLine->folded,
				&lastLine->foldedFormats,
				endPos
			);
		}

		QString lastText = lastLine->text;
		QString remainingLastText = lastText.right(lastText.size() - endPos);
		lastLine->setText(remainingLastText);

		// For first block, delete from start of the selection to the end of
		// the block.

		Line *firstLine = &(*(lines->begin() + startBlock));
		// Relative position inside start block.
		int startPos = cursor->selectionStart()
			- document()->findBlock(cursor->selectionStart()).position();
		// If selection ends not on the first block, we need to
		// take into account all of the folded formatting.
		if (cursor->position() == cursor->selectionStart()) {
			startPos += adjustForUnfolding(
				&firstLine->folded,
				&firstLine->foldedFormats,
				startPos
			);
		}

		QString firstText = firstLine->text;
		QString remainingFirstText = firstText.left(startPos);
		firstLine->setText(remainingFirstText);

		// Delete all lines between start and end of selection.
		if (endBlock > startBlock + 1) {
			lines->erase(lines->begin() + startBlock + 1, lines->begin() + endBlock);
		}

		// Merge first and last block, delete the last one.
		QString mergedText = remainingFirstText + remainingLastText;
		lines->erase(lines->begin() + startBlock + 1);
		(*(lines->begin() + startBlock)).setText(mergedText);

		// Delete text from the document.
		cursor->removeSelectedText();
		// Set cursor position to beginning of the selection.
		cursor->setPosition(selStart);
		*pos = cursor->positionInBlock();
		*current = lines->begin() + startBlock;
	} else {
		QString text = (*(lines->begin() + startBlock)).text;
		int length = cursor->selectionEnd() - cursor->selectionStart();
		// If selection is done forward, i.e. a->bcd|ef, then start position is
		// end of selection minus length of selection.
		// If selection is done backwards, i.e. a|bcd<-e, then the start position is
		// current cursor position.
		int start = (cursor->position() > cursor->selectionStart())
			? (cursor->positionInBlock() - length)
			: (cursor->positionInBlock());
		text.remove(start, length);
		*(*current) = Line(text);
		// Delete text from the document.
		cursor->removeSelectedText();
		// Set cursor position to beginning of the selection.
		cursor->setPosition(selStart);
		*pos = cursor->positionInBlock();
	}

	// Reset format if we're at the end of a list.
	if (isLastListItem(current)) {
		QTextBlockFormat blockFormat;
		blockFormat.setBottomMargin(ParagraphMargin);
		cursor->mergeBlockFormat(blockFormat);
	}
}

void MarkdownWidget::endListOrCode(QTextCursor *cursor, std::vector<Line>::iterator current) {
	// Normal paragraph format.
	QTextBlockFormat blockFormat;
	blockFormat.setBottomMargin(ParagraphMargin);

	// Suspend cursor changes while we're editing.
	disconnect(this, SIGNAL(cursorPositionChanged()), this, SLOT(onCursorMoved()));

	// Set block margin for the previous block.
	if (isLastListItem(&current)) {
		cursor->movePosition(QTextCursor::PreviousBlock);
		cursor->mergeBlockFormat(blockFormat);
		cursor->movePosition(QTextCursor::NextBlock);
	}
	// Delete code block from document, create a normal paragraph.
	cursor->beginEditBlock();
	cursor->deletePreviousChar();
	if ((*current).isCodeBlock) {
		// To end a code block, we have to go outside of code frame and
		// insert new block at that position.
		cursor->movePosition(QTextCursor::NextBlock);
		cursor->insertBlock();
		cursor->setBlockFormat(blockFormat);
		cursor->movePosition(QTextCursor::PreviousBlock);
	} else {
		// To end a list, we insert non-formatted block right at the end of it
		cursor->movePosition(QTextCursor::EndOfBlock);
		cursor->insertBlock();
		cursor->setBlockFormat(blockFormat);
	}
	cursor->endEditBlock();
	// Resume cursor updates.
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(onCursorMoved()));
	setTextCursor(*cursor);
}

void MarkdownWidget::deleteListOrCode(QTextCursor *cursor) {
	// Normal paragraph format.
	QTextBlockFormat blockFormat;
	blockFormat.setBottomMargin(ParagraphMargin);

	// Suspend cursor changes while we're editing.
	disconnect(this, SIGNAL(cursorPositionChanged()), this, SLOT(onCursorMoved()));
	// Delete code block from document, create a normal paragraph.
	cursor->beginEditBlock();
	cursor->deleteChar();
	cursor->movePosition(QTextCursor::PreviousBlock);
	cursor->movePosition(QTextCursor::EndOfBlock);
	cursor->insertBlock();
	cursor->mergeBlockFormat(blockFormat);
	cursor->endEditBlock();
	// Resume cursor updates.
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(onCursorMoved()));
	setTextCursor(*cursor);
}

bool MarkdownWidget::isLastListItem(std::vector<Line>::iterator *current) {
	std::vector<Line> *lines = m_model.lines();

	return (
		(*(*current)).list.listType != ListNone &&
		(
			(*current + 1) == lines->end() ||
			!((*(*current + 1)).list.listType == (*(*current)).list.listType)
		)
	);
}

bool MarkdownWidget::isFirstListItem(std::vector<Line>::iterator *current) {
	std::vector<Line> *lines = m_model.lines();

	return (
		(*(*current)).isListItem() &&
		(
			(*current) == lines->begin() ||
			!((*(*current - 1)).list.listType == (*(*current)).list.listType)
		)
	);
}

bool MarkdownWidget::isLastCodeBlock(std::vector<Line>::iterator *current) {
	std::vector<Line> *lines = m_model.lines();

	return (
		(*(*current)).isCodeBlock &&
		((*current + 1) == lines->end() || !(*(*current + 1)).isCodeBlock)
	);
}

bool MarkdownWidget::isFirstCodeBlock(std::vector<Line>::iterator *current) {
	std::vector<Line> *lines = m_model.lines();

	return (
		(*(*current)).isCodeBlock &&
		((*current) == lines->begin() || !(*(*current - 1)).isCodeBlock)
	);
}

// Highlighter logic.

MarkdownHighlighter::MarkdownHighlighter(Style *style, QTextDocument *doc)
	: QSyntaxHighlighter(doc), m_style(style) {}

void MarkdownHighlighter::setModel(TextModel *model) {
	m_model = model;
}

void MarkdownHighlighter::onActiveBlockChanged(int number) {
	m_activeBlock = number;
}

void MarkdownHighlighter::highlightBlock(const QString &text) {
	if (m_model == nullptr)
		return;

	QTextBlock block = currentBlock();
	int number = block.blockNumber();
	if (number == -1)
		return;

	std::vector<Line> *lines = m_model->lines();
	if (lines->size() <= number) {
		return;
	}

	Line line = (*lines)[number];

	std::vector<FormatRange> ranges = (m_activeBlock == number)
		? line.formats
		: line.foldedFormats;

	for (auto fmt: ranges) {
		setFormat(
			fmt.from,
			fmt.to - fmt.from,
			fmt.qtFormat(m_style, format(fmt.from))
		);
	}
}

// Saving logic.

void MarkdownWidget::throttleSave() {
	if (!m_isDirty)
		return;

	if (m_saveTimer != nullptr)
		return;

	m_saveTimer = new QTimer(this);
	QObject::connect(
		m_saveTimer, SIGNAL(timeout()),
		this, SLOT(saveText())
	);
	m_saveTimer->setSingleShot(true);
	m_saveTimer->start(15000);
}

void MarkdownWidget::saveText() {
	if (!m_isDirty)
		return;

	// Select all text.
	QTextCursor cursor = QTextCursor(document());
	cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
	QString text = getSelection(cursor);

	// Emit text change event.
	emit textChanged(text);

	// Reset dirty flag.
	m_isDirty = false;
	// Delete timer.
	delete m_saveTimer;
	m_saveTimer = nullptr;
}

