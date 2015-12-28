#include <Qt>
#include <QCursor>
#include <QDebug>
#include <QLayout>
#include <QPushButton>
#include <QCheckBox>
#include "layoutview.h"
#include "layoutpresenter.h"
#include "layout.h"
#include "layoutwidget.h"
#include "keyselectionview.h"
#include "hidtables.h"

LayoutView::LayoutView(LayoutPresenter *presenter)
	: mPresenter(presenter)
	, mLayoutWidget(new LayoutWidget)
	, mKeySelectionView(NULL)
	, mShowingMainLayer(true)
{
	QPushButton *loadDefaultsButton = new QPushButton(tr("Load Defaults"));
	QCheckBox* keypadCheckBox = new QCheckBox(tr("Show Keypad"));

	QGridLayout *layout = new QGridLayout;
	layout->addWidget(loadDefaultsButton, 0, 1, 1, 1);
	layout->addWidget(mLayoutWidget, 1, 0, 1, 3);
	layout->addWidget(keypadCheckBox, 2, 1, 1, 1, Qt::AlignTop|Qt::AlignHCenter);
	setLayout(layout);

	mUpdatingLogicalKeyIndex = Layout::NO_KEY;

	connect(mLayoutWidget, SIGNAL(logicalKeyClicked(LogicalKeycode)),
	        this, SLOT(handleLogicalKeyClicked(LogicalKeycode)));

	connect(loadDefaultsButton, SIGNAL(clicked()),
	        mPresenter, SLOT(loadDefaults()));

	connect(keypadCheckBox, SIGNAL(stateChanged(int)),
	        mLayoutWidget, SLOT(setKeypadLayerSelected(int)));
}

void LayoutView::setKeyboardLayout(const Layout& layout) {
	mLayoutWidget->setKeyboardLayout(layout);
}

void LayoutView::setMapping(const QByteArray& m) {
	// just pass the mapping reference down to the widget, which will
	// take a copy
	mLayoutWidget->setMapping(m);
}

void LayoutView::handleLogicalKeyClicked(LogicalKeycode logicalKey) {
	if (!mKeySelectionView) {
		mKeySelectionView = new KeySelectionView(this);
		connect(mKeySelectionView, SIGNAL(hidUsageSelected(QString, HIDKeycode)),
				this, SLOT(hidUsageSelected(QString, HIDKeycode)));
		connect(mKeySelectionView, SIGNAL(dismissed()),
				this, SLOT(keySelectionFinished()));
	}
	mKeySelectionView->move(QCursor::pos());
	mKeySelectionView->show();
	mUpdatingLogicalKeyIndex = logicalKey;
	mLayoutWidget->setSelection(mUpdatingLogicalKeyIndex);
}


void LayoutView::hidUsageSelected(QString name, HIDKeycode hidUsage) {
	Q_UNUSED(name);
	if (mUpdatingLogicalKeyIndex == Layout::NO_KEY) {
		qDebug() << "got key without selection?!";
		return;
	}
	mPresenter->setHIDUsage(mUpdatingLogicalKeyIndex, hidUsage);
}

void LayoutView::keySelectionFinished() {
	mLayoutWidget->setSelection(QSet<uint8_t>());
}
