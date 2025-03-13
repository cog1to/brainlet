#include <QApplication>
#include <QObject>
#include <QColor>
#include <QDebug>

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
	CanvasWidget widget(nullptr, &style, &layout);
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
	Thought *leftFour = makeThought("Left four", map, central->links());
	makeThought("Left five", map, central->links());
	makeThought("Left six", map, central->links());
	makeThought("Abracadabra", map, central->links());
	makeThought("Lorem ipsum again and again", map, central->links());
	makeThought("Eight", map, central->links());
	makeThought("Nine", map, central->links());
	Thought *ten = makeThought("Ten", map, central->links());
	makeThought("Eleven", map, central->links());

	// Parent items.
	Thought *parentOne = makeThought("Parent one", map, central->parents());
	makeThought("Parent two", map, central->parents());
	Thought *parentThree = makeThought("Parent three", map, central->parents());
	Thought *parentTwo = makeThought("Parent two with a very long long long name", map, central->parents());

	// Child items.
	makeThought("First child", map, central->children());
	makeThought("Second jj child", map, central->children());

	// Sibling items.
	makeThought("Sibling One", map, parentOne->children());
	Thought *siblingTwo = makeThought("Sibling Two", map, parentOne->children());
	makeThought("Sibling Three And A Half", map, parentTwo->children());

	// Link from link to parent.
	ten->links().push_back(parentOne->id());
	// Child from link to parent.
	ten->children().push_back(parentThree->id());
	// Link from sibling to parent.
	siblingTwo->links().push_back(parentTwo->id());
	// Child link from parent to link.
	parentThree->children().push_back(leftFour->id());

	// Composed state.
	State state(0, central, map);

	// Assign state.
	layout.setState(&state);

	// Editing callbacks.
	QObject::connect(
		&widget, &CanvasWidget::textChanged,
		[&layout, central](ThoughtId id, QString text, std::function<void(bool)> callback){
			if (text.isEmpty()) {
				callback(false);
			}	else {
				callback(true);
				central->name() = text.toStdString();
				layout.reload();
			}
		}
	);

	return app.exec();
}

