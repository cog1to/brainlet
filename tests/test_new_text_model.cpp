#include <QString>
#include <QStringList>
#include <QList>
#include <QDebug>

#include "model/new_text_model.h"

int main(int argc, char *argv[]) {
	QStringList input = QStringList {
		"abracadabra",
		"second line",
		"",
		"dabrabodabra",
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
		text::Paragraph item = list->at(i);
		qDebug() << "item type:" << item.getType();

		QList<text::Line> *lines = item.getLines();
		for (qsizetype j = 0; j < lines->size(); j++) {
			qDebug() << "line:" << lines->at(j).text;
		}
	}

	return 0;
}

