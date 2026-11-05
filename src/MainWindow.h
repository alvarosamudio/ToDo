#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <DMainWindow>
#include <QDateTime>
#include <QTimer>
#include <QListWidgetItem>
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
  QDateTime reminderAt;
  bool notified;
  int row;
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
  void editReminder(Task *task);

protected:
  void closeEvent(QCloseEvent *event) override;

private:
  void saveTasks() const;
  void loadTasks();
  void showEmptyState();
  void updateStatus();
  void syncTasksFromList();
  void setupMenu();
  void exportTasks();
  void importTasks();
  void scheduleSave();
  void checkReminders();
  void refreshTimeLabels();
  void showMessage(const QString &title, const QString &message,
                   const QString &icon);
  Task *createTask(const QString &name, const QDateTime &createdAt,
                   bool completed, const QDateTime &reminderAt = QDateTime(),
                   bool notified = false, int row = -1);
  void applyFilter(QListWidgetItem *item, Task *task) const;
  int removeTaskFromList(Task *task);

  std::unique_ptr<Ui::MainWindow> ui;
  std::vector<std::unique_ptr<Task>> m_tasks;
  QTimer *m_saveTimer;
  QTimer *m_reminderTimer;
  QTimer *m_undoTimer;
  std::unique_ptr<DeletedTaskInfo> m_lastDeleted;
};

#endif // MAINWINDOW_H
