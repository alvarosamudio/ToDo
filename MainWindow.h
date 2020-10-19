#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <DMainWindow>
#include <vector>
#include "Task.h"

DWIDGET_USE_NAMESPACE

namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public DMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

  void updateStatus();

public slots:
  void addTask();
  void removeTask(Task *task);
  void taskStatusChanged(Task *task);

private:
  Ui::MainWindow *ui;
  std::vector<Task *> m_tasks;
};
#endif // MAINWINDOW_H
