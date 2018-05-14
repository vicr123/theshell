#include "polkitinterface.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false);
    //MainWindow w;
    //w.show();

    PolkitInterface* interface = new PolkitInterface();
    PolkitQt1::UnixSessionSubject subject(QApplication::applicationPid());
    interface->registerListener(subject, "/org/thesuite/polkitAuthAgent");

    return a.exec();

}
