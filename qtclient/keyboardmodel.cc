#include <QDebug>

#include "keyboardmodel.h"
#include "device.h"
#include "trigger.h"

#include "libusb_wrappers.h"

KeyboardModel::KeyboardModel(DeviceSession *keyboard)
	: mLayoutID(keyboard->getLayoutID())
	, mMappingSize(keyboard->getMappingSize())
	, mNumPrograms(keyboard->getNumPrograms())
	, mProgramSpaceRaw(keyboard->getProgramSpaceRaw())
	, mProgramSpace(keyboard->getProgramSpace())
	, mMacroIndexSize(keyboard->getMacroIndexSize())
	, mMacroStorageSize(keyboard->getMacroStorageSize())
	, mKeysPerTrigger(keyboard->getMacroMaxKeys())
	, mDefaultMapping(keyboard->getDefaultMapping())
	, mPrograms(
	    Program::readPrograms(keyboard->getPrograms(),
	                          keyboard->getNumPrograms()))
	, mTriggers(
	    Trigger::readTriggers(keyboard->getMacroIndex(),
	                          keyboard->getMacroStorage(),
	                          mKeysPerTrigger))
	, mLayout(
	    Layout::readLayout(keyboard->getLayoutID()))
	, mMapping(keyboard->getMapping())
{
}
