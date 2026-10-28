# prompts-collector

Настольное приложение для хранения промптов к нейросетям, работающим с
изображениями (Stable Diffusion и подобные).

> Статус: ранняя заготовка. Код в `src/` и `CMakeLists.txt` перенесён из
> другого проекта и пока не собирается; архитектура в процессе обсуждения.

## Цели

- Хранить промпты (позитивные и негативные) в локальной базе SQLite.
- Графический интерфейс на [Dear ImGui](https://github.com/ocornut/imgui).
- Архитектура MVC: модель (данные и SQLite), представление (ImGui),
  контроллер (связывает их).
- Позже:
  - классификация промптов (категории, теги);
  - импорт тегов с [Danbooru](https://danbooru.donmai.us/tags) как
    готовых промптов.

## Сборка

Требуется CMake 3.30+, Ninja и компилятор с поддержкой C++26 (GCC или
Clang). Зависимости подтягиваются через [CPM.cmake](https://github.com/cpm-cmake/CPM.cmake)
в кэш `~/.cache/CPM`.

```sh
cmake --preset gcc_debug
cmake --build --preset gcc_debug
ctest --preset gcc_debug
```

Список пресетов: `cmake --list-presets`. Сборки кладутся в `build/`.

## Структура

```
cmake/      CMake-модули (CPM, встраивание файлов в бинарник)
src/        исходники приложения
ui/         интерфейс (ImGui)
tests/      модульные тесты
```

## Лицензия

GPL-3.0, см. [LICENSE](LICENSE).
