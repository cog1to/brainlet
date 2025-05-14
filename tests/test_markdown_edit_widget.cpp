#include <iostream>

#include <QApplication>
#include <QWidget>
#include <QString>
#include <QFile>
#include <QIODevice>

#include "widgets/markdown_edit_widget.h"

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);

	Style& style = Style::defaultStyle();

	if (argc < 2) {
		std::cout << "Provide a file to render\n";
		return 1;
	}
	
	// Load file.
	QFile file(argv[1]);
	if (!file.open(QIODevice::ReadOnly)) {
		std::cout << "Failed to open a file\n";
		return 1;
	}

	QByteArray content = file.readAll();
	QString string = QString::fromUtf8(content);
	file.close();

	// Prepare model.
	QStringList lines = string.split("\n");

	MarkdownEditWidget *widget = new MarkdownEditWidget(nullptr, &style);
	widget->load(string);

	// Show window.
	QScrollArea window(nullptr);
	window.setWidget(widget);
	window.setWidgetResizable(true);
	window.show();

	// Set focus.

	// Run app.
	return app.exec();
}

