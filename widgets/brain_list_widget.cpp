#include <QWidget>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>

#include "widgets/brain_list_widget.h"
#include "widgets/brain_item_widget.h"
#include "widgets/toast_widget.h"

BrainListWidget::BrainListWidget(QWidget *parent, Style *style)
	: BaseWidget(parent, style),
	m_area(this),
	m_container(nullptr),
	m_layout(&m_container),
	m_text(this)
{
	setContentsMargins(QMargins(8, 8, 8, 8));
	m_area.setWidget(&m_container);
	m_area.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_area.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	m_area.setStyleSheet(QString("border: none"));
	m_container.show();
	m_layout.setContentsMargins(0, 0, 16, 0);
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

	// Set welcome text.
	m_text.setStyleSheet(
		QString("font-size: 16px; color: %1")
		.arg(style->textEditColor().name(QColor::HexArgb))
	);
	QString textPattern = QString("<h2>Welcome!</h2>");
	m_text.setTextFormat(Qt::RichText);
	m_text.setText(textPattern);
	m_text.setWordWrap(true);
	m_text.setAlignment(Qt::AlignHCenter | Qt::AlignTop);
}

void BrainListWidget::showError(QString name) {
	if (m_error != nullptr)
		delete m_error;

	m_error = new ToastWidget(m_style, name);
	m_error->show(this);
}

void BrainListWidget::showEvent(QShowEvent*) {
	emit shown();
}

void BrainListWidget::resizeEvent(QResizeEvent *event) {
	const int spacing = 8;
	QSize size = event->size();
	QMargins margins = contentsMargins();
	QSize areaSize = QSize(
		(size.width() - margins.left() - margins.right() - spacing) / 2,
		size.height() - margins.top() - margins.bottom()
	);

	m_area.setGeometry(
		margins.left(), margins.top(),
		areaSize.width(),
		areaSize.height()
	);

	QSize hint = m_container.sizeHint();
	m_container.resize(areaSize.width() - 16, hint.height());

	m_text.setGeometry(
		margins.left() + areaSize.width() + spacing,
		margins.top(),
		areaSize.width(),
		areaSize.height()
	);
}

void BrainListWidget::layoutContainer() {
	const int spacing = 8;
	QSize currentSize = size();
	QMargins margins = contentsMargins();
	QSize areaSize = QSize(
		(currentSize.width() - margins.left() - margins.right() - spacing) / 2,
		currentSize.height() - margins.top() - margins.bottom()
	);
	QSize hint = m_container.sizeHint();
	m_container.resize(areaSize.width() - 16, hint.height());
}

// Model.

void BrainListWidget::setItems(BrainList list) {
	int idx = 1;

	// Update text.
	if (list.items.size() == 0) {
		QString textPattern = tr("<h2>Welcome!</h2>\n\nYou currently have no saved Brains. Let's create one!");
		m_text.setText(textPattern);
	} else {
		QString textPattern = tr("<h2>Welcome!</h2>\n\n<p>You currently have %1 brain(s).</p>\n\n<p>All of your data is stored in <b>%2</b>.</p>\n\n<p>Total size of saved thoughts: <b>%3</b></p>")
			.arg(list.items.size())
			.arg(QString::fromStdString(list.location))
			.arg(formatSize(list.sizeBytes));
		m_text.setText(textPattern);
	}

	// Sort by timestamp.
	std::vector<Brain>& items = list.items;
	std::sort(items.begin(), items.end(), [](const Brain& a, const Brain& b) {
		return a.timestamp() > b.timestamp();
	});

	// Create new widgets.
	for (auto it = items.begin(); it != items.end(); it++) {
		BrainItemWidget *widget = nullptr;
		if (auto found = m_widgets.find((*it).id()); found != m_widgets.end()) {
			widget = found->second;

			if (
				int foundIndex = m_layout.indexOf(widget);
				foundIndex != -1
			) {
				QLayoutItem *item = m_layout.takeAt(foundIndex);
				m_layout.insertItem(idx, item);
				idx += 1;
			}
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
			connect(
				widget, &BrainItemWidget::renameClicked,
				this, &BrainListWidget::onItemRenameClicked
			);

			m_widgets.insert_or_assign((*it).id(), widget);
			m_layout.addWidget(widget);

			// Rearrange.
			QLayoutItem *item = m_layout.takeAt(m_layout.count() - 1);
			m_layout.insertItem(idx, item);
			idx += 1;
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
	layoutContainer();

	// Repaint.
	update();
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
	emit itemClicked(
		view->id().toStdString(),
		view->name().toStdString()
	);
}

void BrainListWidget::onItemDeleteClicked(BrainItemWidget *view) {
	QMessageBox dialog;
	dialog.setText(tr("Delete \"%1\"?").arg(view->name()));
	dialog.setInformativeText(
		tr("Are you sure you want to delete \"%1\"? All of the Brain's data will be erased from disk.")
		.arg(view->name())
	);
	dialog.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
	dialog.setDefaultButton(QMessageBox::Cancel);

	QSize size = dialog.sizeHint();
	QRect windowPos = window()->geometry();
	dialog.move(
		windowPos.x() + (windowPos.width() - size.width()) / 2,
		windowPos.y() + (windowPos.height() - size.height()) / 2
	);

	int ret = dialog.exec();
	if (ret == QMessageBox::Yes) {
		emit itemDeleteClicked(view->id().toStdString());
	}
}

void BrainListWidget::onNewItemClicked() {
	QInputDialog dialog;
	dialog.setLabelText(tr("Enter a unique name for the new brain.\nAvoid using special symbols that might not be supported by the filesystem."));
	dialog.setWindowTitle(tr("Create new brain"));

	QSize size = dialog.sizeHint();
	QRect windowPos = window()->geometry();
	dialog.move(
		windowPos.x() + (windowPos.width() - size.width()) / 2,
		windowPos.y() + (windowPos.height() - size.height()) / 2
	);

	int ret = dialog.exec();
	if (ret == QDialog::Accepted && !dialog.textValue().isEmpty()) {
		emit newItemCreated(dialog.textValue().trimmed().toStdString());
	}
}

void BrainListWidget::onItemRenameClicked(BrainItemWidget *widget) {
	assert(widget != nullptr);

	QString oldName = widget->name();

	QInputDialog dialog;
	dialog.setLabelText(tr("Enter a new unique name for the brain:"));
	dialog.setWindowTitle(tr("Rename a brain"));
	dialog.setTextValue(oldName);

	QSize size = dialog.sizeHint();
	QRect windowPos = window()->geometry();
	dialog.move(
		windowPos.x() + (windowPos.width() - size.width()) / 2,
		windowPos.y() + (windowPos.height() - size.height()) / 2
	);

	int ret = dialog.exec();
	if (ret == QDialog::Accepted && !dialog.textValue().isEmpty()) {
		emit itemRenamed(
			widget->id().toStdString(),
			dialog.textValue().trimmed().toStdString()
		);
	}
}

// Helpers

QString BrainListWidget::formatSize(uint64_t sizeInBytes) {
	const char* prefixes[] = {
		"B",
		"KB",
		"MB",
		"GB",
		"TB"
	};

	int idx = 0;
	double sizeDouble = sizeInBytes;
	while (sizeDouble > 1024.0) {
		idx += 1;
		sizeDouble /= 1024.0;
	}

	char buffer[256] = {0};
	snprintf(buffer, 255, "%.2f %s", sizeDouble, prefixes[idx]);

	QString result = QString(buffer);
	return result;
}

