#include <QApplication>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>

#include "hidtables.h"
#include "layoutpresenter.h"
#include "layout.h"
#include "keyboardmodel.h"

LayoutPresenter::LayoutPresenter()
	: mModel(NULL)
{
	mView = new LayoutView(this);
}

LayoutPresenter::~LayoutPresenter() {
	if (!mView->parent()) {
		delete mView;
	}
}

void LayoutPresenter::setModel(const QSharedPointer<KeyboardModel>& model) {
	mModel = model;

	mView->setKeyboardLayout(*mModel->getLayout());
	mView->setMapping(*mModel->getMapping());
}


// special keys like keypad and program must be the same on all
// layers.
static bool keyIsSpecial(HIDKeycode hidKey) {
	return (hidKey == HIDTables::HIDUsageProgram ||
	        hidKey == HIDTables::HIDUsageMacroShift ||
	        hidKey == HIDTables::HIDUsageFunctionShift ||
	        hidKey == HIDTables::HIDUsageKeypadShift ||
	        hidKey == HIDTables::HIDUsageLayerLock);
}

void LayoutPresenter::setHIDUsage(LogicalKeycode logicalKey, HIDKeycode hidKey) {
	QByteArray *mapping = mModel->getMapping();

	const auto& layout = *mModel->getLayout();

	PhysicalKeycode pkey =
		layout.logicalKeycodeToPhysical(logicalKey);

	// new key is special => set on all layers
	if (keyIsSpecial(hidKey)) {
		for (unsigned layer = 0; layer < layout.layerCnt; layer++) {
			LogicalKeycode logicalKey = layout.physicalKeycodeToLogical(pkey, layer);
			(*mapping)[logicalKey] = hidKey;
		}
	}
	else {
		// old key was special => clear other layers
		if (keyIsSpecial((*mapping)[logicalKey])) {
			for (unsigned layer = 0; layer < layout.layerCnt; layer++) {
				LogicalKeycode logicalKey = layout.physicalKeycodeToLogical(pkey, layer);
				(*mapping)[logicalKey] = HIDTables::HIDUsageNoKey;
			}
		}

		(*mapping)[logicalKey] = hidKey;
	}

	// and update the view
	mView->setMapping(*mapping);
}

void LayoutPresenter::saveToFile() {
	QByteArray *mapping = mModel->getMapping();
	QString fileName = QFileDialog::getSaveFileName(0,
		"Specify file name to save Keybaord Remap into", "", "Remap File (*.rmp)");
	if (fileName.isEmpty())
		return;
	if (!fileName.endsWith(".rmp"))
		fileName += ".rmp";
	QFile file(fileName);
	if (file.open(QIODevice::WriteOnly) )
		file.write(*mapping);
}

void LayoutPresenter::loadFromFile() {
	QByteArray *mapping = mModel->getMapping();
	QString fileName = QFileDialog::getOpenFileName(0,
		"Select file name to load Keybaord Remap from", "", "Remap File (*.rmp)");
	if (fileName.isEmpty())
		return;
	QFile file(fileName);
	if ( !file.open(QIODevice::ReadOnly) )
		return;
	QByteArray blob = file.readAll();
	if (mapping->size() != blob.size()) {
		QMessageBox::warning(0, "KeyboardClient", "Incorrect file length.");
		return;
	}
	*mapping = blob;
	mView->setMapping(*mapping);
}

void LayoutPresenter::loadDefaults() {
	QByteArray *mapping = mModel->getMapping();
	*mapping = mModel->getDefaultMapping();
	mView->setMapping(*mapping);
}
