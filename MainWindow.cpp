#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QCloseEvent>
#include <QDebug>
#include <QSettings>
#include <QShortcut>
#include <DInputDialog>

DWIDGET_USE_NAMESPACE

MainWindow::MainWindow(QWidget *parent)
    : DMainWindow{parent}, ui{new Ui::MainWindow}, m_tasks{} {
  ui->setupUi(this);

  connect(ui->addTaskButton, &QPushButton::clicked, this, &MainWindow::addTask);

  auto shortcut = new QShortcut(QKeySequence("Ctrl+N"), this);
  connect(shortcut, &QShortcut::activated, this, &MainWindow::addTask);

  loadTasks();
  updateStatus();
}

MainWindow::~MainWindow() {
  for (auto task : m_tasks) delete task;
  delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event) {
  saveTasks();
  DMainWindow::closeEvent(event);
}

void MainWindow::updateStatus() {
  int completeCount{};
  for (auto t : m_tasks) {
    if (t->isCompleted()) ++completeCount;
  }
  const auto todoCount = m_tasks.size() - completeCount;
  ui->statusLabel->setText(
      QString{"Estatus: %1 ToDo / %2 Finalizado"}.arg(todoCount).arg(completeCount));
}

void MainWindow::addTask() {
  bool isOk{};
  auto name = QInputDialog::getText(this, tr("Añadir tarea"), tr("Nombre de la tarea"),
                                    QLineEdit::Normal, tr("Tarea sin título"), &isOk);
  if (isOk && !name.isEmpty()) {
    qDebug() << "Añadiendo tarea nueva";
    auto task = new Task(name);
    connect(task, &Task::removed, this, &MainWindow::removeTask);
    connect(task, &Task::statusChanged, this, &MainWindow::taskStatusChanged);
    m_tasks.emplace_back(task);
    ui->tasksLayout->addWidget(task);
    updateStatus();
    saveTasks();
  }
}

void MainWindow::removeTask(Task *task) {
  if (auto item = std::find(m_tasks.begin(), m_tasks.end(), task);
      item != m_tasks.end())
    m_tasks.erase(item);
  ui->tasksLayout->removeWidget(task);
  task->deleteLater();
  updateStatus();
  saveTasks();
}

void MainWindow::taskStatusChanged(Task *) {
  updateStatus();
  saveTasks();
}

void MainWindow::saveTasks() {
  QSettings settings("deepines", "DeepinesToDo");
  settings.beginWriteArray("tasks");
  for (size_t i = 0; i < m_tasks.size(); ++i) {
    settings.setArrayIndex(static_cast<int>(i));
    settings.setValue("name", m_tasks[i]->name());
    settings.setValue("completed", m_tasks[i]->isCompleted());
  }
  settings.endArray();
}

void MainWindow::loadTasks() {
  QSettings settings("deepines", "DeepinesToDo");
  auto size = settings.beginReadArray("tasks");
  for (int i = 0; i < size; ++i) {
    settings.setArrayIndex(i);
    auto name = settings.value("name").toString();
    auto completed = settings.value("completed").toBool();

    auto task = new Task(name);
    connect(task, &Task::removed, this, &MainWindow::removeTask);
    connect(task, &Task::statusChanged, this, &MainWindow::taskStatusChanged);
    task->setCompleted(completed);
    m_tasks.push_back(task);
    ui->tasksLayout->addWidget(task);
  }
  settings.endArray();
}
