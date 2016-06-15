/*
 * opt3001.h
 *
 *  Created on: 03 июня 2016 г.
 *      Author: developer
 */

#ifndef OPT3001_H_
#define OPT3001_H_


// инициализация
OPT_init();


// читаем результат
// возвращает 0 если мы пытаемся читать при не завершенном преобразовании ацп
// или если случилось переполнение
OPT_read();

// определение вышли ли мы из ракеты возвратит 1 когда вышли
OPT_start();
#endif /* OPT3001_H_ */
