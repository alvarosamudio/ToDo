#ifndef TASK_H
#define TASK_H

#include <QDateTime>
#include <QString>
#include <memory>
#include <DWidget>

DWIDGET_USE_NAMESPACE

namespace Ui {
class Task;
}

class Task : public DWidget {
  Q_OBJECT

public:
  explicit Task(const QString &name, QWidget *parent = nullptr);
  Task(const QString &name, const QDateTime &createdAt, QWidget *parent = nullptr);
  Task(const QString &name, const QDateTime &createdAt, const QDateTime &reminderAt,
       bool notified = false, QWidget *parent = nullptr);
  ~Task();

  void setName(const QString &name);
  QString name() const;
  bool isCompleted() const;
  void setCompleted(bool completed);
  QDateTime createdAt() const;
  QString formatRelativeTime() const;
  void updateDateLabel();

  void setReminderAt(const QDateTime &dt);
  QDateTime reminderAt() const;
  bool hasReminder() const;
  bool isNotified() const;
  void setNotified(bool notified);
  void updateReminderLabel();

public slots:
  void rename();
  void checked(bool isChecked);
  void finishEdit();

signals:
  void removed(Task *task);
  void statusChanged(Task *task);

protected:
  void mouseDoubleClickEvent(QMouseEvent *event) override;
  bool eventFilter(QObject *obj, QEvent *event) override;

private:
  void init();
  std::unique_ptr<Ui::Task> ui;
  QDateTime m_createdAt;
  QDateTime m_reminderAt;
  bool m_notified = false;
};

#endif // TASK_H
