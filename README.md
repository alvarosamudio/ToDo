# ToDo v2.0

App de tareas de la comunidad deepin en Español. Construida con Qt6 y DTK para el escritorio Deepin.

<p align="center">
  <img src="resources/todo-icon.svg" width="128" alt="ToDo Logo">
</p>

## Características

- **Crear, editar, eliminar tareas** con renombrado inline (doble clic)
- **Marcar tareas como completadas** con tachado visual
- **Drag & drop** para reordenar tareas
- **Búsqueda y filtro** en tiempo real por nombre
- **Recordatorios** con fecha/hora — notificación del escritorio al vencimiento
- **Notificaciones de acciones** al crear, completar o eliminar tareas
- **28 idiomas** (i18n): español, inglés, chino, francés, árabe, portugués, ruso, japonés, alemán, italiano, coreano, tailandés, neerlandés, catalán, polaco, turco, ucraniano, bengalí, indonesio, urdu, hausa, swahili, telugu, marathi, hindi, persa, chino tradicional, yoruba
- **Diálogos nativos DTK** (DDialog) para todas las operaciones
- **Iconos DTK** en botones de toolbar y acciones de tarea
- **UI con toolbar en 2 filas**: búsqueda + stats + limpiar / estado + progreso + deshacer + añadir
- **Barra de progreso** con porcentaje visible
- **Atajos de teclado**:
  - `Ctrl+N` — Añadir tarea
  - `Ctrl+Delete` — Limpiar completadas
  - `Ctrl+E` — Exportar a JSON
  - `Ctrl+I` — Importar desde JSON
  - `Ctrl+T` — Estadísticas
  - `Escape` — Limpiar búsqueda
- **Auto-guardado** con debounce (1.5s) — nunca pierdes datos
- **Deshacer eliminar** — la última tarea eliminada se puede restaurar (5 segundos)
- **Exportar/Importar JSON** con soporte de recordatorios
- **Almacenamiento persistente** con QSettings
- **Fecha de creación** con tiempo relativo ("hace 5 min", "ayer")
- **Integración DTK**: diálogo About con icono SVG, menú en barra de título

## Compilar desde código fuente

### Dependencias

- Qt 6.x (Widgets, DBus)
- DTK Widget 6.x (`dtk6widget`)
- pkg-config
- g++ 10+ o clang 12+
- cmake 3.14+
- Qt6 LinguistTools (para traducciones)

### Build con CMake

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Ejecutar

```bash
./ToDo
```

## Instalación

### Paquete Debian/Ubuntu

```bash
# Desde código fuente
dpkg-buildpackage -us -uc

# O instalar .deb pre-compilado
sudo dpkg -i deepines-todo_2.0-1_amd64.deb
sudo apt install -f
```

### Make install (sistema completo)

```bash
cmake -B build -DCMAKE_INSTALL_PREFIX=/usr
sudo cmake --install build
```

Esto instala:
- Binario en `/usr/bin/ToDo`
- Icono SVG en `/usr/share/icons/hicolor/scalable/apps/deepines-todo.svg`
- Desktop entry en `/usr/share/applications/deepines-todo.desktop`
- AppData en `/usr/share/metainfo/deepines-todo.appdata.xml`
- Documentación en `/usr/share/doc/ToDo/`
- Traducciones en el binario (28 idiomas)

## Estructura del proyecto

```
├── CMakeLists.txt              # Build system (Qt6 + DTK + LinguistTools)
├── README.md
├── LICENSE                     # GPL-3.0
├── .gitignore
├── src/                        # Código fuente
│   ├── main.cpp                # Punto de entrada, configuración DTK, About dialog
│   ├── MainWindow.h/.cpp       # Ventana principal, lógica de tareas, notificaciones
│   ├── MainWindow.ui           # UI de la ventana principal (toolbar en 2 filas)
│   ├── Task.h/.cpp             # Widget de tarea individual, recordatorios
│   ├── Task.ui                 # UI de cada tarea (checkbox + fecha + campana + botones)
│   └── NotificationManager.h   # Helper DBus para notificaciones del escritorio
├── translations/               # Archivos de traducción (28 idiomas)
│   ├── ToDo_es.ts
│   ├── ToDo_en.ts
│   ├── ToDo_zh.ts
│   └── ... (28 archivos .ts)
├── resources/                  # Recursos
│   ├── imgs.qrc                # Archivo de recursos Qt
│   ├── Logo-Racoon.png         # Logo de la comunidad
│   ├── todo-icon.svg           # Icono de la aplicación
│   └── deepines-todo.appdata.xml
├── debian/                     # Empaquetado Debian
│   ├── changelog
│   ├── compat
│   ├── control
│   ├── copyright
│   ├── rules
│   └── source/format
└── doc/
    └── IMG/
        └── deepinestodo.png    # Captura de pantalla
```

## Arquitectura

### `MainWindow` (hereda de `DMainWindow`)

Ventana principal que gestiona la lista de tareas:

- CRUD de tareas (crear, renombrar, eliminar, marcar completada)
- Filtrado por nombre en tiempo real
- Reordenamiento con drag & drop (`QListWidget` con `InternalMove`)
- Persistencia con `QSettings` (nombre, estado, fecha creación, recordatorio, notificado)
- Auto-guardado con `QTimer` de debounce (1.5 segundos)
- **Timer de recordatorios** (`QTimer`, 30 segundos) — verifica tareas vencidas y envía notificación
- Undo delete: almacena la última tarea eliminada para restaurar
- Exportar/Importar tareas a formato JSON (incluye campos de recordatorio)
- Diálogos nativos DTK (`DDialog`) para todas las operaciones
- Menú en la barra de título (`DTitlebar`)
- **Notificaciones de acciones**: enviar notificación al crear, completar y eliminar tareas

### `Task` (hereda de `DWidget`)

Widget individual para cada tarea:

- Checkbox con tachado al completar
- Edición inline con `QStackedWidget` (alterna entre vista y `QLineEdit`)
- Doble clic para renombrar
- `Escape` cancela la edición, `Enter` guarda
- Fecha de creación con tiempo relativo
- **Recordatorio** (`m_reminderAt`): DateTime del recordatorio
- **Estado de notificación** (`m_notified`: si ya se envió la notificación)
- **Label de campana** (`reminderLabel`): muestra tiempo restante del recordatorio
- Iconos DTK en botones: `document-edit` (editar) y `edit-delete` (eliminar)
- Señales `removed` y `statusChanged` para comunicarse con `MainWindow`

### `NotificationManager` (clase estática)

Helper para enviar notificaciones del escritorio via DBus:

- Usa `org.freedesktop.Notifications` (especificación freedesktop)
- Sin dependencias adicionales — solo `Qt6::DBus`
- Método estático `send(title, body, icon)`
- Timeout de 5 segundos

### Sistema de notificaciones

```
Task tiene reminderAt (QDateTime)
    → MainWindow tiene m_reminderTimer (30 segundos)
    → Cada tick: verifica tareas vencidas (!isNotified && reminderAt <= now)
    → NotificationManager::send() vía DBus
    → Marca task->setNotified(true)
    → scheduleSave() persiste el cambio
```

### Flujo de datos

```
Usuario interactúa con Task widget
    → Task emite removed/statusChanged
    → MainWindow recibe señal
    → Actualiza m_tasks vector y UI
    → scheduleSave() inicia QTimer (1.5s debounce)
    → saveTasks() escribe a QSettings (incluye reminderAt, notified)
```

### i18n (internacionalización)

- Base: español (es)
- 28 idiomas soportados via Qt Linguist
- Strings envueltos en `tr()` en todo el código fuente
- Traducciones en `translations/ToDo_*.ts`
- Carga automática por locale del sistema en `main.cpp`

## Atajos de teclado

| Atajo | Acción |
|-------|--------|
| `Ctrl+N` | Añadir nueva tarea |
| `Ctrl+Delete` | Limpiar tareas completadas |
| `Ctrl+E` | Exportar tareas a JSON |
| `Ctrl+I` | Importar tareas desde JSON |
| `Ctrl+T` | Mostrar estadísticas |
| `Escape` | Limpiar campo de búsqueda |
| `Doble clic` | Renombrar tarea |
| `Enter` | Guardar nombre al editar |
| `Escape` (editando) | Cancelar edición |

## Licencia

GPL-3.0-or-later — ver [LICENSE](LICENSE).

## Créditos

- **Deepin en Español** — https://deepinenespanol.org
- **Deepin Latin Code** — developers
- **Alvaro Samudio** — autor
