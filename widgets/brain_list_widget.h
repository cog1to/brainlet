#ifndef H_CATALOGUE_WIDGET
#define H_CATALOGUE_WIDGET

#include <unordered_map>
#include <vector>
#include <string>

#include <QWidget>
#include <QScrollArea>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QShowEvent>

#include "widgets/base_widget.h"
#include "widgets/style.h"
#include "widgets/brain_item_widget.h"
#include "model/brain.h"

class BrainListWidget: public BaseWidget {
	Q_OBJECT

public:
	BrainListWidget(QWidget*, Style*);
	// Model.
	void setItems(std::vector<Brain>);
	void showError(QString);

protected:
	void resizeEvent(QResizeEvent*) override;
	void showEvent(QShowEvent*) override;

signals:
	void shown();
	void itemClicked(std::string);
	void itemDeleteClicked(std::string);
	void newItemCreated(std::string);

private slots:
	void onItemClicked(BrainItemWidget*);
	void onItemDeleteClicked(BrainItemWidget*);
	void onNewItemClicked();

private:
	std::unordered_map<std::string, BrainItemWidget*> m_widgets;
	QScrollArea m_area;
	QWidget m_container;
	QVBoxLayout m_layout;
	// Helpers.
	bool findInItems(const std::vector<Brain>&, std::string);
};

#endif

