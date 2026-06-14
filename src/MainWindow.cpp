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
#include <QInputDialog>
#include <QSettings>
#include <QShortcut>
#include <QListWidgetItem>
#include <DDialog>
#include <DTitlebar>

DWIDGET_USE_NAMESPACE

MainWindow::MainWindow(QWidget *parent)
    : DMainWindow{parent}, ui{std::make_unique<Ui::MainWindow>()} {
  ui->setupUi(this);

  m_saveTimer = new QTimer(this);
  m_saveTimer->setSingleShot(true);
  m_saveTimer->setInterval(1500);
  connect(m_saveTimer, &QTimer::timeout, this, [this]() { saveTasks(); });

  connect(ui->addTaskButton, &QPushButton::clicked, this, &MainWindow::addTask);
  connect(ui->searchEdit, &QLineEdit::textChanged, this, &MainWindow::filterTasks);
  connect(ui->clearCompletedButton, &QPushButton::clicked, this, &MainWindow::clearCompleted);
  connect(ui->statsButton, &QPushButton::clicked, this, &MainWindow::showStats);
  connect(ui->undoButton, &QPushButton::clicked, this, &MainWindow::undoDelete);

  auto addShortcut = new QShortcut(QKeySequence("Ctrl+N"), this);
  connect(addShortcut, &QShortcut::activated, this, &MainWindow::addTask);

  auto clearShortcut = new QShortcut(QKeySequence("Ctrl+Delete"), this);
  connect(clearShortcut, &QShortcut::activated, this, &MainWindow::clearCompleted);

  auto escapeShortcut = new QShortcut(QKeySequence("Escape"), this);
  connect(escapeShortcut, &QShortcut::activated, this, [this]() {
    ui->searchEdit->clear();
    ui->searchEdit->clearFocus();
  });

  connect(ui->taskList->model(), &QAbstractItemModel::rowsMoved,
          this, [this]() { syncTasksFromList(); scheduleSave(); });

  setupMenu();
  loadTasks();
  updateStatus();
  showEmptyState();
}

MainWindow::~MainWindow() {
  for (const auto task : m_tasks) delete task;
}

void MainWindow::setupMenu() {
  auto menu = new QMenu(this);
  auto exportAction = menu->addAction(QIcon::fromTheme("document-save-as"),
                                       tr("Exportar tareas..."));
  auto importAction = menu->addAction(QIcon::fromTheme("document-open"),
                                       tr("Importar tareas..."));
  auto statsAction = menu->addAction(QIcon::fromTheme("office-chart-line"),
                                      tr("Estadísticas..."));
  connect(exportAction, &QAction::triggered, this, [this]() { exportTasks(); });
  connect(importAction, &QAction::triggered, this, [this]() { importTasks(); });
  connect(statsAction, &QAction::triggered, this, &MainWindow::showStats);

  auto exportShortcut = new QShortcut(QKeySequence("Ctrl+E"), this);
  connect(exportShortcut, &QShortcut::activated, this, [this]() { exportTasks(); });

  auto importShortcut = new QShortcut(QKeySequence("Ctrl+I"), this);
  connect(importShortcut, &QShortcut::activated, this, [this]() { importTasks(); });

  auto statsShortcut = new QShortcut(QKeySequence("Ctrl+T"), this);
  connect(statsShortcut, &QShortcut::activated, this, &MainWindow::showStats);

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
    obj["createdAt"] = task->createdAt().toString(Qt::ISODate);
    arr.append(obj);
  }

  QJsonDocument doc(arr);
  QFile file(path);
  if (file.open(QIODevice::WriteOnly)) {
    file.write(doc.toJson(QJsonDocument::Indented));
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

    auto createdAt = QDateTime::fromString(obj["createdAt"].toString(), Qt::ISODate);
    if (!createdAt.isValid()) createdAt = QDateTime::currentDateTime();

    createTask(name, createdAt, obj["completed"].toBool());
  }

  updateStatus();
  showEmptyState();
  scheduleSave();
}

Task *MainWindow::createTask(const QString &name, const QDateTime &createdAt,
                              bool completed) {
  const auto task = new Task(name, createdAt);
  connect(task, &Task::removed, this, &MainWindow::removeTask);
  connect(task, &Task::statusChanged, this, &MainWindow::taskStatusChanged);

  const auto item = new QListWidgetItem();
  item->setSizeHint(QSize(0, 40));
  ui->taskList->addItem(item);
  ui->taskList->setItemWidget(item, task);
  task->setCompleted(completed);
  m_tasks.push_back(task);
  return task;
}

void MainWindow::closeEvent(QCloseEvent *event) {
  m_saveTimer->stop();
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
      QString{"%1% completado (%2/%3)"}
          .arg(total > 0 ? QString::number(completeCount * 100 / total) : "0")
          .arg(completeCount)
          .arg(total));

  setWindowTitle(QString{"ToDo — %1 pendiente%2"}
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
    createTask(name, QDateTime::currentDateTime(), false);
    updateStatus();
    showEmptyState();
    scheduleSave();
  }
}

void MainWindow::removeTask(Task *task) {
  const auto result = QMessageBox::question(
      this, tr("Eliminar tarea"),
      tr("¿Estás seguro de eliminar la tarea '%1'?").arg(task->name()),
      QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
  if (result != QMessageBox::Yes) return;

  m_lastDeleted = std::make_unique<DeletedTaskInfo>(
      DeletedTaskInfo{task->name(), task->isCompleted(), task->createdAt()});

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
  scheduleSave();

  ui->undoButton->setVisible(true);
  QTimer::singleShot(5000, this, [this]() {
    if (ui->undoButton->isVisible()) {
      ui->undoButton->setVisible(false);
      m_lastDeleted.reset();
    }
  });
}

void MainWindow::undoDelete() {
  if (!m_lastDeleted) return;

  createTask(m_lastDeleted->name, m_lastDeleted->createdAt,
             m_lastDeleted->completed);
  m_lastDeleted.reset();
  ui->undoButton->setVisible(false);
  updateStatus();
  showEmptyState();
  scheduleSave();
}

void MainWindow::taskStatusChanged(Task *) {
  updateStatus();
  scheduleSave();
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
  scheduleSave();
}

void MainWindow::showStats() {
  int total = static_cast<int>(m_tasks.size());
  int completed{};
  int today{};
  const auto todayStart = QDateTime::currentDateTime().date().startOfDay();

  for (const auto &t : m_tasks) {
    if (t->isCompleted()) ++completed;
    if (t->createdAt() >= todayStart) ++today;
  }
  const auto pending = total - completed;

  auto dialog = new DDialog(this);
  dialog->setWindowTitle(tr("Estadísticas"));
  dialog->setIcon(QIcon::fromTheme("office-chart-line"));

  QString msg;
  msg += tr("Total de tareas: %1\n").arg(total);
  msg += tr("Pendientes: %1\n").arg(pending);
  msg += tr("Finalizadas: %1\n").arg(completed);
  if (total > 0) {
    msg += tr("Completadas: %1%\n").arg(completed * 100 / total);
  }
  msg += tr("Creadas hoy: %1").arg(today);

  dialog->setMessage(msg);
  dialog->addButton(tr("Cerrar"));
  dialog->exec();
  dialog->deleteLater();
}

void MainWindow::scheduleSave() {
  m_saveTimer->start();
}

void MainWindow::saveTasks() const {
  QSettings settings("deepines", "ToDo");
  settings.beginWriteArray("tasks");
  for (int i = 0; i < ui->taskList->count(); ++i) {
    const auto item = ui->taskList->item(i);
    const auto task = static_cast<Task *>(ui->taskList->itemWidget(item));
    if (!task) continue;
    settings.setArrayIndex(i);
    settings.setValue("name", task->name());
    settings.setValue("completed", task->isCompleted());
    settings.setValue("createdAt", task->createdAt().toString(Qt::ISODate));
  }
  settings.endArray();
}

void MainWindow::loadTasks() {
  QSettings settings("deepines", "ToDo");
  const int size = settings.beginReadArray("tasks");
  for (int i = 0; i < size; ++i) {
    settings.setArrayIndex(i);
    const auto name = settings.value("name").toString();
    const auto completed = settings.value("completed").toBool();
    const auto createdAt = QDateTime::fromString(
        settings.value("createdAt").toString(), Qt::ISODate);
    if (name.isEmpty()) continue;

    createTask(name, createdAt.isValid() ? createdAt : QDateTime::currentDateTime(),
               completed);
  }
  settings.endArray();
}
