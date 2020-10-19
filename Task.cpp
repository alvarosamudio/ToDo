#include "Task.h"
#include "ui_Task.h"

#include <DInputDialog>

Task::Task(const QString &name, QWidget *parent)
    : QWidget{parent}, ui{new Ui::Task} {
  ui->setupUi(this);
  setName(name);
  connect(ui->editButton, &QPushButton::clicked, this, &Task::rename);
  connect(ui->removeButton, &QPushButton::clicked,
          [this]() { emit removed(this); });
  connect(ui->checkBox, &QCheckBox::toggled, this, &Task::checked);
}

Task::~Task() { delete ui; }

void Task::setName(const QString &name) { ui->checkBox->setText(name); }

QString Task::name() const { return ui->checkBox->text(); }

bool Task::isCompleted() const { return ui->checkBox->isChecked(); }

void Task::rename() {
  bool isOk{};
  auto value = QInputDialog::getText(this, tr("Editar tarea"), tr("Nombre de tarea"),
                                     QLineEdit::Normal, this->name(), &isOk);
  if (isOk && !value.isEmpty())
    setName(value);
}

void Task::checked(bool isChecked) {
  auto font(ui->checkBox->font());
  font.setStrikeOut(isChecked);
  ui->checkBox->setFont(font);
  emit statusChanged(this);
}
