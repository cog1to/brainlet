#include <QWidget>
#include <QResizeEvent>
#include <QVBoxLayout>

#include "widgets/brain_list_widget.h"
#include "widgets/brain_item_widget.h"

BrainListWidget::BrainListWidget(QWidget *parent, Style *style)
	: BaseWidget(parent, style),
	m_area(this),
	m_container(nullptr),
	m_layout(&m_container)
{
	m_area.setWidget(&m_container);
	m_area.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_area.setStyleSheet(QString("border: none"));
	m_container.show();
	m_layout.setContentsMargins(0, 0, 20, 0);
	m_layout.setSpacing(20);

	// Add new brain button.
	QPushButton *newItemButton = new QPushButton(
		tr("Create a new Brain"),
		nullptr
	);
	newItemButton->setStyleSheet(
		style->brainListButtonStyle("center", style->hoverBorderColor())
	);
	connect(
		newItemButton, &QPushButton::clicked,
		this, &BrainListWidget::onNewItemClicked
	);
	m_layout.addWidget(newItemButton);
}

void BrainListWidget::resizeEvent(QResizeEvent *event) {
	QSize size = event->size();

	m_area.setGeometry(0, 0, size.width(), size.height());

	QSize hint = m_container.sizeHint();
	m_container.resize(size.width() - 20, hint.height());
}

// Model.

void BrainListWidget::setItems(std::vector<Brain> items) {
	// Create new widgets.
	for (auto it = items.begin(); it != items.end(); it++) {
		BrainItemWidget *widget = nullptr;
		if (auto found = m_widgets.find((*it).id()); found != m_widgets.end()) {
			widget = found->second;
		} else {
			widget = new BrainItemWidget(
				nullptr, m_style,
				QString::fromStdString((*it).id()),
				QString::fromStdString((*it).name())
			);

			connect(
				widget, &BrainItemWidget::deleteClicked,
				this, &BrainListWidget::onItemDeleteClicked
			);
			connect(
				widget, &BrainItemWidget::buttonClicked,
				this, &BrainListWidget::onItemClicked
			);

			m_widgets.insert_or_assign((*it).id(), widget);
			m_layout.addWidget(widget);
		}

		widget->setName(QString::fromStdString((*it).name()));
		widget->show();
	}

	// Gather missing ids.
	std::vector<std::string> idsToDelete;
	for (auto it = m_widgets.begin(); it != m_widgets.end(); it++) {
		if (!findInItems(items, it->first)) {
			idsToDelete.push_back(it->first);
		}
	}

	// Delete missing widgets.
	for (auto it = idsToDelete.begin(); it != idsToDelete.end(); it++) {
		if (auto found = m_widgets.find(*it); found != m_widgets.end())
			delete found->second;
		m_widgets.erase((*it));
	}

	// Resize container.
	QSize hint = m_container.sizeHint();
	m_container.resize(size().width() - 20, hint.height());
}

bool BrainListWidget::findInItems(
	const std::vector<Brain>& items,
	std::string id
) {
	for (auto it = items.begin(); it != items.end(); it++) {
		if (QString str = QString::fromStdString((*it).id()); str == id) {
			return true;
		}
	}

	return false;
}

// Slots.

void BrainListWidget::onItemClicked(BrainItemWidget *view) {
	emit itemClicked(view->id().toStdString());
}

void BrainListWidget::onItemDeleteClicked(BrainItemWidget *view) {
	emit itemDeleteClicked(view->id().toStdString());	
}

void BrainListWidget::onNewItemClicked() {
	emit newItemClicked();
}

