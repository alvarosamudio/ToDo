# Deepines ToDo

App de tareas de la comunidad deepin en Español. Construida con Qt5/6 y DTK para el escritorio Deepin.

<p align="center">
  <img src="resources/Logo-Racoon.png" width="128" alt="Deepines ToDo Logo">
</p>

## Caracteristicas

- **Crear, editar, eliminar tareas** con renombrado inline (doble clic)
- **Marcar tareas como completadas** con tachado visual
- **Drag & drop** para reordenar tareas
- **Busqueda y filtro** en tiempo real por nombre
- **Atajos de teclado**:
  - `Ctrl+N` — Anadir tarea
  - `Ctrl+Delete` — Limpiar completadas
  - `Ctrl+E` — Exportar a JSON
  - `Ctrl+I` — Importar desde JSON
  - `Ctrl+T` — Estadisticas
  - `Escape` — Limpiar busqueda
- **Auto-guardado** con debounce (1.5s) — nunca pierdes datos
- **Deshacer eliminar** — la ultima tarea eliminada se puede restaurar (5 segundos)
- **Barra de progreso** con porcentaje de completitud
- **Dialogo de estadisticas**: total, pendientes, finalizadas, porcentaje, creadas hoy
- **Exportar/Importar JSON** desde el menu del titulo o atajos
- **Almacenamiento persistente** con QSettings
- **Fecha de creacion** con tiempo relativo ("hace 5 min", "ayer")
- **Integracion DTK**: dialogo About y menu en la barra de titulo

## Capturas

*(Agregar capturas aqui)*

## Compilar desde codigo fuente

### Dependencias

- Qt 5.15+ o Qt 6.x (Widgets, DBus)
- DTK Widget 5.5+
- pkg-config
- g++ 10+ o clang 12+
- cmake 3.14+ (para build con CMake)

### Build con qmake

```bash
mkdir build && cd build
qmake -qt=5 ../Deepines-ToDo.pro
make -j$(nproc)
```

### Build con CMake

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Ejecutar

```bash
./DeepinesToDo
```

## Instalacion

### Paquete Debian/Ubuntu

```bash
# Desde codigo fuente
dpkg-buildpackage -us -uc

# O instalar .deb pre-compilado
sudo dpkg -i deepines-todo_1.0.0-1_amd64.deb
sudo apt install -f
```

### Make install (sistema completo)

```bash
sudo make install
```

Esto instala:
- Binario en `/usr/bin/DeepinesToDo`
- Icono en `/usr/share/icons/hicolor/256x256/apps/deepines-todo.png`
- Desktop entry en `/usr/share/applications/deepines-todo.desktop`
- Documentacion en `/usr/share/doc/DeepinesToDo/`

## Estructura del proyecto

```
├── CMakeLists.txt              # Build system CMake
├── Deepines-ToDo.pro           # Build system qmake
├── README.md
├── LICENSE                     # GPL-3.0
├── .gitignore
├── src/                        # Codigo fuente
│   ├── main.cpp                # Punto de entrada, configuracion DTK
│   ├── MainWindow.h/.cpp       # Ventana principal, logica de tareas
│   ├── MainWindow.ui           # UI de la ventana principal (Qt Designer)
│   ├── Task.h/.cpp             # Widget de tarea individual
│   └── Task.ui                 # UI de cada tarea (Qt Designer)
├── resources/                  # Recursos
│   ├── imgs.qrc                # Archivo de recursos Qt
│   ├── Logo-Racoon.png         # Icono de la aplicacion
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

La aplicacion esta compuesta por dos clases principales:

### `MainWindow` (hereda de `DMainWindow`)

Ventana principal que gestiona la lista de tareas. Responsabilidades:

- CRUD de tareas (crear, renombrar, eliminar, marcar completada)
- Filtrado por nombre en tiempo real
- Reordenamiento con drag & drop (`QListWidget` con `InternalMove`)
- Persistencia con `QSettings` (guarda nombre, estado y fecha de creacion)
- Auto-guardado con `QTimer` de debounce (1.5 segundos)
- Undo delete: almacena la ultima tarea eliminada para restaurar
- Exportar/Importar tareas a formato JSON
- Dialogo de estadisticas con `DDialog`
- Menu en la barra de titulo (`DTitlebar`)

### `Task` (hereda de `DWidget`)

Widget individual para cada tarea. Funcionalidad:

- Checkbox con tachado al completar
- Edicion inline con `QStackedWidget` (alterna entre vista y `QLineEdit`)
- Doble clic para renombrar
- `Escape` cancela la edicion, `Enter` guarda
- Fecha de creacion con tiempo relativo
- Senales `removed` y `statusChanged` para comunicarse con `MainWindow`

### Flujo de datos

```
Usuario interactua con Task widget
    → Task emite removed/statusChanged
    → MainWindow recibe senal
    → Actualiza m_tasks vector y UI
    → scheduleSave() inicia QTimer (1.5s debounce)
    → saveTasks() escribe a QSettings
```

## Atajos de teclado

| Atajo | Accion |
|-------|--------|
| `Ctrl+N` | Anadir nueva tarea |
| `Ctrl+Delete` | Limpiar tareas completadas |
| `Ctrl+E` | Exportar tareas a JSON |
| `Ctrl+I` | Importar tareas desde JSON |
| `Ctrl+T` | Mostrar estadisticas |
| `Escape` | Limpiar campo de busqueda |
| `Doble clic` | Renombrar tarea |
| `Enter` | Guardar nombre al editar |
| `Escape` (editando) | Cancelar edicion |

## Licencia

GPL-3.0-or-later — ver [LICENSE](LICENSE).

## Creditos

- **Deepin en Espanol** — https://deepinenespanol.org
- **Deepin Latin Code** — developers
- **Alvaro Samudio** — autor
