#include "plugin.h"

#include <QTranslator>
#include <QLocale>

Plugin::Plugin(QObject *parent) :
    QObject(parent)
{
    translator = new QTranslator;

    panes.append(new BluetoothManagement());
}

QList<StatusCenterPaneObject*> Plugin::availablePanes() {
    return panes;
}

void Plugin::loadLanguage(QString language) {
    translator->load(language, QString(SHAREDIR) + "translations");
    QApplication::instance()->installTranslator(translator);
}
