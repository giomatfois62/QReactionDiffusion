#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFormLayout>
#include <QDoubleSpinBox>

#include "solver.h"

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
    void on_models_currentTextChanged(const QString &arg1);
    void updateModel();

private:
    void init();
    void loadModels();
    void setModel();

    Ui::MainWindow *ui;
    QFormLayout *layout;

    QMap<QString, QDoubleSpinBox*> params;
    QHash<QString, Model> m_models;
};

#endif // MAINWINDOW_H
