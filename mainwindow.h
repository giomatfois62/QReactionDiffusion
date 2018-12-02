#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

#include "solver.h"

#include <vector>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_reset_clicked();
    void on_start_clicked();
    void on_save_clicked();
    void on_stop_clicked();
    void on_gridSize_editingFinished();

    void render();

private:
    void init();
    void draw();

    Ui::MainWindow *ui;
    QPixmap pixmap;
    Solver solver;

    int currentIteration;
    bool isActive;
};

#endif // MAINWINDOW_H
