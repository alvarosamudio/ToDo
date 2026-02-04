#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <DMainWindow>
#include <QDateTime>
#include <QTimer>
#include <vector>
#include "Task.h"

DWIDGET_USE_NAMESPACE

namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

struct DeletedTaskInfo {
  QString name;
  bool completed;
  QDateTime createdAt;
};

class MainWindow : public DMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

  void updateStatus();
  void syncTasksFromList();

public slots:
  void addTask();
  void removeTask(Task *task);
  void taskStatusChanged(Task *task);
  void filterTasks(const QString &text);
  void clearCompleted();
  void showStats();
  void undoDelete();

protected:
  void closeEvent(QCloseEvent *event) override;

private:
  void saveTasks() const;
  void loadTasks();
  void showEmptyState();
  void setupMenu();
  void exportTasks() const;
  void importTasks();
  void scheduleSave();
  Task *createTask(const QString &name, const QDateTime &createdAt,
                   bool completed);

  Ui::MainWindow *ui;
  std::vector<Task *> m_tasks;
  QTimer *m_saveTimer;
  std::unique_ptr<DeletedTaskInfo> m_lastDeleted;
};

#endif // MAINWINDOW_H
