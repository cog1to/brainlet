#include <iostream>
#include <vector>

#include <QApplication>
#include <QString>
#include <QIODevice>

#include "model/thought.h"
#include "entity/thought_entity.h"
#include "entity/connection_entity.h"
#include "entity/memory_repository.h"
#include "widgets/brain_list_widget.h"
#include "presenters/brain_list_presenter.h"

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);

	Style& style = Style::defaultStyle();

	// Make repo.
	std::vector<ThoughtEntity> thoughts;
	std::vector<ConnectionEntity> conns;
	MemoryRepository repo = MemoryRepository(thoughts, conns, 0);

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

