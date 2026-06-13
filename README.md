# ToDo

App de tareas de la comunidad deepin en Español. Construida con Qt6 y DTK para el escritorio Deepin.

<p align="center">
  <img src="resources/Logo-Racoon.png" width="128" alt="Deepines ToDo Logo">
</p>

## Características

- **Crear, editar, eliminar tareas** con renombrado inline (doble clic)
- **Marcar tareas como completadas** con tachado visual
- **Drag & drop** para reordenar tareas
- **Búsqueda y filtro** en tiempo real por nombre
- **Atajos de teclado**:
  - `Ctrl+N` — Añadir tarea
  - `Ctrl+Delete` — Limpiar completadas
  - `Ctrl+E` — Exportar a JSON
  - `Ctrl+I` — Importar desde JSON
  - `Ctrl+T` — Estadísticas
  - `Escape` — Limpiar búsqueda
- **Auto-guardado** con debounce (1.5s) — nunca pierdes datos
- **Deshacer eliminar** — la última tarea eliminada se puede restaurar (5 segundos)
- **Barra de progreso** con porcentaje de completitud
- **Diálogo de estadísticas**: total, pendientes, finalizadas, porcentaje, creadas hoy
- **Exportar/Importar JSON** desde el menú del título o atajos
- **Almacenamiento persistente** con QSettings
- **Fecha de creación** con tiempo relativo ("hace 5 min", "ayer")
- **Integración DTK**: diálogo About y menú en la barra de título

## Capturas

*(Agregar capturas aquí)*

## Compilar desde código fuente

### Dependencias

- Qt 6.x (Widgets, DBus)
- DTK Widget 6.x
- pkg-config
- g++ 10+ o clang 12+
- cmake 3.14+

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
sudo dpkg -i deepines-todo_1.0.0-1_amd64.deb
sudo apt install -f
```

### Make install (sistema completo)

```bash
cmake -B build -DCMAKE_INSTALL_PREFIX=/usr
sudo cmake --install build
```

Esto instala:
- Binario en `/usr/bin/ToDo`
- Icono en `/usr/share/icons/hicolor/256x256/apps/deepines-todo.png`
- Desktop entry en `/usr/share/applications/deepines-todo.desktop`
- Documentación en `/usr/share/doc/ToDo/`

## Estructura del proyecto

```
├── CMakeLists.txt              # Build system CMake
├── Deepines-ToDo.pro           # Build system qmake
├── README.md
├── LICENSE                     # GPL-3.0
├── .gitignore
├── src/                        # Código fuente
│   ├── main.cpp                # Punto de entrada, configuración DTK
│   ├── MainWindow.h/.cpp       # Ventana principal, lógica de tareas
│   ├── MainWindow.ui           # UI de la ventana principal (Qt Designer)
│   ├── Task.h/.cpp             # Widget de tarea individual
│   └── Task.ui                 # UI de cada tarea (Qt Designer)
├── resources/                  # Recursos
│   ├── imgs.qrc                # Archivo de recursos Qt
│   ├── Logo-Racoon.png         # Icono de la aplicación
│   └── deepines-todo.desktop   # Archivo .desktop para Linux
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

La aplicación está compuesta por dos clases principales:

### `MainWindow` (hereda de `DMainWindow`)

Ventana principal que gestiona la lista de tareas. Responsabilidades:

- CRUD de tareas (crear, renombrar, eliminar, marcar completada)
- Filtrado por nombre en tiempo real
- Reordenamiento con drag & drop (`QListWidget` con `InternalMove`)
- Persistencia con `QSettings` (guarda nombre, estado y fecha de creación)
- Auto-guardado con `QTimer` de debounce (1.5 segundos)
- Undo delete: almacena la última tarea eliminada para restaurar
- Exportar/Importar tareas a formato JSON
- Diálogo de estadísticas con `DDialog`
- Menú en la barra de título (`DTitlebar`)

### `Task` (hereda de `DWidget`)

Widget individual para cada tarea. Funcionalidad:

- Checkbox con tachado al completar
- Edición inline con `QStackedWidget` (alterna entre vista y `QLineEdit`)
- Doble clic para renombrar
- `Escape` cancela la edición, `Enter` guarda
- Fecha de creación con tiempo relativo
- Señales `removed` y `statusChanged` para comunicarse con `MainWindow`

### Flujo de datos

```
Usuario interactúa con Task widget
    → Task emite removed/statusChanged
    → MainWindow recibe señal
    → Actualiza m_tasks vector y UI
    → scheduleSave() inicia QTimer (1.5s debounce)
    → saveTasks() escribe a QSettings
```

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
