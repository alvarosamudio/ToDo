#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QDebug>
#include <DInputDialog>

DWIDGET_USE_NAMESPACE


MainWindow::MainWindow(QWidget *parent)
    : DMainWindow{parent}, ui{new Ui::MainWindow}, m_tasks{} {
  ui->setupUi(this);
  connect(ui->addTaskButton, &QPushButton::clicked, this, &MainWindow::addTask);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::updateStatus() {
  int completeCount{};
  for (auto t : m_tasks) {
    if (t->isCompleted()) {
      ++completeCount;
    }
  }
  const auto todoCount = m_tasks.size() - completeCount;
  ui->statusLabel->setText(
      QString{"Estatus: %1 ToDo / %2 Finalizado"}.arg(todoCount).arg(
          completeCount));
}

void MainWindow::addTask() {
  bool isOk{};
  auto name =
      QInputDialog::getText(this, tr("Añadir tarea"), tr("Nombre de la tarea"),
                            QLineEdit::Normal, tr("Tarea sin título"), &isOk);
  if (isOk && !name.isEmpty()) {
    qDebug() << "Añadiendo tarea nueva ";
    auto task = new Task(name);
    connect(task, &Task::removed, this, &MainWindow::removeTask);
    connect(task, &Task::statusChanged, this, &MainWindow::taskStatusChanged);
    m_tasks.emplace_back(task);
    ui->tasksLayout->addWidget(task);
  }
}

void MainWindow::removeTask(Task *task) {
  if (auto item = std::find(m_tasks.begin(), m_tasks.end(), task);
      item != m_tasks.end())
    m_tasks.erase(item);
  ui->tasksLayout->removeWidget(task);
  delete task;
  updateStatus();
}

void MainWindow::taskStatusChanged(Task * /*task*/) { updateStatus(); }
