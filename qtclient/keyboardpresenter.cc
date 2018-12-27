#include <iostream>
#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>
#include "keyboardpresenter.h"
#include "keyboardcomm.h"
#include "keyboardmodel.h"
#include "keyboardview.h"
#include "layout.h"
#include "hexdump.h"

KeyboardPresenter::KeyboardPresenter()
	: mKeyboardModel(NULL)
{
	mView.reset(new KeyboardView(this, createSubviewList()));

	// distribute model to sub-presenters
	connect(this, SIGNAL(modelChanged(const QSharedPointer<KeyboardModel>&)),
			&mLayoutPresenter, SLOT(setModel(const QSharedPointer<KeyboardModel>&)));

	connect(this, SIGNAL(modelChanged(const QSharedPointer<KeyboardModel>&)),
			&mProgramsPresenter, SLOT(setModel(const QSharedPointer<KeyboardModel>&)));

	connect(this, SIGNAL(modelChanged(const QSharedPointer<KeyboardModel>&)),
			&mTriggersPresenter, SLOT(setModel(const QSharedPointer<KeyboardModel>&)));

	connect(this, SIGNAL(modelChanged(const QSharedPointer<KeyboardModel>&)),
			&mValuesPresenter, SLOT(setModel(const QSharedPointer<KeyboardModel>&)));

	connect(this, SIGNAL(deviceChanged(const QSharedPointer<Device>&)),
			&mValuesPresenter, SLOT(setDevice(const QSharedPointer<Device>&)));
}

QList<QPair<QString, QWidget*> > KeyboardPresenter::createSubviewList() {
	QList<QPair<QString, QWidget*> > subviews;
	subviews << QPair<QString, QWidget*>(
		tr("Layout"), mLayoutPresenter.getWidget());
	subviews << QPair<QString, QWidget*>(
		tr("Programs"), mProgramsPresenter.getWidget());
	subviews << QPair<QString, QWidget*>(
		tr("Triggers"), mTriggersPresenter.getWidget());
	subviews << QPair<QString, QWidget*>(
		tr("Advanced"), mValuesPresenter.getWidget());
	return subviews;
}


// required to use a QScopedPointer with a forward decl (destructor
// must have full type available)
KeyboardPresenter::~KeyboardPresenter()
{
}

void KeyboardPresenter::showAction() {
	updateDeviceListAction();
	mView->show();
}

void KeyboardPresenter::updateDeviceListAction() {
	QStringList names;

	mDevices = KeyboardComm::enumerate();
	const KeyboardComm::DeviceList& devices = mDevices;
	for (KeyboardComm::DeviceList::const_iterator it = devices.constBegin();
	     it != devices.constEnd();
         ++it)
	{
		try {
			names.push_back((*it)->getName());
		}
		catch (const DeviceError& e) {
			names.push_back("ERROR");
			qWarning() << "DeviceError during update: " << e.what();
		}
	}

	mView->updateDevices(names);
}

void KeyboardPresenter::selectDeviceAction(int index) {
	if (index == -1) {
		mCurrentDevice.clear();
		mView->showNoKeyboard();
	}
	else {
		mCurrentDevice = mDevices.at(index);
		emit deviceChanged(mCurrentDevice);
		mView->showKeyboard();
		downloadAction();
	}
}

void KeyboardPresenter::downloadAction() {
	if (!mCurrentDevice) return;
	try {
		QSharedPointer<DeviceSession> session = mCurrentDevice->newSession();
		mKeyboardModel = QSharedPointer<KeyboardModel>(
			new KeyboardModel(session.data()));
		emit modelChanged(mKeyboardModel);
	}
	catch (DeviceError& devError) {
		qDebug() << "Error downloading settings: " << devError.what();
	}
}


void KeyboardPresenter::uploadAction() {
	if (!mCurrentDevice) return;
	{ // check that the model (possibly loaded from file) corresponds to the connected keyboard
		QSharedPointer<DeviceSession> curKbSession(mCurrentDevice->newSession());
		if ( curKbSession->getLayoutID() != mKeyboardModel->getLayoutID() ||
		     curKbSession->getMappingSize() != mKeyboardModel->getMappingSize() ||
		     curKbSession->getNumPrograms() != mKeyboardModel->getNumPrograms() ||
		     curKbSession->getMacroMaxKeys() != mKeyboardModel->getKeysPerTrigger() ||
		     curKbSession->getProgramSpace() != mKeyboardModel->getProgramSpace() ||
		     curKbSession->getMacroIndexSize() != mKeyboardModel->getMacroIndexSize() ||
		     curKbSession->getMacroStorageSize() != mKeyboardModel->getMacroStorageSize() ) {
			QMessageBox::warning(0, "KeyboardClient",
				"Advanced parameter mismatch between the selected keyboard and the keyboard config loaded in this client.");
			return; }
	}

	QByteArray mapping = *mKeyboardModel->getMapping();
	QByteArray programs =
		Program::encodePrograms(*mKeyboardModel->getPrograms(),
								mKeyboardModel->getNumPrograms(),
								mKeyboardModel->getProgramSpace());

	QPair<QByteArray, QByteArray> encodedMacros =
		Trigger::encodeTriggers(*mKeyboardModel->getTriggers(),
								mKeyboardModel->getKeysPerTrigger(),
								mKeyboardModel->getMacroIndexSize(),
								mKeyboardModel->getMacroStorageSize());

	try {
		QSharedPointer<DeviceSession> session = mCurrentDevice->newSession();

		// qDebug() has an implicit endl, we add an extra one for a
		// gap between dumps.

		qDebug() << "Uploading mapping:" << endl
				 << hexdump(mapping) << endl;
		session->setMapping(mapping);

		if(programs.length() > 0) {
			qDebug() << "Uploading programs:" << endl
					 << hexdump(programs) << endl;
			session->setPrograms(programs);
		}

		qDebug() << "Uploading macro index:" << endl
				 << hexdump(encodedMacros.first) << endl;
		session->setMacroIndex(encodedMacros.first);

		if(encodedMacros.second.length() > 0) {
			qDebug() << "Uploading macro data: " << endl
					 << hexdump(encodedMacros.second) << endl;
			session->setMacroStorage(encodedMacros.second);
		}
	}
	catch (DeviceError& e) {
		qDebug() << "DeviceError uploading: " << e.what();
	}
}


void KeyboardPresenter::saveToFileAction() {
	QString fileName = QFileDialog::getSaveFileName(0,
		"Specify file name to save Keybaord Config into", "", "Keyboard Config File (*.kbc)");
	if (fileName.isEmpty()) return;
	if (!fileName.endsWith(".kbc")) fileName += ".kbc";
	QFile file(fileName);
	if (!file.open(QIODevice::WriteOnly) ) {
		QMessageBox::warning(0, "KeyboardClient", "Cannot open the file for writing."); return; }
	QDataStream out(&file);
	out << *mKeyboardModel;
}


void KeyboardPresenter::loadFromFileAction() {
	QString fileName = QFileDialog::getOpenFileName(0,
		"Select file name to load Keybaord Coffing from", "", "Keybaord Config File (*.kbc)");
	if (fileName.isEmpty()) return;
	QFile file(fileName);
	if ( !file.open(QIODevice::ReadOnly) ) {
		QMessageBox::warning(0, "KeyboardClient", "Cannot open the file for reading."); return; }

	QDataStream in(&file);
	QSharedPointer<KeyboardModel> newKeyboardModel(new KeyboardModel);
	in >> *newKeyboardModel;
	if (in.status() != QDataStream::Ok || newKeyboardModel->getLayoutID() == 0) {
		QMessageBox::warning(0, "KeyboardClient", "The file is corrupted."); return; }

	mKeyboardModel = newKeyboardModel;
	emit modelChanged(mKeyboardModel);
}
