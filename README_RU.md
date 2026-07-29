# Плагин AltShiftMenuFix для Notepad++

[English version](README.md)

AltShiftMenuFix — небольшой плагин, который предотвращает
случайную активацию меню Notepad++ после переключения раскладки клавиатуры
сочетанием <kbd>Alt</kbd>+<kbd>Shift</kbd>.

## Почему возникает проблема

Windows воспринимает отпускание Alt как сигнал для возможной активации строки
меню приложения. Если Left Alt+Shift также назначено для переключения языка,
одна и та же последовательность клавиш может одновременно сменить раскладку и
отправить Notepad++ команду активации меню с клавиатуры.

Если отпустить Alt, пока Shift всё ещё удерживается, Notepad++ может перевести
фокус на строку меню. Следующая введённая буква будет воспринята как мнемоника
меню: вместо появления буквы в документе откроется соответствующее меню.

## Что делает плагин

Плагин устанавливает небольшой фильтр сообщений для главного окна Notepad++.
Он отбрасывает только команду клавиатурной активации меню, возникающую при
отпускании Alt без буквы меню, когда Shift всё ещё удерживается.

Плагин не изменяет:

- переключение раскладки Windows по Alt+Shift;
- обычную активацию меню одиночным нажатием Alt;
- сочетания Alt+буква;
- остальные горячие клавиши Notepad++;
- окна, созданные другими плагинами.

Плагин не создаёт файлы настроек, не подключается к сети, не собирает
телеметрию и не запускает фоновые процессы.

## Как воспроизвести проблему без плагина

В Windows переключение языка должно быть назначено на Left Alt+Shift:

1. Установите фокус в редактор Notepad++.
2. Нажмите и удерживайте Left Shift.
3. Нажмите и отпустите Left Alt, продолжая удерживать Shift.
4. Отпустите Shift и введите букву, используемую меню Notepad++.

Без плагина фокус может перейти в меню, после чего оно откроется. При
включённом плагине буква будет как обычно вставлена в документ.

## Установка

1. Скачайте `NppAltShiftMenuFix-1.0-x64.zip` в разделе Downloads ниже.
2. Закройте все окна Notepad++.
3. Создайте `%ProgramFiles%\Notepad++\plugins\AltShiftMenuFix`, если этой папки
   ещё нет.
4. Извлеките единственный файл `AltShiftMenuFix.dll` в эту папку.
5. Запустите Notepad++.

Для удаления закройте Notepad++ и удалите папку плагина `AltShiftMenuFix`.

## Совместимость

- Windows x64
- Notepad++ x64
- Протестировано с Notepad++ 8.9.6

## Сборка

Необходимы Windows x64, CMake 3.21 или новее, а также Visual Studio 2022 Build
Tools либо MinGW-w64 с компилятором ресурсов.

Visual Studio:

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
cpack --config build/CPackConfig.cmake -C Release -B dist
```

MinGW-w64 с Ninja:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

Репозиторий содержит только минимальную часть ABI Notepad++, необходимую этому
плагину. Демонстрационный проект `plugintemplate` и его docking framework не
собираются и не распространяются.

## Лицензия

AltShiftMenuFix распространяется на условиях
[GNU General Public License v3.0 или более поздней версии](LICENSE).

## Downloads

- [Последний релиз](https://github.com/numbereleven-a/Notepadpp_AltShiftMenuFix/releases/latest)
- [Скачать AltShiftMenuFix 1.0 для Notepad++ x64](https://github.com/numbereleven-a/Notepadpp_AltShiftMenuFix/releases/download/v1.0/NppAltShiftMenuFix-1.0-x64.zip)
