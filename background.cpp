#include "background.h"
#include "ui_background.h"

Background::Background(MainWindow* mainwindow, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Background)
{
    ui->setupUi(this);
    this->mainwindow = mainwindow;

    QRect screenGeometry = QApplication::desktop()->screenGeometry();

    QSettings settings;
    QString backPath = settings.value("desktop/background", "").toString();
    if (backPath == "") {
        backPath = "/usr/share/icons/theos/backgrounds/triangle/1920x1080.png";
        settings.setValue("desktop/background", backPath);
    }

    QGraphicsScene* scene = new QGraphicsScene();
    scene->addPixmap(QPixmap(backPath));
    scene->setSceneRect(screenGeometry);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setSceneRect(screenGeometry);
}

Background::~Background()
{
    delete ui;
}

void Background::on_graphicsView_customContextMenuRequested(const QPoint &pos)
{
    QMenu *menu = new QMenu(this);
    menu->addSection("For desktop");
    menu->addAction(ui->actionChange_Background);
    menu->addSection("For system");
    menu->addAction(ui->actionOpen_theShell_Settings);
    if (QFile("/usr/bin/systemsettings5").exists()) {
        menu->addAction(ui->actionOpen_System_Settings);
    }
    menu->addAction(ui->actionOpen_Status_Center);

    menu->exec(ui->graphicsView->mapToGlobal(pos));
}

void Background::on_actionOpen_Status_Center_triggered()
{
    mainwindow->getInfoPane()->show(InfoPaneDropdown::Clock);
}

void Background::on_actionOpen_theShell_Settings_triggered()
{
    mainwindow->getInfoPane()->show(InfoPaneDropdown::Settings);
}

void Background::on_actionOpen_System_Settings_triggered()
{
    QProcess::startDetached("systemsettings5");
}

void Background::on_actionChange_Background_triggered()
{
    ChooseBackground *background = new ChooseBackground(this);
    background->show();
}
