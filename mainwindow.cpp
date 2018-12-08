#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "solver.h"

#include <QGraphicsPixmapItem>
#include <QPainter>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QLabel>
#include <QSettings>
#include <QInputDialog>
#include <QStandardPaths>
#include <QDoubleSpinBox>

#include <map>

using namespace  std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    layout(new QFormLayout())
{
    ui->setupUi(this);
    loadModels();
}

MainWindow::~MainWindow()
{
    delete layout;
    delete ui;
}

void MainWindow::on_reset_clicked()
{
    init();
}

void MainWindow::on_start_clicked()
{
    updateModel();
    ui->rdWidget->start();
}

void MainWindow::on_stop_clicked()
{
    ui->rdWidget->stop();
}

void MainWindow::on_gridSize_editingFinished()
{
    init();
}

void MainWindow::on_dt_editingFinished()
{
    float dt = ui->dt->value();
    ui->rdWidget->setTimeStep(dt);
}

void MainWindow::on_save_clicked()
{
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

    ui->rdWidget->pixmap().save(&file, "PNG");
}

void MainWindow::init()
{
    ui->rdWidget->stop();

    int size = ui->gridSize->value();
    float dt = ui->dt->value();
    ui->rdWidget->init(size, dt);
}

void MainWindow::loadModels()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QDir::separator() + QString("models");
    QDir dir(path);
    dir.mkpath(path);

    // populate dir with default models
    if(dir.isEmpty())
    {
        QHash<QString, Model> models = {
            {"Gray-Scott", {
                 {
                     {"du", {0,1,0.00002}},
                     {"dv", {0,1,0.00001}},
                     {"b", {0,1,0.025}},
                     {"d", {0,1,0.082}}
                 },
                 "-x*y^2+b-b*x",
                 "x*y^2-d*y"
             }},
            {"Fitzhugh-Nagumo", {
                 {
                     {"du", {0,1,0.00002}},
                     {"dv", {0,1,0.00001}},
                     {"b", {0,1,0.04}},
                     {"d", {0,1,0.1}},
                     {"lambda", {0,1,0.1}}
                 },
                 "-x*y^2+b-b*x",
                 "x*y^2-d*y"
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

        QHash<QString, Model>::const_iterator it = models.constBegin();
        while(it != models.constEnd())
        {
            saveModel(it.value(), it.key());
            ++it;
        }
    }

    // load from file
    QList<QFileInfo> files = dir.entryInfoList(QStringList() << "*.rd",QDir::Files);
    foreach(QFileInfo filename, files) {
        loadModel(filename.absoluteFilePath());
    }

    connect(ui->fu,&QLineEdit::editingFinished,this,&MainWindow::updateModel);
    connect(ui->fv,&QLineEdit::editingFinished,this,&MainWindow::updateModel);
}

void MainWindow::setModel(Model &model)
{
    Solver *solver = ui->rdWidget->solver();
    solver->setModel(model);
}

void MainWindow::saveModel(const Model &model, const QString &modelName)
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    path += QDir::separator() + QString("models");
    QDir dir(path);
    dir.mkpath(path);

    path += QDir::separator() + modelName + ".rd";
    QSettings settings(path, QSettings::IniFormat);
    for(const QString& key : settings.allKeys())
        settings.remove(key);

    settings.setValue("name", modelName);
    settings.setValue("reactionTerms/fu", QString::fromStdString(model.fu));
    settings.setValue("reactionTerms/fv", QString::fromStdString(model.fv));

    settings.beginWriteArray("params/");
    int count = 0;
    map<string, Param>::const_iterator it = model.params.begin();
    while (it != model.params.end())
    {
        settings.setArrayIndex(count);
        QString paramName = QString::fromStdString(it->first);
        settings.setValue("name", paramName);

        float min = it->second.min;
        settings.setValue("min", QString::number(min));

        float max = it->second.max;
        settings.setValue("max", QString::number(max));

        float value = it->second.value;
        settings.setValue("value", QString::number(value));

        ++it;
        ++count;
    }
    settings.endArray();
    settings.sync();

    loadModel(path);
}

void MainWindow::loadModel(QString fileName)
{
    Model model;
    QSettings settings(fileName, QSettings::IniFormat);

    QString fu = settings.value("reactionTerms/fu").toString();
    QString fv = settings.value("reactionTerms/fv").toString();
    model.fu = fu.toStdString();
    model.fv = fv.toStdString();

    int size = settings.beginReadArray("params");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);

        string paramName = settings.value("name").toString().toStdString();
        float min = settings.value("min").toFloat();
        float max = settings.value("max").toFloat();
        float value = settings.value("value").toFloat();
        Param param = {min, max, value};

        model.params.insert(pair<string,Param>(paramName, param));
    }
    settings.endArray();

    QString modelName = settings.value("name").toString();
    m_models.insert(modelName, model);

    if(ui->models->findText(modelName) == -1)
        ui->models->addItem(modelName);
    ui->models->setCurrentText(modelName); // trigger current model update
}

void MainWindow::clearCurrentModelLayout()
{
    int rows = layout->rowCount();
    for (int i = rows -1; i >= 0; --i)
        layout->removeRow(i);
    params.clear();
}

void MainWindow::createModelLayout(Model &model)
{
    // setup ui with spinboxes and linedit for model parameters
    ui->fu->setText(QString::fromStdString(model.fu));
    ui->fv->setText(QString::fromStdString(model.fv));

    map<string, Param>::const_iterator it = model.params.begin();
    while (it != model.params.end())
    {
        QLabel *label = new QLabel(QString(it->first.c_str()));

        QDoubleSpinBox *spinbox = new QDoubleSpinBox();
        spinbox->setDecimals(6);
        spinbox->setMinimum(it->second.min);
        spinbox->setMaximum(it->second.max);
        spinbox->setValue(it->second.value);
        spinbox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

        connect(spinbox,&QDoubleSpinBox::editingFinished,this,&MainWindow::updateModel);

        params.insert(label->text(), spinbox);
        layout->addRow(label, spinbox);

        ++it;
    }

    ui->params->setLayout(layout);
}

void MainWindow::updateModel()
{
    Model *model = &m_models[ui->models->currentText()];

    QMap<QString, QDoubleSpinBox*>::const_iterator it = params.constBegin();
    while (it != params.constEnd())
    {
        string paramName = it.key().toStdString();
        QDoubleSpinBox *box = it.value();
        double paramValue = box->value();
        model->params[paramName].value = paramValue;
        ++it;
    }

    model->fu = ui->fu->text().toStdString();
    model->fv = ui->fv->text().toStdString();

    setModel(*model);
}

void MainWindow::on_models_currentTextChanged(const QString &arg1)
{
    Model model = m_models[arg1];

    clearCurrentModelLayout();
    createModelLayout(model);
    updateModel();
}

void MainWindow::showEvent(QShowEvent *event)
{
    init();
}

void MainWindow::on_saveModel_clicked()
{
    QString currentModelName = ui->models->currentText();
    Model model = m_models[currentModelName];

    bool ok;
    QString modelName = QInputDialog::getText(this, tr("Save Model"),
                                         tr("Model name:"), QLineEdit::Normal,
                                         currentModelName, &ok);

    if(ok && !modelName.isEmpty())
        saveModel(model, modelName);
}

void MainWindow::on_loadModel_clicked()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    path += QDir::separator() + QString("models");

    QString fileName = QFileDialog::getOpenFileName(this, tr("Load Model"),
                                                    path,
                                                    tr("Model (*.rd)"));

    if(!fileName.isEmpty())
        loadModel(fileName);
}
