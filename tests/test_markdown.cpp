#include <iostream>

#include <QApplication>
#include <QString>
#include <QFile>
#include <QIODevice>

#include "model/text_model.h"
#include "widgets/markdown_widget.h"

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

	MarkdownWidget widget(nullptr, &style);
	widget.resize(600, 600);

	// Show window.
	widget.show();

	// Set text.
	widget.load(string);

	// Run app.
	return app.exec();
}
