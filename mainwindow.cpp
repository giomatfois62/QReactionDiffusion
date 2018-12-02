#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "solver.h"

#include <QGraphicsPixmapItem>
#include <QPainter>
#include <QFileDialog>
#include <QMessageBox>

#include <cmath>

using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    currentIteration(0),
    isActive(0)
{
    ui->setupUi(this);

    QTimer::singleShot(0, this, &MainWindow::on_reset_clicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_reset_clicked()
{
    isActive = false;
    init();
    draw();
}

void MainWindow::on_start_clicked()
{
    isActive = true;
    QTimer::singleShot(0, this, &MainWindow::render);
}

void MainWindow::on_stop_clicked()
{
    isActive = false;
    draw();
}

void MainWindow::on_gridSize_editingFinished()
{
    init();
    draw();
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

    pixmap.save(&file, "PNG");
}

void MainWindow::init()
{
    int size = ui->gridSize->value();
    solver.setSize(size);
    pixmap = QPixmap(size, size);

    solver.b = ui->b->value();
    solver.d = ui->d->value();
    solver.dt = ui->dt->value();
    solver.du = ui->du->value();
    solver.dv = ui->dv->value();

    int iterations = ui->iterations->value();
    currentIteration = 0;
    ui->progress->setMinimum(currentIteration);
    ui->progress->setMaximum(iterations);
    ui->progress->setValue(currentIteration);
}

void MainWindow::draw()
{
    int size = solver.u0.size();

    QPainter painter(&pixmap);
    for(int i = 0; i  < size; i++)
    {
        for(int j = 0; j < size; j++)
        {
            double val = (solver.u0[i][j] - solver.minu) / (solver.maxu - solver.minu);
            int b = 255 * val;
            int g = 255 * (1 - val);

            QColor color = QColor(0, g , b);
            painter.setPen(color);

            painter.drawPoint(i, j);
        }
    }

    ui->glWidget->setTexture(pixmap.toImage());
    ui->glWidget->update();
}

void MainWindow::render()
{
    int iterations = ui->iterations->value();

    if(!isActive || currentIteration >= iterations)
        return;

    solver.solve();
    draw();

    currentIteration++;
    ui->progress->setValue(currentIteration);
    if(currentIteration < iterations)
        QTimer::singleShot(1, this, &MainWindow::render);
}
