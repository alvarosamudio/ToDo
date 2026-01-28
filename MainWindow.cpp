#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QCloseEvent>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QIcon>
#include <QMenu>
#include <QMessageBox>
#include <QSettings>
#include <QShortcut>
#include <QListWidgetItem>
#include <DInputDialog>
#include <DTitlebar>

DWIDGET_USE_NAMESPACE

MainWindow::MainWindow(QWidget *parent)
    : DMainWindow{parent}, ui{new Ui::MainWindow} {
  ui->setupUi(this);

  connect(ui->addTaskButton, &QPushButton::clicked, this, &MainWindow::addTask);
  connect(ui->searchEdit, &QLineEdit::textChanged, this, &MainWindow::filterTasks);
  connect(ui->clearCompletedButton, &QPushButton::clicked, this, &MainWindow::clearCompleted);

  auto addShortcut = new QShortcut(QKeySequence("Ctrl+N"), this);
  connect(addShortcut, &QShortcut::activated, this, &MainWindow::addTask);

  connect(ui->taskList->model(), &QAbstractItemModel::rowsMoved,
          this, [this]() { syncTasksFromList(); saveTasks(); });

  setupMenu();
  loadTasks();
  updateStatus();
  showEmptyState();
}

MainWindow::~MainWindow() {
  for (const auto task : m_tasks) delete task;
  delete ui;
}

void MainWindow::setupMenu() {
  auto menu = new QMenu(this);
  auto exportAction = menu->addAction(QIcon::fromTheme("document-save-as"),
                                       tr("Exportar tareas..."));
  auto importAction = menu->addAction(QIcon::fromTheme("document-open"),
                                       tr("Importar tareas..."));
  connect(exportAction, &QAction::triggered, this, [this]() { exportTasks(); });
  connect(importAction, &QAction::triggered, this, [this]() { importTasks(); });

  if (auto tb = this->titlebar()) {
    tb->setMenu(menu);
  }
}

void MainWindow::exportTasks() const {
  const auto path = QFileDialog::getSaveFileName(
      nullptr, tr("Exportar tareas"), "tareas.json", tr("JSON (*.json)"));
  if (path.isEmpty()) return;

  QJsonArray arr;
  for (const auto &task : m_tasks) {
    QJsonObject obj;
    obj["name"] = task->name();
    obj["completed"] = task->isCompleted();
    arr.append(obj);
  }

  QJsonDocument doc(arr);
  QFile file(path);
  if (file.open(QIODevice::WriteOnly)) {
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
  }
}

void MainWindow::importTasks() {
  const auto path = QFileDialog::getOpenFileName(
      nullptr, tr("Importar tareas"), "", tr("JSON (*.json)"));
  if (path.isEmpty()) return;

  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) return;

  const auto doc = QJsonDocument::fromJson(file.readAll());
  file.close();

  const auto arr = doc.array();
  for (const auto &val : arr) {
    const auto obj = val.toObject();
    const auto name = obj["name"].toString();
    if (name.isEmpty()) continue;

    const auto task = new Task(name);
    connect(task, &Task::removed, this, &MainWindow::removeTask);
    connect(task, &Task::statusChanged, this, &MainWindow::taskStatusChanged);
    task->setCompleted(obj["completed"].toBool());

    const auto item = new QListWidgetItem();
    item->setSizeHint(QSize(0, 40));
    ui->taskList->addItem(item);
    ui->taskList->setItemWidget(item, task);
    m_tasks.push_back(task);
  }

  updateStatus();
  showEmptyState();
  saveTasks();
}

void MainWindow::closeEvent(QCloseEvent *event) {
  saveTasks();
  DMainWindow::closeEvent(event);
}

void MainWindow::syncTasksFromList() {
  std::vector<Task *> newOrder;
  for (int i = 0; i < ui->taskList->count(); ++i) {
    const auto item = ui->taskList->item(i);
    const auto task = static_cast<Task *>(ui->taskList->itemWidget(item));
    if (task) newOrder.push_back(task);
  }
  m_tasks = std::move(newOrder);
}

void MainWindow::updateStatus() {
  int completeCount{};
  for (const auto t : m_tasks) {
    if (t->isCompleted()) ++completeCount;
  }
  const auto total = static_cast<int>(m_tasks.size());
  const auto todoCount = total - completeCount;

  ui->statusLabel->setText(
      QString{"Estatus: %1 ToDo / %2 Finalizado"}.arg(todoCount).arg(completeCount));

  ui->progressBar->setValue(total > 0 ? (completeCount * 100 / total) : 0);
  ui->progressBar->setToolTip(
      QString{"%1% completado (%2/%3)"}.arg(
          total > 0 ? QString::number(completeCount * 100 / total) : "0")
          .arg(completeCount).arg(total));

  setWindowTitle(QString{"Deepines ToDo — %1 pendiente%2"}
                     .arg(todoCount)
                     .arg(todoCount == 1 ? "" : "s"));
}

void MainWindow::showEmptyState() {
  ui->emptyLabel->setVisible(m_tasks.empty());
}

void MainWindow::addTask() {
  bool isOk{};
  const auto name = QInputDialog::getText(
      this, tr("Añadir tarea"), tr("Nombre de la tarea"),
      QLineEdit::Normal, tr("Tarea sin título"), &isOk);
  if (isOk && !name.isEmpty()) {
    qDebug() << "Añadiendo tarea nueva";
    const auto task = new Task(name);
    connect(task, &Task::removed, this, &MainWindow::removeTask);
    connect(task, &Task::statusChanged, this, &MainWindow::taskStatusChanged);

    const auto item = new QListWidgetItem();
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
  const auto result = QMessageBox::question(
      this, tr("Eliminar tarea"),
      tr("¿Estás seguro de eliminar la tarea '%1'?").arg(task->name()),
      QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
  if (result != QMessageBox::Yes) return;

  for (int i = 0; i < ui->taskList->count(); ++i) {
    const auto item = ui->taskList->item(i);
    if (ui->taskList->itemWidget(item) == task) {
      delete ui->taskList->takeItem(i);
      break;
    }
  }

  if (const auto it = std::find(m_tasks.cbegin(), m_tasks.cend(), task);
      it != m_tasks.cend())
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
    const auto item = ui->taskList->item(i);
    const auto task = static_cast<Task *>(ui->taskList->itemWidget(item));
    item->setHidden(!text.isEmpty() &&
                    !task->name().contains(text, Qt::CaseInsensitive));
  }
}

void MainWindow::clearCompleted() {
  if (std::none_of(m_tasks.cbegin(), m_tasks.cend(),
                   [](const Task *t) { return t->isCompleted(); }))
    return;

  const auto result = QMessageBox::question(
      this, tr("Limpiar tareas"),
      tr("¿Eliminar todas las tareas finalizadas?"),
      QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
  if (result != QMessageBox::Yes) return;

  for (int i = ui->taskList->count() - 1; i >= 0; --i) {
    const auto item = ui->taskList->item(i);
    const auto task = static_cast<Task *>(ui->taskList->itemWidget(item));
    if (task->isCompleted()) {
      delete ui->taskList->takeItem(i);
      if (const auto it = std::find(m_tasks.cbegin(), m_tasks.cend(), task);
          it != m_tasks.cend())
        m_tasks.erase(it);
      task->deleteLater();
    }
  }
  updateStatus();
  showEmptyState();
  saveTasks();
}

void MainWindow::saveTasks() const {
  QSettings settings("deepines", "DeepinesToDo");
  settings.beginWriteArray("tasks");
  for (int i = 0; i < ui->taskList->count(); ++i) {
    const auto item = ui->taskList->item(i);
    const auto task = static_cast<Task *>(ui->taskList->itemWidget(item));
    if (!task) continue;
    settings.setArrayIndex(i);
    settings.setValue("name", task->name());
    settings.setValue("completed", task->isCompleted());
  }
  settings.endArray();
}

void MainWindow::loadTasks() {
  QSettings settings("deepines", "DeepinesToDo");
  const int size = settings.beginReadArray("tasks");
  for (int i = 0; i < size; ++i) {
    settings.setArrayIndex(i);
    const auto name = settings.value("name").toString();
    const auto completed = settings.value("completed").toBool();
    if (name.isEmpty()) continue;

    const auto task = new Task(name);
    connect(task, &Task::removed, this, &MainWindow::removeTask);
    connect(task, &Task::statusChanged, this, &MainWindow::taskStatusChanged);
    task->setCompleted(completed);

    const auto item = new QListWidgetItem();
    item->setSizeHint(QSize(0, 40));
    ui->taskList->addItem(item);
    ui->taskList->setItemWidget(item, task);
    m_tasks.push_back(task);
  }
  settings.endArray();
}
