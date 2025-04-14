#ifndef H_DISMISSABLE_PRESENTER
#define H_DISMISSABLE_PRESENTER

#include <QObject>

class DismissablePresenter: public QObject {
	Q_OBJECT

public slots:
	virtual void onDismiss() = 0;
};

#endif
