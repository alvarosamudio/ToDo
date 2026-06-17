#include "Task.h"
#include "ui_Task.h"

#include <QKeyEvent>
#include <QIcon>

Task::Task(const QString &name, QWidget *parent)
    : DWidget{parent}, ui{std::make_unique<Ui::Task>()}, m_createdAt{QDateTime::currentDateTime()} {
  init();
  setName(name);
}

Task::Task(const QString &name, const QDateTime &createdAt, QWidget *parent)
    : DWidget{parent}, ui{std::make_unique<Ui::Task>()}, m_createdAt{createdAt} {
  init();
  setName(name);
}

Task::Task(const QString &name, const QDateTime &createdAt, const QDateTime &reminderAt,
           bool notified, QWidget *parent)
    : DWidget{parent}, ui{std::make_unique<Ui::Task>()}, m_createdAt{createdAt},
      m_reminderAt{reminderAt}, m_notified{notified} {
  init();
  setName(name);
  updateReminderLabel();
}

void Task::init() {
  ui->setupUi(this);

  ui->editButton->setIcon(QIcon::fromTheme("document-edit"));
  ui->removeButton->setIcon(QIcon::fromTheme("edit-delete"));

  auto datePalette = ui->dateLabel->palette();
  datePalette.setColor(QPalette::WindowText, datePalette.color(QPalette::PlaceholderText));
  ui->dateLabel->setPalette(datePalette);

  connect(ui->editButton, &QPushButton::clicked, this, &Task::rename);
  connect(ui->removeButton, &QPushButton::clicked,
          [this]() { emit removed(this); });
  connect(ui->checkBox, &QCheckBox::toggled, this, &Task::checked);
  connect(ui->editBox, &QLineEdit::returnPressed, this, &Task::finishEdit);
  ui->editBox->installEventFilter(this);
}

Task::~Task() = default;

void Task::setName(const QString &name) {
  ui->checkBox->setText(name);
  ui->editBox->setText(name);
}

QString Task::name() const { return ui->checkBox->text(); }

bool Task::isCompleted() const { return ui->checkBox->isChecked(); }

QDateTime Task::createdAt() const { return m_createdAt; }

QString Task::formatRelativeTime() const {
  const auto now = QDateTime::currentDateTime();
  const auto secs = m_createdAt.secsTo(now);
  if (secs < 0) return tr("ahora");
  if (secs < 60) return tr("ahora");
  if (secs < 3600) {
    const int mins = secs / 60;
    return tr("hace %1 min").arg(mins);
  }
  if (secs < 86400) {
    const int hours = secs / 3600;
    return tr("hace %1 h").arg(hours);
  }
  const int days = secs / 86400;
  if (days == 1) return tr("ayer");
  if (days < 30) return tr("hace %1 d").arg(days);
  if (days < 365) {
    const int months = days / 30;
    return tr("hace %1 %2").arg(months).arg(months == 1 ? tr("mes") : tr("meses"));
  }
  const int years = days / 365;
  return tr("hace %1 %2").arg(years).arg(years == 1 ? tr("año") : tr("años"));
}

void Task::setCompleted(bool completed) {
  const auto wasBlocked = ui->checkBox->blockSignals(true);
  ui->checkBox->setChecked(completed);
  ui->checkBox->blockSignals(wasBlocked);
  auto font(ui->checkBox->font());
  font.setStrikeOut(completed);
  ui->checkBox->setFont(font);
}

void Task::updateDateLabel() {
  ui->dateLabel->setText(formatRelativeTime());
}

void Task::setReminderAt(const QDateTime &dt) {
  m_reminderAt = dt;
  if (dt.isValid()) m_notified = false;
  updateReminderLabel();
}

QDateTime Task::reminderAt() const { return m_reminderAt; }

bool Task::hasReminder() const { return m_reminderAt.isValid(); }

bool Task::isNotified() const { return m_notified; }

void Task::setNotified(bool notified) { m_notified = notified; }

void Task::updateReminderLabel() {
  if (m_reminderAt.isValid()) {
    ui->reminderLabel->setVisible(true);
    const auto now = QDateTime::currentDateTime();
    const auto secs = now.secsTo(m_reminderAt);
    if (secs < 0) {
      ui->reminderLabel->setText(tr("⏰ vencido"));
    } else if (secs < 60) {
      ui->reminderLabel->setText(tr("⏰ ahora"));
    } else if (secs < 3600) {
      ui->reminderLabel->setText(tr("⏰ en %1 min").arg(secs / 60));
    } else if (secs < 86400) {
      ui->reminderLabel->setText(tr("⏰ en %1 h").arg(secs / 3600));
    } else {
      ui->reminderLabel->setText(tr("⏰ en %1 d").arg(secs / 86400));
    }
  } else {
    ui->reminderLabel->setVisible(false);
  }
}

void Task::rename() {
  ui->stackedWidget->setCurrentIndex(1);
  ui->editBox->setText(name());
  ui->editBox->selectAll();
  ui->editBox->setFocus();
}

void Task::finishEdit() {
  if (!ui->editBox->text().isEmpty()) {
    setName(ui->editBox->text());
  }
  ui->stackedWidget->setCurrentIndex(0);
}

void Task::checked(bool isChecked) {
  auto font(ui->checkBox->font());
  font.setStrikeOut(isChecked);
  ui->checkBox->setFont(font);
  emit statusChanged(this);
}

void Task::mouseDoubleClickEvent(QMouseEvent *event) {
  if (ui->stackedWidget->currentIndex() == 0) {
    rename();
  }
  QWidget::mouseDoubleClickEvent(event);
}

bool Task::eventFilter(QObject *obj, QEvent *event) {
  if (obj == ui->editBox && event->type() == QEvent::KeyPress) {
    const auto keyEvent = static_cast<QKeyEvent *>(event);
    if (keyEvent->key() == Qt::Key_Escape) {
      ui->stackedWidget->setCurrentIndex(0);
      return true;
    }
  }
  return QWidget::eventFilter(obj, event);
}
