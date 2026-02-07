# Deepines ToDo

A simple and elegant todo list application built with Qt5 and DTK for the Deepin desktop environment.

## Features

- **Create, edit, delete tasks** with inline rename (double-click)
- **Mark tasks as completed** with strike-through visual feedback
- **Drag & drop** to reorder tasks
- **Search & filter** tasks in real time
- **Keyboard shortcuts**: `Ctrl+N` add, `Ctrl+Delete` clear completed, `Ctrl+E` export, `Ctrl+I` import, `Ctrl+T` stats, `Escape` clear search
- **Auto-save** with debounce timer — never lose your data
- **Undo delete** — last deleted task can be restored
- **Progress bar** showing completion percentage
- **Statistics dialog** with totals, completion rate, and tasks created today
- **Export/Import to JSON** via the window menu or keyboard shortcuts
- **Persistent storage** using QSettings
- **Creation date tracking** with relative time display ("hace 5 min", "ayer")
- **Deepin DTK integration** with About dialog and titlebar menu

## Screenshots

*(Add screenshots here)*

## Build from source

### Dependencies

- Qt 5.15+ (Widgets, DBus)
- DTK Widget 5.5+
- pkg-config
- g++ 10+ or clang 12+

### Build with qmake

```bash
mkdir build && cd build
qmake -qt=5 ../Deepines-ToDo.pro
make -j$(nproc)
```

### Build with CMake

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Run

```bash
./DeepinesToDo
```

## Installation

### Debian/Ubuntu package

```bash
# From source
dpkg-buildpackage -us -uc

# Or install pre-built .deb
sudo dpkg -i deepines-todo_1.0.0-1_amd64.deb
sudo apt install -f
```

### Make install (system-wide)

```bash
sudo make install
```

## Project structure

```
├── CMakeLists.txt          # CMake build system
├── Deepines-ToDo.pro       # qmake project file
├── README.md
├── LICENSE
├── .gitignore
├── src/                    # Source code
│   ├── main.cpp
│   ├── MainWindow.h/.cpp/.ui
│   └── Task.h/.cpp/.ui
├── resources/              # Assets
│   ├── imgs.qrc
│   ├── Logo-Racoon.png
│   └── deepines-todo.desktop
├── debian/                 # Debian packaging
│   ├── changelog
│   ├── compat
│   ├── control
│   ├── copyright
│   ├── rules
│   └── source/format
└── doc/
```

## License

GPL-3.0-or-later — see [LICENSE](LICENSE).

## Credits

- **Deepin en Español** — https://deepinenespanol.org
- **Deepin Latin Code** — developers
