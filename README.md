# Теория параллелизма, задание 1

## Техническое задание

Заполнить масив float/double значениями синуса (один период на всю длину массива), количество элементов 10^7. Посчитать сумму и вывести в терминал.

Написать make и cmake файлы для сборки. Сделать push на GitHub, в разных ветках разные сборщики. Должна быть возможность выбора типа массива (float или double) во время сборки.

В Readme файле укажите как выбирать тип массива, и какой вывод дали float и double варианты.

## Сборка с помощью Make

**Для типа данных float**:

```sh

   make float

```

**Для типа данных double**:

```sh

   make double

```

## Сборка с помощью CMake

**Для типа данных float**:

```sh

   mkdir build

   cd build

   cmake -DTYPE=float ..

   make

   ./sin_float

```

**Для типа данных double**:

```sh

   mkdir build

   cd build

   cmake -DTYPE=double ..

   make

   ./sin_double

```

## Запуск

- при запуске с типом данных float выводится 
```sh
Sum f :-0.0277862
```

- при запуске с типом данных double выводится 
```sh
Sum d :4.89582e-11
```
