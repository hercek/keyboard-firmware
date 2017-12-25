#include <QApplication>
#include "keyboardpresenter.h"
#include "libusb.h"

int main(int argc, char **argv) {
#if defined(Q_OS_WIN)
	{
	HMODULE hModule = GetModuleHandleW(NULL);
	WCHAR path[MAX_PATH];
	GetModuleFileNameW(hModule, path, MAX_PATH);
	QString pluginDir( QString::fromWCharArray(path) );
	int pos = pluginDir.lastIndexOf( '\\' );
	pluginDir.truncate( pos );
	pluginDir += "\\plugins";
	QCoreApplication::addLibraryPath(pluginDir);
	}
#endif

	QApplication app(argc, argv);

	libusb_init(NULL);

	{
		KeyboardPresenter mainPresenter;
		mainPresenter.showAction();
		app.exec();
	}

	libusb_exit(NULL);
	return 0;
}
