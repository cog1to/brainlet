#include <QPlainTextEdit>
#include <QTextEdit>
#include <QTextBlock>
#include <QString>
#include <QStringList>
#include <QTimer>

#include "widgets/markdown_widget.h"

#include <QDebug>

MarkdownWidget::MarkdownWidget(QWidget *parent, Style *style)
	: QTextEdit(parent)
{
	m_style = style;

	setWordWrapMode(QTextOption::WordWrap);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	// Apply style.
	setStyleSheet(
		QString("padding: 5px; background-color: %1; color: %2; font: %3 %4px \"%5\"")
		.arg(style->background().name(QColor::HexRgb))
		.arg(style->textEditColor().name(QColor::HexRgb))
		.arg(style->textEditFont().bold() ? "bold" : "")
		.arg(style->textEditFont().pixelSize())
		.arg(style->textEditFont().family())
	);

	// Install highlighter.
	m_highlighter = new MarkdownHighlighter(style, document());

	// Cursor movement.
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(onCursorMoved()));
}

MarkdownWidget::~MarkdownWidget() {
	delete m_highlighter;
}

void MarkdownWidget::load(QString data) {
	QStringList lines = data.split("\n");
	m_model = TextModel(lines);
	m_highlighter->setModel(&m_model);

	QTextDocument *doc = document();
	QTextCursor cursor(doc);

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

	// Append paragraphs.
	QTextList *list = nullptr;
	QTextFrame *code = nullptr;
	std::vector<Line> *mlines = m_model.lines();
	std::vector<Line>::iterator it;
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
				list = nullptr;
				cursor.setBlockFormat(blockFormat);
			} else if (code != nullptr) {
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
			} else if (((*it).isCodeBlock == true) && (*(it + 1)).isCodeBlock == false) {
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

	// Reset cursor.
	m_prevBlock = cursor.block().blockNumber();
	cursor.setPosition(0);
	setTextCursor(cursor);
}

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
			if (isLastCodeBlock(&current)) {
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
				deleteListOrCode(&cursor);
				(*current).list = ListItem();
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

					// Suspend cursor changes while we're editing.
					disconnect(this, SIGNAL(cursorPositionChanged()), this, SLOT(onCursorMoved()));

					cursor.movePosition(QTextCursor::PreviousBlock);
					cursor.movePosition(QTextCursor::EndOfBlock);
					cursor.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor);
					cursor.removeSelectedText();
					cursor.insertBlock();
					cursor.movePosition(QTextCursor::PreviousCharacter);

					// Suspend cursor changes while we're editing.
					connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(onCursorMoved()));

					// Restore cursor.
					setTextCursor(cursor);
				}
				return;
			} else {
				if (current != lines->begin()) {
					// Merge with previous item.
					int prevLength = (*(current - 1)).text.size();
					bool isCodeBlock = (*current).isCodeBlock;
					QString text = (*(current - 1)).text + (*current).text;
					(*(current - 1)).setText(text);
					lines->erase(current);

					if (!isCodeBlock && (*(current - 1)).isCodeBlock) {
						cursor.beginEditBlock();
						cursor.select(QTextCursor::LineUnderCursor);
						cursor.removeSelectedText();
						cursor.deleteChar();
						cursor.movePosition(QTextCursor::PreviousBlock);
						cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, prevLength);
						cursor.endEditBlock();
						setTextCursor(cursor);
					} else {
						cursor.deletePreviousChar();
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

	if (textCursor().block() == cursor.block()) {
		// Update text.
		QString text = cursor.block().text();
		// TODO: List and code block creation.
		// Something like:
		//   if text == "```" {
		//     createCodeBlock()
		//   } else if text == "[-+*] " {
		//     createList()
		//   } else if text == "[0-9]+. " {
		//		 createEnumeratedList()
		//   }
		(*current).setText(text);
		// Update highlighting after we've saved to the model.
		m_highlighter->rehighlight();
	}
}

// Cursor.

void MarkdownWidget::onCursorMoved() {
	std::vector<Line> *lines = m_model.lines();
	QTextCursor cursor = textCursor();
	int position = cursor.position();
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
		// Adjust original cursor position to accout for folding.
		if (number > prevBlock.blockNumber())
			position -= prev.text.size() - prev.folded.size();
	}

	// Save prev cursor.
	m_prevBlock = cursor.block().blockNumber();

	// Update current block.
	if (number >= 0) {
		Line line = (*m_model.lines())[number];
		formatBlock(cursor.block(), &line.text, &line.formats);
		position += adjustForUnfolding(&line.folded, &line.foldedFormats, posInBlock);
	}

	// Restore cursor.
	QTextCursor newCursor = QTextCursor(this->document());
	newCursor.setPosition(position);
	if (newCursor.position() >= 0) {
		this->setTextCursor(newCursor);
	}

	// Resume cursor updates.
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(onCursorMoved()));
}

int MarkdownWidget::adjustForUnfolding(
	QString *text,
	std::vector<FormatRange> *ranges,
	int positionInBlock
) {
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
		// For last block, delete from start of the block to the end of selection.
		QString lastText = (*(lines->begin() + endBlock)).text;
		QString remainingLastText = lastText.right(
			lastText.size() - 
				(cursor->selectionEnd() - document()->findBlock(cursor->selectionEnd()).position())
		);
		(*(lines->begin() + endBlock)).setText(remainingLastText);
		
		// For first block, delete from start of the selection to the end of the block.
		QString firstText = (*(lines->begin() + startBlock)).text;
		QString remainingFirstText = firstText.left(
			cursor->selectionStart() - document()->findBlock(cursor->selectionStart()).position()
		);
		(*(lines->begin() + startBlock)).setText(remainingFirstText);
		
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
		int start = cursor->positionInBlock() - length;
		text.remove(start, length);
		(*(*current)).setText(text);
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

	return ((*(*current)).list.listType != ListNone && 
		((*current + 1) == lines->end() || !((*(*current + 1)).list.listType == (*(*current)).list.listType))
	);
}

bool MarkdownWidget::isFirstListItem(std::vector<Line>::iterator *current) {
	std::vector<Line> *lines = m_model.lines();

	return ((*(*current)).isListItem() && 
		((*current) == lines->begin() || !((*(*current - 1)).list.listType == (*(*current)).list.listType))
	);
}

bool MarkdownWidget::isLastCodeBlock(std::vector<Line>::iterator *current) {
	std::vector<Line> *lines = m_model.lines();

	return ((*(*current)).isCodeBlock && 
		((*current + 1) == lines->end() || !(*(*current + 1)).isCodeBlock)
	);
}

bool MarkdownWidget::isFirstCodeBlock(std::vector<Line>::iterator *current) {
	std::vector<Line> *lines = m_model.lines();

	return ((*(*current)).isCodeBlock && 
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

	std::vector<FormatRange> ranges = (m_activeBlock == number) ? line.formats : line.foldedFormats;

	for (auto fmt: ranges) {
		setFormat(
			fmt.from,
			fmt.to - fmt.from,
			fmt.qtFormat(m_style, format(fmt.from))
		);
	}
}

