#include <QApplication>
#include <QObject>
#include <QColor>

#include "widgets/widgets.h"
#include "layout/default_layout.h"

Thought *makeThought(
	std::string name,
	std::unordered_map<ThoughtId, Thought *>* map,
	std::vector<ThoughtId>& list
) {
	static unsigned long int id = 1;

	Thought *thought = new Thought(id, name, false, false, false);
	map->insert({thought->id(), thought});
	list.push_back(thought->id());

	id += 1;
	return thought;
}

int main(int argc, char *argv[]) {
	Style& style = Style::defaultStyle();

	QApplication app(argc, argv);

	DefaultLayout layout(&style);
	BaseCanvasWidget widget(nullptr, &style, &layout);
	widget.resize(800, 800);

	widget.show();

	// Add state.
	Thought *central = new Thought(
		0, "Lorem ipsum dolor sit amet. 39一くめ第泊セ作研び環携でごばひ年自メ載1相ルシコナ選北アキナサ償全ム茨岡ルイフサ思詐手あょた。", true, false, true);
	std::unordered_map<ThoughtId, Thought *>* map = new std::unordered_map<ThoughtId, Thought *>();

	// Link items.
	makeThought("Left one", map, central->links());
	makeThought("Left two", map, central->links());
	makeThought("Left three", map, central->links());
	makeThought("Left four", map, central->links());
	makeThought("Left five", map, central->links());
	makeThought("Left six", map, central->links());
	makeThought("Abracadabra", map, central->links());
	makeThought("Lorem ipsum again and again", map, central->links());
	makeThought("Eight", map, central->links());
	makeThought("Nine", map, central->links());
	makeThought("Ten", map, central->links());
	makeThought("Eleven", map, central->links());

	// Parent items.
	Thought *parentOne = makeThought("Parent one", map, central->parents());
	makeThought("Parent two", map, central->parents());
	makeThought("Parent three", map, central->parents());
	Thought *parentTwo = makeThought("Parent two with a very long long long name", map, central->parents());

	// Child items.
	makeThought("First child", map, central->children());
	makeThought("Second jj child", map, central->children());

	// Sibling items.
	makeThought("Sibling One", map, parentOne->children());
	makeThought("Sibling Two", map, parentOne->children());
	makeThought("Sibling Three And A Half", map, parentTwo->children());

	// Composed state.
	State state(central, map);

	// Assign state.
	widget.setState(&state);

	return app.exec();
}

