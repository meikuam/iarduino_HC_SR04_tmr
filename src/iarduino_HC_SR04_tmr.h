//	Библиотека для работы с ультразвуковым датчиком расстояния HC-SR04: http://iarduino.ru/shop/Sensory-Datchiki/ultrazvukovogo-datchika-hc-sr04-rasstoyaniya-dvizheniya.html
//  Версия: 1.0.2
//  Последнюю версию библиотеки Вы можете скачать по ссылке: https://iarduino.ru/file/548.html
//  Подробное описание функции бибилиотеки доступно по ссылке: http://wiki.iarduino.ru/page/ultrazvukovoy-datchik-izmereniya-rasstoyaniya-hc-sr04/
//  Библиотека является собственностью интернет магазина iarduino.ru и может свободно использоваться и распространяться!
//  При публикации устройств или скетчей с использованием данной библиотеки, как целиком, так и её частей,
//  в том числе и в некоммерческих целях, просим Вас опубликовать ссылку: http://iarduino.ru
//  Автор библиотеки: Панькин Павел
//  Если у Вас возникли технические вопросы, напишите нам: shop@iarduino.ru

#ifndef iarduino_HC_SR04_tmr_h
#define iarduino_HC_SR04_tmr_h

#if defined(ARDUINO) && (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#if defined(ESP32)														//
	static hw_timer_t *Esp32Timer = timerBegin(2, 3, true);				//	Определяем структуру настройки 2 таймера, предделитель = 3 (потом его изменим), флаг = true - счёт вперёд.
	extern void timer_callback_ESP32(void);								//	Подключаем функцию обработки прерываний 2 таймера для плат ESP32.
#elif defined(ESP8266)													//
	extern void timer_callback_ESP8266(void);							//	Подключаем функцию обработки прерываний 1 таймера для плат ESP8266.
#elif defined(RENESAS_CORTEX_M4)										//
	#include <FspTimer.h>												//	Подключаем библиотеку управления таймерами для плат Arduino UNO R4.
	static FspTimer objTimer;											//	Создаём объект для работы с таймером.
	extern void timer_callback_R4(timer_callback_args_t*);				//	Подключаем функцию обработки прерываний таймера для плат Arduino UNO R4.
#endif																	//
																		//
class	iarduino_HC_SR04_tmr{											//
	public:					iarduino_HC_SR04_tmr(uint8_t, uint8_t	);	//	Конструктор класса								(вывод TRIG, вывод ECHO)
		void				begin				(uint16_t=50		);	//	Объявляем функцию инициализации датчика			(период опроса датчика в мс) от 50 до 3000.
		void				work				(bool				);	//	Объявляем функцию включения/отключения датчика	(флаг)
		long				distance			(int8_t=23			);	//	Объявляем функцию получения расстояния			([t°C]) от ±127.
		long				averaging			= 0;					//	Определяем коэффициент усреднения показаний		(0-без усреднений, 1-минимальное усреднение, ... 100-сильное усреднение, ... 2147483648-через день показания дойдут до реальных)
	private:															//
		#include			"iarduino_HC_SR04_tmr_Timer.h"				//	Подключаем функцию конфигурирования таймера Timer_Begin( частота Гц ).
		volatile uint8_t	numObj				= 0;					//	Определяем переменную для хранения номера экземпляра данного класса.
		float				valData;									//	Переменная для рассчета усреднённого результата.
};																		//
																		//
class iarduino_HC_SR04_tmr_VolatileClass{								//
	public:																//
		volatile	uint8_t	sumObj;										//	Объявляем  переменную для хранения количества подключённых датчиков (будет определена в конструкторе).
	#if defined(ESP32)													//
		volatile uint32_t*	pinTRIG_PRT			[4];					//	Объявляем  массив для хранения указателя на адрес  регистра выходных значений вывода TRIG.
		volatile uint32_t*	pinECHO_PRT			[4];					//	Объявляем  массив для хранения указателя на адрес  регистра входных  значений вывода ECHO.
		volatile uint32_t	pinTRIG_MSK			[4];					//	Объявляем  массив для хранения маски вывода TRIG в регистре выходных значений.
		volatile uint32_t	pinECHO_MSK			[4];					//	Объявляем  массив для хранения маски вывода ECHO в регистре входных  значений.
	#else																//
		volatile uint16_t*	pinTRIG_PRT			[4];					//	Объявляем  массив для хранения указателя на адрес  регистра выходных значений вывода TRIG.
		volatile uint16_t*	pinECHO_PRT			[4];					//	Объявляем  массив для хранения указателя на адрес  регистра входных  значений вывода ECHO.
		volatile uint16_t	pinTRIG_MSK			[4];					//	Объявляем  массив для хранения маски вывода TRIG в регистре выходных значений.
		volatile uint16_t	pinECHO_MSK			[4];					//	Объявляем  массив для хранения маски вывода ECHO в регистре входных  значений.
	#endif																//
		volatile uint16_t	cntECHO_HIGH		[4];					//	Объявляем  массив для подсчёта количества прерываний вызваных при наличии «1» на выводе ECHO.
		volatile uint16_t	cntECHO_LOW			[4];					//	Объявляем  массив для подсчёта количества прерываний вызваных при наличии «0» на выводе ECHO.
		volatile uint16_t	valECHO_DATA		[4];					//	Объявляем  массив для хранения количества прерываний вызваных при наличии «1» на выводе ECHO.
		volatile bool		flgECHO_LEVEL		[4];					//	Объявляем  массив для хранения предыдущего логического уровня на выводе ECHO.
		volatile bool		state				[4];					//	Объявляем  массив для хранения состояния датчика (вкл/выкл).
		volatile uint16_t	maxINT				[4];					//	Объявляем  массив для хранения количества прерываний таймера между опросом датчика.
};																		//
																		//
#endif																	//