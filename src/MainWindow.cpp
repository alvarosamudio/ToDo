#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "NotificationManager.h"
#include <QCloseEvent>
#include <algorithm>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QIcon>
#include <QMenu>
#include <QLineEdit>
#include <QCheckBox>
#include <QDateTimeEdit>
#include <QSettings>
#include <QShortcut>
#include <QListWidgetItem>
#include <DDialog>
#include <DTitlebar>

DWIDGET_USE_NAMESPACE

MainWindow::MainWindow(QWidget *parent)
    : DMainWindow{parent}, ui{std::make_unique<Ui::MainWindow>()} {
  ui->setupUi(this);

  ui->searchEdit->addAction(QIcon::fromTheme("edit-find"), QLineEdit::LeadingPosition);

  ui->statsButton->setIcon(QIcon::fromTheme("office-chart-line"));
  ui->clearCompletedButton->setIcon(QIcon::fromTheme("edit-clear-list"));
  ui->addTaskButton->setIcon(QIcon::fromTheme("list-add"));

  m_saveTimer = new QTimer(this);
  m_saveTimer->setSingleShot(true);
  m_saveTimer->setInterval(1500);
  connect(m_saveTimer, &QTimer::timeout, this, [this]() { saveTasks(); });

  m_reminderTimer = new QTimer(this);
  m_reminderTimer->setInterval(30000);
  connect(m_reminderTimer, &QTimer::timeout, this, [this]() {
    checkReminders();
    refreshTimeLabels();
  });
  m_reminderTimer->start();

  m_undoTimer = new QTimer(this);
  m_undoTimer->setSingleShot(true);
  m_undoTimer->setInterval(5000);
  connect(m_undoTimer, &QTimer::timeout, this, [this]() {
    ui->undoButton->setVisible(false);
    m_lastDeleted.reset();
  });

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
  // Avisar de recordatorios que vencieron mientras la app estaba cerrada.
  QTimer::singleShot(0, this, &MainWindow::checkReminders);
}

void MainWindow::showMessage(const QString &title, const QString &message,
                             const QString &icon) {
  DDialog dialog(this);
  dialog.setTitle(title);
  dialog.setIcon(QIcon::fromTheme(icon));
  dialog.setMessage(message);
  dialog.addButton(tr("Cerrar"));
  dialog.exec();
}

MainWindow::~MainWindow() = default;

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

void MainWindow::exportTasks() {
  const auto path = QFileDialog::getSaveFileName(
      this, tr("Exportar tareas"), "tareas.json", tr("JSON (*.json)"));
  if (path.isEmpty()) return;

  QJsonArray arr;
  for (const auto &task : m_tasks) {
    QJsonObject obj;
    obj["name"] = task->name();
    obj["completed"] = task->isCompleted();
    obj["createdAt"] = task->createdAt().toString(Qt::ISODate);
    if (task->hasReminder()) {
      obj["reminderAt"] = task->reminderAt().toString(Qt::ISODate);
    }
    obj["notified"] = task->isNotified();
    arr.append(obj);
  }

  QJsonDocument doc(arr);
  QFile file(path);
  if (!file.open(QIODevice::WriteOnly) ||
      file.write(doc.toJson(QJsonDocument::Indented)) < 0) {
    showMessage(tr("Error"), tr("No se pudo guardar el archivo."), "dialog-error");
  }
}

void MainWindow::importTasks() {
  const auto path = QFileDialog::getOpenFileName(
      this, tr("Importar tareas"), "", tr("JSON (*.json)"));
  if (path.isEmpty()) return;

  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) {
    showMessage(tr("Error"), tr("No se pudo abrir el archivo."), "dialog-error");
    return;
  }

  const auto doc = QJsonDocument::fromJson(file.readAll());
  file.close();

  if (doc.isNull() || !doc.isArray()) {
    showMessage(tr("Error"), tr("El archivo no contiene un JSON válido."),
                "dialog-error");
    return;
  }

  const auto arr = doc.array();
  int imported = 0;
  for (const auto &val : arr) {
    if (!val.isObject()) continue;
    const auto obj = val.toObject();
    const auto name = obj["name"].toString().trimmed();
    if (name.isEmpty()) continue;

    auto createdAt = QDateTime::fromString(obj["createdAt"].toString(), Qt::ISODate);
    if (!createdAt.isValid()) createdAt = QDateTime::currentDateTime();

    auto reminderAt = QDateTime::fromString(obj["reminderAt"].toString(), Qt::ISODate);
    const auto notified = obj["notified"].toBool();

    createTask(name, createdAt, obj["completed"].toBool(), reminderAt, notified);
    ++imported;
  }

  updateStatus();
  showEmptyState();
  scheduleSave();

  if (imported > 0) {
    showMessage(tr("Importar tareas"), tr("Se importaron %1 tareas.").arg(imported),
                "dialog-information");
  }
}

Task *MainWindow::createTask(const QString &name, const QDateTime &createdAt,
                              bool completed, const QDateTime &reminderAt,
                              bool notified, int row) {
  auto task = std::make_unique<Task>(name, createdAt, reminderAt, notified);
  Task *taskPtr = task.get();
  connect(taskPtr, &Task::removed, this, &MainWindow::removeTask);
  connect(taskPtr, &Task::statusChanged, this, &MainWindow::taskStatusChanged);
  connect(taskPtr, &Task::reminderRequested, this, &MainWindow::editReminder);
  connect(taskPtr, &Task::renamed, this, [this](Task *t) {
    for (int i = 0; i < ui->taskList->count(); ++i) {
      const auto item = ui->taskList->item(i);
      if (ui->taskList->itemWidget(item) == t) applyFilter(item, t);
    }
    scheduleSave();
  });

  if (row < 0 || row > ui->taskList->count()) row = ui->taskList->count();
  const auto item = new QListWidgetItem();
  item->setSizeHint(QSize(0, 40));
  ui->taskList->insertItem(row, item);
  ui->taskList->setItemWidget(item, taskPtr);
  taskPtr->setCompleted(completed);
  taskPtr->updateDateLabel();
  taskPtr->updateReminderLabel();
  applyFilter(item, taskPtr);
  m_tasks.insert(m_tasks.begin() + std::min<size_t>(row, m_tasks.size()),
                 std::move(task));
  return taskPtr;
}

void MainWindow::applyFilter(QListWidgetItem *item, Task *task) const {
  const auto text = ui->searchEdit->text();
  item->setHidden(!text.isEmpty() &&
                  !task->name().contains(text, Qt::CaseInsensitive));
}

void MainWindow::refreshTimeLabels() {
  for (const auto &task : m_tasks) {
    task->updateDateLabel();
    task->updateReminderLabel();
  }
}

void MainWindow::closeEvent(QCloseEvent *event) {
  m_saveTimer->stop();
  m_reminderTimer->stop();
  saveTasks();
  DMainWindow::closeEvent(event);
}

void MainWindow::syncTasksFromList() {
  std::vector<Task *> ordered;
  for (int i = 0; i < ui->taskList->count(); ++i) {
    const auto item = ui->taskList->item(i);
    const auto task = static_cast<Task *>(ui->taskList->itemWidget(item));
    if (task) ordered.push_back(task);
  }

  std::vector<std::unique_ptr<Task>> newOrder;
  for (Task *t : ordered) {
    auto it = std::find_if(m_tasks.begin(), m_tasks.end(),
                           [t](const std::unique_ptr<Task> &ptr) { return ptr.get() == t; });
    if (it != m_tasks.end()) {
      newOrder.push_back(std::move(*it));
    }
  }
  m_tasks = std::move(newOrder);
}

void MainWindow::updateStatus() {
  int completeCount{};
  for (const auto &t : m_tasks) {
    if (t->isCompleted()) ++completeCount;
  }
  const auto total = static_cast<int>(m_tasks.size());
  const auto todoCount = total - completeCount;

  ui->statusLabel->setText(
      tr("Estatus: %1 ToDo / %2 Finalizado").arg(todoCount).arg(completeCount));

  ui->progressBar->setValue(total > 0 ? (completeCount * 100 / total) : 0);
  ui->progressBar->setToolTip(
      tr("%1%% completado (%2/%3)")
          .arg(total > 0 ? QString::number(completeCount * 100 / total) : "0")
          .arg(completeCount)
          .arg(total));

  setWindowTitle(tr("ToDo — %1 pendiente%2")
                     .arg(todoCount)
                     .arg(todoCount == 1 ? "" : "s"));
}

void MainWindow::showEmptyState() {
  const bool empty = m_tasks.empty();
  ui->emptyLabel->setVisible(empty);
  if (empty) {
    auto pal = ui->emptyLabel->palette();
    pal.setColor(QPalette::WindowText, pal.color(QPalette::PlaceholderText));
    ui->emptyLabel->setPalette(pal);
  }
}

void MainWindow::addTask() {
  auto dialog = new DDialog(this);
  dialog->setTitle(tr("Añadir tarea"));
  dialog->setIcon(QIcon::fromTheme("list-add"));

  auto edit = new QLineEdit(dialog);
  edit->setPlaceholderText(tr("Nombre de la tarea"));
  edit->setText(tr("Tarea sin título"));
  edit->selectAll();
  dialog->addContent(edit);

  auto reminderCheck = new QCheckBox(tr("Agregar recordatorio"), dialog);
  dialog->addContent(reminderCheck);

  auto dateTimeEdit = new QDateTimeEdit(QDateTime::currentDateTime().addSecs(3600), dialog);
  dateTimeEdit->setDisplayFormat("dd/MM/yyyy HH:mm");
  dateTimeEdit->setCalendarPopup(true);
  dateTimeEdit->setVisible(false);
  dialog->addContent(dateTimeEdit);

  connect(reminderCheck, &QCheckBox::toggled, dateTimeEdit, &QWidget::setVisible);

  dialog->addButton(tr("Cancelar"));
  dialog->addButton(tr("Añadir"), true);

  connect(dialog, &DDialog::buttonClicked, this,
          [this, dialog, edit, reminderCheck, dateTimeEdit](int index) {
    const auto name = edit->text().trimmed();
    if (index == 1 && !name.isEmpty()) {
      const QDateTime reminder = reminderCheck->isChecked()
          ? dateTimeEdit->dateTime()
          : QDateTime();
      createTask(name, QDateTime::currentDateTime(), false, reminder);
      updateStatus();
      showEmptyState();
      scheduleSave();
      NotificationManager::send(tr("Tarea creada"), name);
    }
    dialog->deleteLater();
  });

  edit->setFocus();
  dialog->exec();
}

int MainWindow::removeTaskFromList(Task *task) {
  for (int i = 0; i < ui->taskList->count(); ++i) {
    const auto item = ui->taskList->item(i);
    if (ui->taskList->itemWidget(item) == task) {
      delete ui->taskList->takeItem(i);
      return i;
    }
  }
  return -1;
}

void MainWindow::removeTask(Task *task) {
  auto dialog = new DDialog(this);
  dialog->setTitle(tr("Eliminar tarea"));
  dialog->setIcon(QIcon::fromTheme("dialog-warning"));
  dialog->setMessage(tr("¿Estás seguro de eliminar la tarea '%1'?").arg(task->name()));

  dialog->addButton(tr("Cancelar"));
  dialog->addButton(tr("Eliminar"), true);

  connect(dialog, &DDialog::buttonClicked, this, [this, task, dialog](int index) {
    dialog->deleteLater();
    if (index != 1) return;

    NotificationManager::send(tr("Tarea eliminada"), task->name());

    const int row = removeTaskFromList(task);
    m_lastDeleted = std::make_unique<DeletedTaskInfo>(DeletedTaskInfo{
        task->name(), task->isCompleted(), task->createdAt(),
        task->reminderAt(), task->isNotified(), row});

    auto it = std::find_if(m_tasks.begin(), m_tasks.end(),
                           [task](const std::unique_ptr<Task> &ptr) { return ptr.get() == task; });
    if (it != m_tasks.end()) {
      it->release();
      m_tasks.erase(it);
    }
    task->deleteLater();
    updateStatus();
    showEmptyState();
    scheduleSave();

    ui->undoButton->setVisible(true);
    m_undoTimer->start();  // reinicia la cuenta si ya había un deshacer pendiente
  });

  dialog->exec();
}

void MainWindow::undoDelete() {
  if (!m_lastDeleted) return;

  m_undoTimer->stop();
  createTask(m_lastDeleted->name, m_lastDeleted->createdAt,
             m_lastDeleted->completed, m_lastDeleted->reminderAt,
             m_lastDeleted->notified, m_lastDeleted->row);
  m_lastDeleted.reset();
  ui->undoButton->setVisible(false);
  updateStatus();
  showEmptyState();
  scheduleSave();
}

void MainWindow::taskStatusChanged(Task *task) {
  if (task->isCompleted()) {
    NotificationManager::send(tr("Tarea completada"), task->name());
  }
  updateStatus();
  scheduleSave();
}

void MainWindow::filterTasks(const QString &text) {
  Q_UNUSED(text);
  for (int i = 0; i < ui->taskList->count(); ++i) {
    const auto item = ui->taskList->item(i);
    applyFilter(item, static_cast<Task *>(ui->taskList->itemWidget(item)));
  }
}

void MainWindow::clearCompleted() {
  if (std::none_of(m_tasks.cbegin(), m_tasks.cend(),
                   [](const std::unique_ptr<Task> &t) { return t->isCompleted(); }))
    return;

  auto dialog = new DDialog(this);
  dialog->setTitle(tr("Limpiar tareas"));
  dialog->setIcon(QIcon::fromTheme("dialog-warning"));
  dialog->setMessage(tr("¿Eliminar todas las tareas finalizadas?"));

  dialog->addButton(tr("Cancelar"));
  dialog->addButton(tr("Eliminar"), true);

  connect(dialog, &DDialog::buttonClicked, this, [this, dialog](int index) {
    dialog->deleteLater();
    if (index != 1) return;

    for (int i = ui->taskList->count() - 1; i >= 0; --i) {
      const auto item = ui->taskList->item(i);
      const auto task = static_cast<Task *>(ui->taskList->itemWidget(item));
      if (task->isCompleted()) {
        delete ui->taskList->takeItem(i);
        auto it = std::find_if(m_tasks.begin(), m_tasks.end(),
                               [task](const std::unique_ptr<Task> &ptr) { return ptr.get() == task; });
        if (it != m_tasks.end()) {
          it->release();
          m_tasks.erase(it);
        }
        task->deleteLater();
      }
    }
    updateStatus();
    showEmptyState();
    scheduleSave();
  });

  dialog->exec();
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
  dialog->setTitle(tr("Estadísticas"));
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

void MainWindow::checkReminders() {
  const auto now = QDateTime::currentDateTime();
  bool changed = false;
  for (const auto &task : m_tasks) {
    if (task->hasReminder() && !task->isNotified() &&
        task->reminderAt() <= now && !task->isCompleted()) {
      task->setNotified(true);
      task->updateReminderLabel();
      NotificationManager::send(tr("Recordatorio"), task->name());
      changed = true;
    }
  }
  if (changed) scheduleSave();
}

void MainWindow::scheduleSave() {
  m_saveTimer->start();
}

void MainWindow::saveTasks() const {
  QSettings settings("deepines", "ToDo");
  // Borrar el array anterior: beginWriteArray no elimina claves viejas, así que
  // un recordatorio quitado o tareas sobrantes reaparecerían al recargar.
  settings.remove("tasks");
  settings.beginWriteArray("tasks");
  int i = 0;
  for (const auto &task : m_tasks) {
    settings.setArrayIndex(i++);
    settings.setValue("name", task->name());
    settings.setValue("completed", task->isCompleted());
    settings.setValue("createdAt", task->createdAt().toString(Qt::ISODate));
    if (task->hasReminder()) {
      settings.setValue("reminderAt", task->reminderAt().toString(Qt::ISODate));
    }
    settings.setValue("notified", task->isNotified());
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

    const auto reminderAt = QDateTime::fromString(
        settings.value("reminderAt").toString(), Qt::ISODate);
    const auto notified = settings.value("notified", false).toBool();

    createTask(name, createdAt.isValid() ? createdAt : QDateTime::currentDateTime(),
               completed, reminderAt, notified);
  }
  settings.endArray();
}

void MainWindow::editReminder(Task *task) {
  auto dialog = new DDialog(this);
  dialog->setTitle(tr("Recordatorio"));
  dialog->setIcon(QIcon::fromTheme("appointment-new"));
  dialog->setMessage(task->name());

  auto reminderCheck = new QCheckBox(tr("Agregar recordatorio"), dialog);
  reminderCheck->setChecked(task->hasReminder());
  dialog->addContent(reminderCheck);

  const auto initial = task->hasReminder()
      ? task->reminderAt()
      : QDateTime::currentDateTime().addSecs(3600);
  auto dateTimeEdit = new QDateTimeEdit(initial, dialog);
  dateTimeEdit->setDisplayFormat("dd/MM/yyyy HH:mm");
  dateTimeEdit->setCalendarPopup(true);
  dateTimeEdit->setEnabled(task->hasReminder());
  dialog->addContent(dateTimeEdit);
  connect(reminderCheck, &QCheckBox::toggled, dateTimeEdit, &QWidget::setEnabled);

  dialog->addButton(tr("Cancelar"));
  dialog->addButton(tr("Guardar"), true);

  connect(dialog, &DDialog::buttonClicked, this,
          [this, task, dialog, reminderCheck, dateTimeEdit](int index) {
    dialog->deleteLater();
    if (index != 1) return;
    task->setReminderAt(reminderCheck->isChecked() ? dateTimeEdit->dateTime()
                                                   : QDateTime());
    scheduleSave();
    checkReminders();
  });

  dialog->exec();
}
