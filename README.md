# Test Task Blue Pill board

Проект для автозагрузчика для МК STM32F103C8T6 на основе отладочной платы Blue Pill.
Проект выполнен в IAR (CMSIS). 

В проекте предусмотрены 3 версии прошивки: загрузчик и 2 приложения с разной частотой мигания светодиода.
Чтобы собрать проект для загрузчика в файле  main.h необходимо раскомментировать дефайн PROGRAM_BOOTLOADER и в настройках проекта выставить для линковщика файл конфигурации по умолчанию.
Для сборки приложений со светодиодом необходимо раскомментировать дефайн PROGRAM_VER1 или PROGRAM_VER2 и в настройках проекта выставить для линковщика файл конфигурации "$PROJ_DIR$\Project.icf".

Загрузка прошивки производится по интерфейсу UART.

Для заливки прошивки через загрузчик сначала отправляется команда "RUN!". 
Потом отправляются параметры прошивки 12Б: 
* HEADER (size = 12Б)
    * 2Б - id = 100
    * 2Б - block num = 0
    * 4Б - размер прошивки в байтах
    * 2Б - crc прошивки
    * 2Б - crc заголовка

Затем прошивка разбитая на куски пакетами по 128Б в формате:
* DATA PACKET (size = 128Б)
    * 2Б - id = 101
    * 2Б - block num = 1..n
    * 122Б - часть прошивки
    * 2Б - crc пакета

Когда прошивка получена полностью, программа проверяет CRC полной прошивки, и при успехе перепрыгивает на приложение.

Для возврата из приложения к загрузчику необходимо отправить команду "BOOT".

##### UART не протестирован, так как ножки у платы не припаены.
