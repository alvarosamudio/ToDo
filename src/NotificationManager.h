#ifndef NOTIFICATIONMANAGER_H
#define NOTIFICATIONMANAGER_H

#include <QDBusInterface>
#include <QDBusConnection>
#include <QString>

class NotificationManager {
public:
  static void send(const QString &title, const QString &body,
                   const QString &icon = "deepines-todo") {
    QDBusInterface iface("org.freedesktop.Notifications",
                         "/org/freedesktop/Notifications",
                         "org.freedesktop.Notifications");
    if (!iface.isValid()) return;

    iface.call("Notify",
               QString("ToDo"),
               static_cast<quint32>(0),
               icon,
               title,
               body,
               QStringList(),
               QVariantMap(),
               static_cast<int>(5000));
  }
};

#endif // NOTIFICATIONMANAGER_H
