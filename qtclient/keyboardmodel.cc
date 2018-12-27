#include <QDebug>

#include "keyboardmodel.h"
#include "device.h"
#include "trigger.h"

#include "libusb_wrappers.h"

KeyboardModel::KeyboardModel(DeviceSession *keyboard)
	: mLayoutID(keyboard->getLayoutID())
	, mMappingSize(keyboard->getMappingSize())
	, mNumPrograms(keyboard->getNumPrograms())
	, mKeysPerTrigger(keyboard->getMacroMaxKeys())
	, mProgramSpace(keyboard->getProgramSpace())
	, mMacroIndexSize(keyboard->getMacroIndexSize())
	, mMacroStorageSize(keyboard->getMacroStorageSize())
	, mDefaultMapping(keyboard->getDefaultMapping())
	, mMapping(keyboard->getMapping())
	, mPrograms(
	    Program::readPrograms(keyboard->getPrograms(),
	                          keyboard->getNumPrograms()))
	, mTriggers(
	    Trigger::readTriggers(keyboard->getMacroIndex(),
	                          keyboard->getMacroStorage(),
	                          mKeysPerTrigger))
	, mLayout(
	    Layout::readLayout(keyboard->getLayoutID()))
{
}

QDataStream& operator<<(QDataStream& out, KeyboardModel const& kbm){
	out << uint16_t(1);
	out << kbm.mLayoutID << kbm.mMappingSize << kbm.mNumPrograms << kbm.mKeysPerTrigger <<
		kbm.mProgramSpace << kbm.mMacroIndexSize << kbm.mMacroStorageSize;
	out << kbm.mDefaultMapping << kbm.mMapping;

	QByteArray programs(
		Program::encodePrograms(kbm.mPrograms, kbm.mNumPrograms, kbm.mProgramSpace) );
	out << programs;

	QPair<QByteArray,QByteArray> macros(
		Trigger::encodeTriggers(kbm.mTriggers, kbm.mKeysPerTrigger, kbm.mMacroIndexSize, kbm.mMacroStorageSize) );
	out << macros.first << macros.second;
	return out;
}

QDataStream& operator>>(QDataStream& in, KeyboardModel& kbm){
	uint16_t dataFileVersion;
	in >> dataFileVersion;
	if ( dataFileVersion != 1 ) {
		in.setStatus( QDataStream::ReadCorruptData ); return in; }

	in >> kbm.mLayoutID >> kbm.mMappingSize >> kbm.mNumPrograms >> kbm.mKeysPerTrigger >>
		kbm.mProgramSpace >> kbm.mMacroIndexSize >> kbm.mMacroStorageSize;
	in >> kbm.mDefaultMapping >> kbm.mMapping;

	QByteArray programs, macroIdx, macroStg;
	in >> programs >> macroIdx >> macroStg;

	if ( kbm.mMappingSize != kbm.mDefaultMapping.size() ||
		 kbm.mMappingSize != kbm.mMapping.size() ||
		 kbm.mProgramSpace < programs.size() ||
		 kbm.mMacroIndexSize != macroIdx.size() ||
		 kbm.mMacroStorageSize < macroStg.size()) {
		in.setStatus( QDataStream::ReadCorruptData );
		return in;
	}

	kbm.mPrograms = Program::readPrograms(programs, kbm.mNumPrograms);
	kbm.mTriggers = Trigger::readTriggers(macroIdx, macroStg, kbm.mKeysPerTrigger);

	kbm.mLayout = Layout::readLayout(kbm.mLayoutID);
	return in;
}
