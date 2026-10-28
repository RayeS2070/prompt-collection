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

Требуется CMake 3.30+, Ninja и компилятор с поддержкой C++26 (GCC или
Clang). Все библиотеки — SQLite, SDL3, Dear ImGui, GoogleTest — собираются
из исходников, которые [CPM.cmake](https://github.com/cpm-cmake/CPM.cmake)
скачивает в кэш `~/.cache/CPM`. Из системы нужны только заголовки платформы,
которые требует SDL (X11/Wayland, OpenGL и т.п.).

```sh
cmake --preset gcc_debug
cmake --build --preset gcc_debug
ctest --preset gcc_debug
```

Сборки кладутся в `build/<пресет>/`. `cmake --list-presets` показывает
пресеты, доступные на текущей ОС:

| Пресет | Где | Что |
|---|---|---|
| `gcc_debug`, `gcc_release` | Linux; Windows в шелле MSYS2 UCRT64 | GCC / MinGW GCC |
| `clang_debug`, `clang_release` | Linux | Clang + LLD |
| `clang_asan` | Linux | Clang с AddressSanitizer и UBSan |
| `mingw_debug`, `mingw_release` | Linux | кросс-сборка под Windows (MinGW-w64), тесты через wine |
| `macos_debug`, `macos_release` | macOS | Clang и libc++ из Homebrew LLVM; нужен `export LLVM_ROOT=$(brew --prefix llvm)` |

## CI

GitHub Actions ([.github/workflows](.github/workflows)):

- `CI` — на каждый push и pull request: Linux (`gcc_debug`, `clang_asan`)
  и Windows (MSYS2 UCRT64, `gcc_debug`).
- `macOS` — только вручную: Actions → macOS → Run workflow.

## Запуск

```sh
build/gcc_debug/ui/prompt_collection [DATABASE]
```

По умолчанию база — `prompts.db` в каталоге данных пользователя:
`~/.local/share/pcollect/` на Linux, `%APPDATA%\pcollect\` на Windows,
`~/Library/Application Support/pcollect/` на macOS. Ctrl+S сохраняет
промпт.

## Архитектура

Однонаправленный поток данных: view каждый кадр рисует `AppState` и
собирает действия пользователя (`app::Action`), а controller после кадра
применяет их к модели и обновляет состояние.

```
src/model/  модель: Prompt, PromptStore (SQLite); без UI
src/app/    контроллер: AppState, Action, Controller
            (у каждой библиотеки include/<lib>/ и src/)
ui/         view на ImGui и главный цикл (SDL3 + SDL_Renderer)
tests/      тесты модели и контроллера (GoogleTest)
cmake/      CMake-модули (CPM, встраивание файлов в бинарник)
```

## Лицензия

GPL-3.0, см. [LICENSE](LICENSE).
