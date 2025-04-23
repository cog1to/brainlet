#include <iostream>
#include <vector>

#include <QApplication>
#include <QString>
#include <QIODevice>

#include "entity/folder_brains_repository.h"
#include "widgets/brain_list_widget.h"
#include "presenters/brain_list_presenter.h"

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);

	Style& style = Style::defaultStyle();

	// Make repo.
	FolderBrainsRepository repo = FolderBrainsRepository("readonly");

	BrainListWidget widget = BrainListWidget(nullptr, &style);
	BrainListPresenter presenter = BrainListPresenter(&widget, &repo);

	widget.setStyleSheet(
		QString("background-color: %1")
		.arg(style.background().name(QColor::HexArgb))
	);
	widget.resize(600, 600);
	widget.show();

	return app.exec();
}

