#include <QApplication>
#include <QObject>
#include <QColor>

#include "widgets/widgets.h"
#include "layout/default_layout.h"

int main(int argc, char *argv[]) {
	Style& style = Style::defaultStyle();

	QApplication app(argc, argv);

	DefaultLayout layout(&style);
	BaseCanvasWidget widget(nullptr, &style, &layout);
	widget.resize(800, 800);

	widget.show();

	// Add state.
	Thought *central = new Thought(0, "Lorem ipsum dolor sit amet. 39一くめ第泊セ作研び環携でごばひ年自メ載1相ルシコナ選北アキナサ償全ム茨岡ルイフサ思詐手あょた。", true, false, true);
	std::unordered_map<ThoughtId, Thought *>* map = new std::unordered_map<ThoughtId, Thought *>();

	// Link items.
	Thought *leftOne = new Thought(1, "Left two", false, false, false);
	map->insert({leftOne->id(), leftOne});
	central->links().push_back(leftOne->id());
	Thought *leftTwo = new Thought(2, "Left one with a very long name that doesn't fit in a single line", false, false, false);
	map->insert({leftTwo->id(), leftTwo});
	central->links().push_back(leftTwo->id());
	Thought *leftThree = new Thought(5, "Layout test widget #005", false, false, false);
	map->insert({leftThree->id(), leftThree});
	central->links().push_back(leftThree->id());

	// Parent items.
	Thought *parentOne = new Thought(3, "Parent one", false, true, false);
	map->insert({parentOne->id(), parentOne});
	central->parents().push_back(parentOne->id());
	Thought *parentTwo = new Thought(4, "Some other parent node", false, true, false);
	map->insert({parentTwo->id(), parentTwo});
	central->parents().push_back(parentTwo->id());

	// Composed state.
	State state(central, map);

	// Assign state.
	widget.setState(&state);

	return app.exec();
}

