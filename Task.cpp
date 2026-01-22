#include "Task.h"
#include "ui_Task.h"

#include <QKeyEvent>

Task::Task(const QString &name, QWidget *parent)
    : QWidget{parent}, ui{new Ui::Task} {
  ui->setupUi(this);
  setName(name);
  connect(ui->editButton, &QPushButton::clicked, this, &Task::rename);
  connect(ui->removeButton, &QPushButton::clicked,
          [this]() { emit removed(this); });
  connect(ui->checkBox, &QCheckBox::toggled, this, &Task::checked);
  connect(ui->editBox, &QLineEdit::returnPressed, this, &Task::finishEdit);
  connect(ui->editBox, &QLineEdit::editingFinished, this, &Task::finishEdit);
  ui->editBox->installEventFilter(this);
}

Task::~Task() { delete ui; }

void Task::setName(const QString &name) {
  ui->checkBox->setText(name);
  ui->editBox->setText(name);
}

QString Task::name() const { return ui->checkBox->text(); }

bool Task::isCompleted() const { return ui->checkBox->isChecked(); }

void Task::setCompleted(bool completed) {
  const auto wasBlocked = ui->checkBox->blockSignals(true);
  ui->checkBox->setChecked(completed);
  ui->checkBox->blockSignals(wasBlocked);
  auto font(ui->checkBox->font());
  font.setStrikeOut(completed);
  ui->checkBox->setFont(font);
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
    auto keyEvent = static_cast<QKeyEvent *>(event);
    if (keyEvent->key() == Qt::Key_Escape) {
      ui->stackedWidget->setCurrentIndex(0);
      return true;
    }
  }
  return QWidget::eventFilter(obj, event);
}
