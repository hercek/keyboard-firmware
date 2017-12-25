#include <Qt>
#include <QCursor>
#include <QDebug>
#include <QLayout>
#include <QPushButton>
#include "layoutview.h"
#include "layoutpresenter.h"
#include "layout.h"
#include "layoutwidget.h"
#include "layeredlayoutwidget.h"
#include "keyselectionview.h"
#include "hidtables.h"

LayoutView::LayoutView(LayoutPresenter *presenter)
	: mPresenter(presenter)
	, mLayoutWidget(new LayeredLayoutWidget)
	, mKeySelectionView(NULL)
{
	QPushButton *saveToFileButton = new QPushButton(tr("Save to File"));
	QPushButton *loadFromFileButton = new QPushButton(tr("Load from File"));
	QPushButton *loadDefaultsButton = new QPushButton(tr("Load Defaults"));

	QHBoxLayout *buttonLayout = new QHBoxLayout;
	buttonLayout->addWidget(saveToFileButton,0);
	buttonLayout->addWidget(loadFromFileButton,0);
	buttonLayout->addWidget(loadDefaultsButton,0);

	QGridLayout *layout = new QGridLayout;
	layout->addLayout(buttonLayout, 0, 1, 1, 1);
	layout->addWidget(mLayoutWidget, 1, 0, 1, 3);
	setLayout(layout);

	mUpdatingLogicalKeyIndex = Layout::NO_KEY;

	connect(mLayoutWidget, SIGNAL(logicalKeyClicked(LogicalKeycode)),
	        this, SLOT(handleLogicalKeyClicked(LogicalKeycode)));

	connect(saveToFileButton, SIGNAL(clicked()),
	        mPresenter, SLOT(saveToFile()));
	connect(loadFromFileButton, SIGNAL(clicked()),
	        mPresenter, SLOT(loadFromFile()));
	connect(loadDefaultsButton, SIGNAL(clicked()),
	        mPresenter, SLOT(loadDefaults()));
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
