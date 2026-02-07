#include "MainWindow.h"
#include <DApplication>
#include <DWidgetUtil>
#include <DAboutDialog>

DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[]) {
  DApplication a(argc, argv);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  a.setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

  a.setProductName("Deepines ToDo");
  a.setApplicationName("Deepines ToDo");

  DAboutDialog dialog;
  dialog.setWindowTitle("Acerca de Deepines ToDo");
  dialog.setProductName("Deepines ToDo");
  dialog.setVersion(DApplication::buildVersion("Version 1.0"));
  dialog.setDescription(
      QStringLiteral(
          "<span style='font-size:8pt; font-weight:600;'>"
          "ToDo App de la comunidad deepin en Español"
          "</span><br/><br/>"
          "<span style='font-size:8pt; font-weight:600;'>Deepin en Español </span>"
          "<a href='https://deepinenespanol.org'>https://deepinenespanol.org</a><br/>"
          "<span style='font-size:8pt; font-weight:600;'>Deepin Latin Code - developers</span>"));
  dialog.setCompanyLogo(QPixmap("://Logo-Racoon.png"));
  dialog.setWebsiteName("deepinenespanol.org");
  dialog.setWebsiteLink("https://deepinenespanol.org");

  a.setAboutDialog(&dialog);

  MainWindow w;
  w.show();
  Dtk::Widget::moveToCenter(&w);
  return a.exec();
}
