#ifndef H_CATALOGUE_WIDGET
#define H_CATALOGUE_WIDGET

#include <QWidget>

#include "widgets/base_widget.h"
#include "widgets/style.h"
#include "widgets/brain_item_widget.h"

class CatalogueWidget: public BaseWidget {
	Q_OBJECT

public:
	CatalogueWidget(QWidget*, Style*);

signals:
	void shown();

private:
	std::vector<BrainItemWidget*> m_widgets;
};

#endif

