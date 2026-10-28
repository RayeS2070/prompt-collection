# prompts-collector

Настольное приложение для хранения промптов к нейросетям, работающим с
изображениями (Stable Diffusion и подобные).

> Статус: proof of concept — список промптов с фильтром и редактор.

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

Требуется CMake 3.30+, Ninja, компилятор с поддержкой C++26 (GCC или
Clang) и SQLite 3 из системы. SDL3, Dear ImGui и GoogleTest подтягиваются
через [CPM.cmake](https://github.com/cpm-cmake/CPM.cmake) в кэш
`~/.cache/CPM`.

```sh
cmake --preset gcc_debug
cmake --build --preset gcc_debug
ctest --preset gcc_debug
```

Список пресетов: `cmake --list-presets`. Сборки кладутся в `build/`.

## Запуск

```sh
build/linux/gcc_debug/ui/prompt_collection [DATABASE]
```

По умолчанию база — `$XDG_DATA_HOME/pcollect/prompts.db`
(`~/.local/share/pcollect/prompts.db`). Ctrl+S сохраняет промпт.

## Архитектура

Однонаправленный поток данных: view каждый кадр рисует `AppState` и
собирает действия пользователя (`app::Action`), а controller после кадра
применяет их к модели и обновляет состояние.

```
src/model/  модель: Prompt, PromptStore (SQLite); без UI
src/app/    контроллер: AppState, Action, Controller
ui/         view на ImGui и главный цикл (SDL3 + SDL_Renderer)
tests/      тесты модели и контроллера (GoogleTest)
cmake/      CMake-модули (CPM, встраивание файлов в бинарник)
```

## Лицензия

GPL-3.0, см. [LICENSE](LICENSE).
