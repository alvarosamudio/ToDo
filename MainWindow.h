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
  void syncTasksFromList();

public slots:
  void addTask();
  void removeTask(Task *task);
  void taskStatusChanged(Task *task);
  void filterTasks(const QString &text);
  void clearCompleted();

protected:
  void closeEvent(QCloseEvent *event) override;

private:
  void saveTasks();
  void loadTasks();
  void showEmptyState();

  Ui::MainWindow *ui;
  std::vector<Task *> m_tasks;
};

#endif // MAINWINDOW_H
