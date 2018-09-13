#include "plugin.h"

Plugin::Plugin(QObject *parent) :
    QObject(parent)
{
    translator = new QTranslator;

    panes.append(new Overview());
    panes.append(new OverviewSettings());
}

QList<StatusCenterPaneObject*> Plugin::availablePanes() {
    return panes;
}

void Plugin::loadLanguage(QString language) {
    translator->load(language, QString(SHAREDIR) + "translations");
    QApplication::instance()->installTranslator(translator);

    for (StatusCenterPaneObject* pane : panes) {
        pane->message("retranslate");
    }
}
