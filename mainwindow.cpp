#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "solver.h"

#include <QGraphicsPixmapItem>
#include <QPainter>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QLabel>
#include <QDoubleSpinBox>

#include <map>

using namespace  std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    layout = new QFormLayout();
    loadModels();

    QTimer::singleShot(0, this, &MainWindow::init);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_reset_clicked()
{
    init();
}

void MainWindow::on_start_clicked()
{
    updateModel();
    ui->glWidget->start();
}

void MainWindow::on_stop_clicked()
{
    ui->glWidget->stop();
}

void MainWindow::on_gridSize_editingFinished()
{
    init();
}

void MainWindow::on_save_clicked()
{
    // TODO: consider doing save operations here (also for saving custom model)
    QString fileName = QFileDialog::getSaveFileName(this,
           tr("Save Picture"), "",
           tr("PNG (*.png);;All Files (*)"));

    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::information(this, tr("Unable to open file"), file.errorString());
        return;
    }

    ui->glWidget->pixmap().save(&file, "PNG");
}

void MainWindow::init()
{
    ui->glWidget->stop();

    int size = ui->gridSize->value();
    float dt = ui->dt->value();
    ui->glWidget->init(size, dt);
}

void MainWindow::loadModels()
{
    // TODO: load from file
    m_models = {
        {"Gray-Scott", {
             {
                 {"du", {0,1,0.00002}},
                 {"dv", {0,1,0.00001}},
                 {"b", {0,1,0.025}},
                 {"d", {0,1,0.082}}
             },
             "-x*y*y+b-b*x",
             "x*y*y-d*y"
         }},
        {"Fitzhugh-Nagumo", {
             {
                 {"du", {0,1,0.00002}},
                 {"dv", {0,1,0.00001}},
                 {"b", {0,1,0.04}},
                 {"d", {0,1,0.1}},
                 {"lambda", {0,1,0.1}}
             },
             "-x*y*y+b-b*x",
             "x*y*y-d*y"
         }},
        {"Custom", {
             {
                 {"du", {0,1,0.00002}},
                 {"dv", {0,1,0.00001}}
             },
             "",
             ""
         }}
    };

    ui->models->addItems(QStringList(m_models.keys()));
}

void MainWindow::setModel()
{
    Model model = m_models[ui->models->currentText()];
    Solver *solver = ui->glWidget->solver();
    solver->setModel(model);
}

void MainWindow::updateModel()
{
    Model model = m_models[ui->models->currentText()];

    QMap<QString, QDoubleSpinBox*>::iterator it = params.begin();
    while (it != params.end()) {
        QString paramName = it.key();
        double paramValue = it.value()->value();
        model.params[paramName.toStdString()].value = paramValue;
        ++it;
    }

    model.fu = ui->fu->text().toStdString();
    model.fv = ui->fv->text().toStdString();

    Solver *solver = ui->glWidget->solver();
    solver->setModel(model);
}

void MainWindow::on_models_currentTextChanged(const QString &arg1)
{
    int rows = layout->rowCount();
    for (int i = rows -1; i >= 0; --i) {
        qDebug() << "Removing row " << i;
        layout->removeRow(i);
    }

    qDebug() << "Setting model " << arg1;
    Model model = m_models[arg1];

    ui->fu->setText(QString::fromStdString(model.fu));
    ui->fv->setText(QString::fromStdString(model.fv));

    // setup ui with spinboxes and linedit for model parameters
    params.clear();
    map<string, param>::iterator it = model.params.begin();
    while (it != model.params.end()) {
        QLabel *label = new QLabel(QString(it->first.c_str()));

        QDoubleSpinBox *spinbox = new QDoubleSpinBox();
        spinbox->setDecimals(6);
        spinbox->setMinimum(it->second.min);
        spinbox->setMaximum(it->second.max);
        spinbox->setValue(it->second.value);
        spinbox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

        params.insert(label->text(), spinbox);

        layout->addRow(label, spinbox);

        ++it;
    }

    ui->params->setLayout(layout);
}
