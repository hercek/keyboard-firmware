#include <stdexcept>

#include <QXmlDefaultHandler>
#include <QXmlSimpleReader>

#include "layout.h"

namespace {
class XMLLayoutHandler : public QXmlDefaultHandler {
	Layout* mLayout;

public:
	XMLLayoutHandler(Layout* layout)
		: mLayout(layout)
	{}

	virtual bool startElement(const QString& namespaceURI,
	                          const QString& localName,
	                          const QString& qName,
	                          const QXmlAttributes& atts)
	{
		Q_UNUSED(namespaceURI);
		Q_UNUSED(qName);

		if (localName == "keyboard") {
			mLayout->layout = atts.value("layout");
			mLayout->imageName = atts.value("image");
			bool ok;
			mLayout->layerCnt = atts.value("layerCnt").toUInt(&ok);
			if (!ok) mLayout->layerCnt = 2;
		}
		else if (localName == "key") {
			Layout::Key k = { atts.value("name"),
			                  QRect(atts.value("x").toInt(),
			                        atts.value("y").toInt(),
			                        atts.value("w").toInt(),
			                        atts.value("h").toInt())
			};
			mLayout->keys.push_back(k);
		}
		return true;
	}
};
};

Layout Layout::readLayout(int layoutID) {
	Layout layout;

	QString layoutXml =
		QString(":/layout/%1.xml").arg(layoutID);
	XMLLayoutHandler handler(&layout);
	QXmlSimpleReader xmlReader;
	QFile layoutXmlFile(layoutXml);
	QXmlInputSource source(&layoutXmlFile);
	xmlReader.setContentHandler(&handler);
	if (!xmlReader.parse(source)) {
		throw std::runtime_error("Failed to parse layout xml");
	}

	return layout;
}

QString Layout::namePosition(const LogicalKeycode logicalKeycode) const {
	PhysicalKeycode physicalKeycode = logicalKeycodeToPhysical( logicalKeycode );
	if (physicalKeycode >= keys.count())
		return QString("Invalid Position");
	unsigned layerId = layerForLogicalKey( logicalKeycode );
	switch (layerId) {
		case 0: // Normal
			return keys[physicalKeycode].name;
		case 1: // Keypad
			return QString("K:%1").arg(keys[physicalKeycode].name);
		case 2: // Function
			return QString("F:%2").arg(keys[physicalKeycode].name);
		default: // Higher function layers
			return QString("F%1:%2").arg(QString::number(layerId), keys[physicalKeycode].name);
	}
}
