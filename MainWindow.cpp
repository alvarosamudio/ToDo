#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QCloseEvent>
#include <QDebug>
#include <QMessageBox>
#include <QSettings>
#include <QShortcut>
#include <QListWidgetItem>
#include <DInputDialog>

DWIDGET_USE_NAMESPACE

MainWindow::MainWindow(QWidget *parent)
    : DMainWindow{parent}, ui{new Ui::MainWindow} {
  ui->setupUi(this);

  connect(ui->addTaskButton, &QPushButton::clicked, this, &MainWindow::addTask);
  connect(ui->searchEdit, &QLineEdit::textChanged, this, &MainWindow::filterTasks);
  connect(ui->clearCompletedButton, &QPushButton::clicked, this, &MainWindow::clearCompleted);

  auto shortcut = new QShortcut(QKeySequence("Ctrl+N"), this);
  connect(shortcut, &QShortcut::activated, this, &MainWindow::addTask);

  connect(ui->taskList->model(), &QAbstractItemModel::rowsMoved,
          this, [this]() { syncTasksFromList(); saveTasks(); });

  loadTasks();
  updateStatus();
  showEmptyState();
}

MainWindow::~MainWindow() {
  for (auto task : m_tasks) delete task;
  delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event) {
  saveTasks();
  DMainWindow::closeEvent(event);
}

void MainWindow::syncTasksFromList() {
  std::vector<Task *> newOrder;
  for (int i = 0; i < ui->taskList->count(); ++i) {
    auto item = ui->taskList->item(i);
    auto task = static_cast<Task *>(ui->taskList->itemWidget(item));
    if (task) newOrder.push_back(task);
  }
  m_tasks = std::move(newOrder);
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

void MainWindow::showEmptyState() {
  ui->emptyLabel->setVisible(m_tasks.empty());
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

    auto item = new QListWidgetItem();
    item->setSizeHint(QSize(0, 40));
    ui->taskList->addItem(item);
    ui->taskList->setItemWidget(item, task);
    m_tasks.emplace_back(task);
    updateStatus();
    showEmptyState();
    saveTasks();
  }
}

void MainWindow::removeTask(Task *task) {
  auto result = QMessageBox::question(
      this, tr("Eliminar tarea"),
      tr("¿Estás seguro de eliminar la tarea '%1'?").arg(task->name()),
      QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
  if (result != QMessageBox::Yes) return;

  for (int i = 0; i < ui->taskList->count(); ++i) {
    auto item = ui->taskList->item(i);
    if (ui->taskList->itemWidget(item) == task) {
      delete ui->taskList->takeItem(i);
      break;
    }
  }

  if (auto it = std::find(m_tasks.begin(), m_tasks.end(), task);
      it != m_tasks.end())
    m_tasks.erase(it);

  task->deleteLater();
  updateStatus();
  showEmptyState();
  saveTasks();
}

void MainWindow::taskStatusChanged(Task *) {
  updateStatus();
  saveTasks();
}

void MainWindow::filterTasks(const QString &text) {
  for (int i = 0; i < ui->taskList->count(); ++i) {
    auto item = ui->taskList->item(i);
    auto task = static_cast<Task *>(ui->taskList->itemWidget(item));
    if (text.isEmpty()) {
      item->setHidden(false);
    } else {
      item->setHidden(!task->name().contains(text, Qt::CaseInsensitive));
    }
  }
}

void MainWindow::clearCompleted() {
  if (std::none_of(m_tasks.begin(), m_tasks.end(),
                   [](Task *t) { return t->isCompleted(); }))
    return;

  auto result = QMessageBox::question(
      this, tr("Limpiar tareas"),
      tr("¿Eliminar todas las tareas finalizadas?"),
      QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
  if (result != QMessageBox::Yes) return;

  for (int i = ui->taskList->count() - 1; i >= 0; --i) {
    auto item = ui->taskList->item(i);
    auto task = static_cast<Task *>(ui->taskList->itemWidget(item));
    if (task->isCompleted()) {
      delete ui->taskList->takeItem(i);
      if (auto it = std::find(m_tasks.begin(), m_tasks.end(), task);
          it != m_tasks.end())
        m_tasks.erase(it);
      task->deleteLater();
    }
  }
  updateStatus();
  showEmptyState();
  saveTasks();
}

void MainWindow::saveTasks() {
  QSettings settings("deepines", "DeepinesToDo");
  settings.beginWriteArray("tasks");
  for (int i = 0; i < ui->taskList->count(); ++i) {
    auto item = ui->taskList->item(i);
    auto task = static_cast<Task *>(ui->taskList->itemWidget(item));
    if (!task) continue;
    settings.setArrayIndex(i);
    settings.setValue("name", task->name());
    settings.setValue("completed", task->isCompleted());
  }
  settings.endArray();
}

void MainWindow::loadTasks() {
  QSettings settings("deepines", "DeepinesToDo");
  int size = settings.beginReadArray("tasks");
  for (int i = 0; i < size; ++i) {
    settings.setArrayIndex(i);
    auto name = settings.value("name").toString();
    auto completed = settings.value("completed").toBool();

    auto task = new Task(name);
    connect(task, &Task::removed, this, &MainWindow::removeTask);
    connect(task, &Task::statusChanged, this, &MainWindow::taskStatusChanged);
    task->setCompleted(completed);

    auto item = new QListWidgetItem();
    item->setSizeHint(QSize(0, 40));
    ui->taskList->addItem(item);
    ui->taskList->setItemWidget(item, task);
    m_tasks.push_back(task);
  }
  settings.endArray();
}
