#ifndef TASK_H
#define TASK_H

#include <QDateTime>
#include <QString>
#include <DWidget>

namespace Ui {
class Task;
}

class Task : public QWidget {
  Q_OBJECT

public:
  explicit Task(const QString &name, QWidget *parent = nullptr);
  Task(const QString &name, const QDateTime &createdAt, QWidget *parent = nullptr);
  ~Task();

  void setName(const QString &name);
  QString name() const;
  bool isCompleted() const;
  void setCompleted(bool completed);
  QDateTime createdAt() const;
  QString formatRelativeTime() const;

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
  Ui::Task *ui;
  QDateTime m_createdAt;
};

#endif // TASK_H
