#ifndef H_BASE_LAYOUT
#define H_BASE_LAYOUT

#include <unordered_map>

#include <QSize>

#include "model/state.h"
#include "model/thought.h"
#include "layout/item_layout.h"
#include "layout/scroll_area_layout.h"
#include "layout/item_connection.h"
#include "widgets/style.h"
#include "widgets/thought_widget.h"

class BaseLayout {
public:
	BaseLayout(Style*);
	// State manipulation.
	virtual void setState(State*);
	virtual void setSize(QSize);
	virtual void setStyle(Style*);
	virtual void reload() = 0;
	// Layout model.
	virtual const std::unordered_map<ThoughtId, ItemLayout>* items() const = 0;
	virtual const std::unordered_map<unsigned int, ScrollAreaLayout>* scrollAreas() const = 0;
	virtual const std::vector<ItemConnection>* connections() const = 0;

public slots:
	virtual void onScroll(unsigned int, int);

protected:
	QSize m_size;
	State *m_state = nullptr;
	Style *m_style = nullptr;
	ThoughtWidget m_template;
};

#endif

