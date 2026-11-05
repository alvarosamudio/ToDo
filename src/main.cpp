#include "MainWindow.h"
#include <DApplication>
#include <DWidgetUtil>
#include <DAboutDialog>
#include <QTranslator>

DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[]) {
  DApplication a(argc, argv);

  a.setProductName("ToDo");
  a.setApplicationName("ToDo");
  a.setApplicationVersion(APP_VERSION);

  // QTranslator::load(QLocale, ...) prueba "es_ES", luego "es", etc., así que
  // locales como en_US o zh_CN encuentran ToDo_en.qm / ToDo_zh.qm.
  QTranslator translator;
  if (translator.load(QLocale(), "ToDo", "_", ":/translations")) {
    a.installTranslator(&translator);
  }

  DAboutDialog dialog;
  dialog.setWindowTitle("Acerca de ToDo");
  dialog.setProductName("ToDo");
  dialog.setProductIcon(QIcon("://todo-icon.svg"));
  dialog.setVersion(DApplication::buildVersion(APP_VERSION));
  dialog.setDescription(
      QStringLiteral(
          "<span style='font-size:8pt; font-weight:600;'>"
          "App de tareas de la comunidad deepin en Español"
          "</span><br/><br/>"
          "<span style='font-size:8pt; font-weight:600;'>Deepin en Español </span>"
          "<a href='https://deepinenespanol.org'>https://deepinenespanol.org</a><br/>"
          "<span style='font-size:8pt; font-weight:600;'>Deepin Latin Code - developers</span>"));
  dialog.setCompanyLogo(QPixmap("://todo-icon.svg"));
  dialog.setWebsiteName("deepinenespanol.org");
  dialog.setWebsiteLink("https://deepinenespanol.org");

  a.setAboutDialog(&dialog);

  MainWindow w;
  w.show();
  Dtk::Widget::moveToCenter(&w);
  return a.exec();
}
