#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <DMainWindow>
#include <QDateTime>
#include <QTimer>
#include <memory>
#include <vector>
#include "Task.h"

DWIDGET_USE_NAMESPACE

namespace Ui {
class MainWindow;
}

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
  void updateStatus();
  void syncTasksFromList();
  void setupMenu();
  void exportTasks() const;
  void importTasks();
  void scheduleSave();
  void checkReminders();
  Task *createTask(const QString &name, const QDateTime &createdAt,
                   bool completed);
  Task *createTask(const QString &name, const QDateTime &createdAt,
                   bool completed, const QDateTime &reminderAt,
                   bool notified = false);
  void removeTaskFromList(Task *task);

  std::unique_ptr<Ui::MainWindow> ui;
  std::vector<std::unique_ptr<Task>> m_tasks;
  QTimer *m_saveTimer;
  QTimer *m_reminderTimer;
  std::unique_ptr<DeletedTaskInfo> m_lastDeleted;
};

#endif // MAINWINDOW_H
