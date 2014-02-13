#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <iostream>
#include <string>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // Ctrl+w shortcut
    QShortcut* ctrlw = new QShortcut(QKeySequence(tr("Ctrl+w")), this);
    ctrlw->setContext(Qt::ApplicationShortcut);
    connect(ctrlw, SIGNAL(activated()), this, SLOT(close()));

    // ui connections
    ui->setupUi(this);
    connect(ui->viewer, SIGNAL(infoChanged()), this, SLOT(updateInfo()));
    connect(ui->tf, SIGNAL(tfChanged(mslib::TF&)), ui->viewer, SLOT(tfChanged(mslib::TF&)));
    connect(ui->zoom, SIGNAL(valueChanged(int)), ui->viewer, SLOT(zoomChanged(int)));
    connect(ui->viewer, SIGNAL(zoomSignal(int)), ui->zoom, SLOT(setValue(int)));
    connect(ui->cut, SIGNAL(valueChanged(int)), ui->viewer, SLOT(cutChanged(int)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_open_clicked()
{
    std::string filename = QFileDialog::getOpenFileName(this, tr("Open Explorable Image"), QString(), tr("Explorable Images (*.png)")).toStdString();
    if (!ui->viewer->open(filename))
    {
        // TODO: handle error
    }
}

void MainWindow::updateInfo()
{
    std::string infoViewer = ui->viewer->getInfo();
    ui->info->setText(QString(infoViewer.c_str()));
}
