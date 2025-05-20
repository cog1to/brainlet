#include <QApplication>
#include <QVBoxLayout>
#include <QList>
#include <QWidget>

#include "model/text_model.h"
#include "widgets/base_widget.h"
#include "widgets/markdown_block_widget.h"

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);

	Style& style = Style::defaultStyle();

	BaseWidget window = BaseWidget(nullptr, &style);
	QVBoxLayout layout = QVBoxLayout(&window);

	QStringList input = QStringList {
		"abracadabra",
		"second line",
		"",
		"Hahaha **bold** [Link](https://www.google.com/) and some *italic*. Lorem ipsum dolor sit amet ***bold and italic***. aslkfhjasdfh awlih aliweuhf iluawhef iuaweh fiulwaeh ilfhewailuf haweiluf haweilh ewailfh awielfh ilawehf iluawehf ilawehf ihawefilu haweilf haweif hlawehf l",
		"",
		"- item 1",
		"- item 2 adklfjadsfadslkhf aio34h t9u3w4htiuq3h iu3h iuh iua hieuar guh ila ilah iltha43a ilghailh 4iual3h4iut q3 ",
		"- item 3",
		"",
		"1. num 1",
		"1. num 2",
		"",
		"",
		"third",
		"",
		"```",
		"code 1",
		"code 2",
		"```",
		""
	};

	text::TextModel model = text::TextModel(input);
	QList<text::Paragraph> *list = model.paragraphs();

	for (qsizetype i = 0; i < list->size(); ++i) {
		text::Paragraph *p = &((*list)[i]);
		qDebug() << "item type:" << p->getType();

		QList<text::Line> *lines = p->getLines();
		for (qsizetype j = 0; j < lines->size(); j++) {
			qDebug() << "line:" << lines->at(j).text;
		}

		MarkdownBlock *block = new MarkdownBlock(nullptr, &style, nullptr);
		block->setStyleSheet("background-color: #0b0b0b");
		block->setParagraph(p);

		layout.addWidget(block);
	}

	layout.addStretch();

	window.show();
	return app.exec();
}

