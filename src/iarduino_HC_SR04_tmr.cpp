#include "iarduino_HC_SR04_tmr.h"																				//
																												//
volatile	iarduino_HC_SR04_tmr_VolatileClass IHCSR04TVC;														//	Создаём объект IHCSR04TVC.
																												//
//		КОНСТРУКТОР КЛАССА:																						//
		iarduino_HC_SR04_tmr::iarduino_HC_SR04_tmr(uint8_t trig, uint8_t echo){									//	Параметры:				«trig» - номер вывода TRIG, «echo» - номер вывода ECHO.
			static uint8_t sumObj=0;																			//	Определяем количество созданных экземпляров класса iarduino_HC_SR04_tmr.
		//	Если подключено менее 4 датчиков:																	//
			if(sumObj<4){																						//	Если уже создано менее 4 экземпляров класса iarduino_HC_SR04_tmr, то ...
				numObj = sumObj;																				//	Сохраняем номер текущего экземпляра класса iarduino_HC_SR04_tmr.
				IHCSR04TVC.sumObj = ++sumObj;																	//	Увеличиваем количество созданных экземпляров класса iarduino_HC_SR04_tmr.
			//	Сохраняем адреса портов и маски выводов:														//
			#if defined(ESP32)																					//
				IHCSR04TVC.pinTRIG_PRT[numObj] = (uint32_t*)portOutputRegister (digitalPinToPort(trig));		//	Сохраняем адрес регистра выходных значений вывода «trig».
				IHCSR04TVC.pinECHO_PRT[numObj] = (uint32_t*)portInputRegister  (digitalPinToPort(echo));		//	Сохраняем адрес регистра входных  значений вывода «echo».
			#else																								//
				IHCSR04TVC.pinTRIG_PRT[numObj] = (uint16_t*)portOutputRegister (digitalPinToPort(trig));		//	Сохраняем адрес регистра выходных значений вывода «trig».
				IHCSR04TVC.pinECHO_PRT[numObj] = (uint16_t*)portInputRegister  (digitalPinToPort(echo));		//	Сохраняем адрес регистра входных  значений вывода «echo».
			#endif																								//
				IHCSR04TVC.pinTRIG_MSK[numObj] = digitalPinToBitMask(trig);										//	Сохраняем маску вывода «trig» в регистре выходных значений.
				IHCSR04TVC.pinECHO_MSK[numObj] = digitalPinToBitMask(echo);										//	Сохраняем маску вывода «echo» в регистре входных  значений.
			//	Конфигурируем выводы:																			//
				pinMode(trig, OUTPUT);																			//	Конфигурируем вывод «trig» как выход.
				pinMode(echo, INPUT );																			//	Конфигурируем вывод «echo» как вход.
			//	Устанавливаем «0» на выводе «trig»:																//
				*IHCSR04TVC.pinTRIG_PRT[numObj] &= ~IHCSR04TVC.pinTRIG_MSK[numObj];								//	Устанавливаем низкий логический уровень на выводе «trig».
			//	Отключаем датчик:																				//	
				work(false);																					//	Включим при инициализации.
			}																									//
}																												//
																												//
//		ИНИЦИАЛИЗАЦИЯ ДАТЧИКА:																					//	Возвращаемое значение:	нет.
void	iarduino_HC_SR04_tmr::begin(uint16_t period){															//	Параметр:				«period» - период опроса датчика в мс (от 50 до 3000).
			if( period<50   ){ period=50;   }																	//
			if( period>3000 ){ period=3000; }																	//
		//	Настраиваем и запускаем таймер:																		//
			Timer_Begin(20000);																					//	Настраиваем таймер на работу с частотой прерываний 20'000 Гц (каждые 50мкс).
		//	Определяем количество прерываний таймера между опросами датчика:									//
			IHCSR04TVC.maxINT[numObj]=period*20;																//	Количество прерываний = период в секундах * 20'000 Гц = период в мс * 20 кГц.
			IHCSR04TVC.valECHO_DATA[numObj]=600;																//	Если модуль не подключён или опрашивается впервые, то результатом будет не 0см, а 400см (максимум).
		//	Включаем опрос датчика:																				//
			work(true);																							//	Разрешаем опрос датчика.
}																												//
																												//
//		ВКЛЮЧЕНИЕ И ОТКЛЮЧЕНИЕ ДАТЧИКА:																			//	Возвращаемое значение:	нет.
void	iarduino_HC_SR04_tmr::work(bool f){																		//	Параметр:				«f» - флаг.
			if( IHCSR04TVC.state			[numObj]!= f ){														//	Если состояние датчика «state[numObj]» отличается от состояния флага «f», то ...
				IHCSR04TVC.state			[numObj] = f;														//	Разрешаем или запрещаем опрос датчика.
				IHCSR04TVC.flgECHO_LEVEL	[numObj] = 0;														//	Сбрасываем предыдущий логический уровень вывода ECHO.
				IHCSR04TVC.cntECHO_HIGH		[numObj] = 0;														//	Сбрасываем количество прерываний вызваных при наличии «1» на выводе ECHO.
				IHCSR04TVC.cntECHO_LOW		[numObj] = IHCSR04TVC.maxINT[numObj];								//	Устанавливаем максимальное количество прерываний вызваных при наличии «0» на выводе ECHO.
			}																									//
}																												//
																												//
//		ОБРАБОТКА ПРЕРЫВАНИЙ ТАЙМЕРА:																			//
		Timer_Callback(Timer_Argument){																			//
			volatile uint8_t	i=IHCSR04TVC.sumObj;															//	Определяем переменную «i» присвоив ей количество используемых датчиков.
			volatile bool		start = false;																	//	Определяем флаг указывающий на то, что был запущен один из датчиков.
			volatile bool		echo;																			//	Объявляем переменную «echo» для хранения логического уровня на выводе ECHO.
			while( i-- ){																						//	Проходим по всем датчикам в обратном порядке.
				if( IHCSR04TVC.state[i] ){																		//	Если датчик «i» включён.
				//	Читаем логический уровень:																	//
					echo = (*IHCSR04TVC.pinECHO_PRT[i]&IHCSR04TVC.pinECHO_MSK[i])==0?0:1;						//	Читаем логический уровень с вывода ECHO.
				//	Приращаем счетчики прерываний таймера:														//
					if(echo){IHCSR04TVC.cntECHO_HIGH[i]++;}														//	Если на выводе ECHO установлена «1», то приращаем счётчик «cntECHO_HIGH».
					else    {IHCSR04TVC.cntECHO_LOW [i]++;}														//	Если на выводе ECHO установлен  «0», то приращаем счётчик «cntECHO_LOW».
				//	Обрабатываем переполнение счётчика «cntECHO_LOW»:											//
					if((IHCSR04TVC.cntECHO_LOW[i]>IHCSR04TVC.maxINT[i])&&(start==false)){						//	Если счетчик прерываний вызваных при наличии «0» на выводе ECHO превысил максимальное значение и флаг «start» сброшен.
						IHCSR04TVC.cntECHO_LOW[i]=0;                      start= true;							//	Сбрасываем счетчик прерываний вызваных при наличии «0» на выводе ECHO и устанавливаем флаг «start».
					//	Запускаем датчик:																		//
						*IHCSR04TVC.pinTRIG_PRT[i] |= IHCSR04TVC.pinTRIG_MSK[i];								//	Устанавливаем «1» на выводе TRIG.
						delayMicroseconds(10);																	//	Ждём 10 мкс
						*IHCSR04TVC.pinTRIG_PRT[i] &=~IHCSR04TVC.pinTRIG_MSK[i];								//	Устанавливаем «0» на выводе TRIG.
					}																							//
				//	Обрабатываем переполнение счётчика «cntECHO_HIGH»:											//
					if( IHCSR04TVC.cntECHO_HIGH[i]>600){														//	Если счетчик прерываний вызваных при наличии «1» на выводе ECHO превысил значение 600. Примечание: 600 прерываний * (1 / 20'000 Гц) = 30 мс.
						IHCSR04TVC.cntECHO_HIGH[i]=600;															//	Запрещаем приращение счётчика прерываний вызваных при наличии «1» на выводе ECHO.
						IHCSR04TVC.valECHO_DATA[i]=600;															//	Сохраняем значение счётчика количества прерываний вызваных при наличии «1» на выводе ECHO в переменную valECHO_DATA.
					}																							//
				//	Обрабатываем измнение уровей на выводе ECHO:												//
					if( IHCSR04TVC.flgECHO_LEVEL[i]!=echo ){													//	Если текущий логический уровень вывода ECHO не совпадает с предыдущим.
						if(echo){																				//	Если уровень на вывода ECHO сменился с «0» на «1», то ...
							IHCSR04TVC.cntECHO_HIGH[i]=1;														//	Сбрасываем счётчик количества прерываний вызваных при наличии «1» на выводе ECHO.
							IHCSR04TVC.cntECHO_LOW [i]=0;														//	Сбрасываем счётчик количества прерываний вызваных при наличии «0» на выводе ECHO.
						}else{																					//	Если уровень на вывода ECHO сменился с «1» на «0», то ...
							IHCSR04TVC.valECHO_DATA[i]=IHCSR04TVC.cntECHO_HIGH[i];								//	Сохраняем значение счётчика количества прерываний вызваных при наличии «1» на выводе ECHO в переменную valECHO_DATA.
							IHCSR04TVC.cntECHO_LOW [i]=1;														//	Сбрасываем счётчик количества прерываний вызваных при наличии «0» на выводе ECHO.
						}																						//
					}																							//
				//	Сохраняем логический уровень:																//
					IHCSR04TVC.flgECHO_LEVEL[i]=echo;															//	Сохраняем текущий логический уровень вывода ECHO как предыдущий.
				}																								//
			}																									//
}																												//
																												//
//		ОПРЕДЕЛЕНИЕ РАССТОЯНИЯ:																					//	Возвращаемое значение:	Расстояние до препятствия в см.
long	iarduino_HC_SR04_tmr::distance(int8_t t){																//	Параметр:				«t» - текущая температура в °C.
		//	Определяем расстояние:																				//
			long l = IHCSR04TVC.valECHO_DATA[numObj]*50*sqrt(273+t)/100;										//	Определяем расстояние в сантиметрах: L = длительность в мкс * корень(t+273) / 1000, где длительность = количество прерываний * 50 мкс.
			if( l>4000 ){ l=4000; }																				//	Ограничиваем расстояние значением в 400 см.
			if( averaging==0 ){ return l; }																		//
		//	Усредняем результат:																				//
			valData *= float(averaging)-1.0;																	//	Умножаем предыдущий результат на коэффициент усреднения - 1.
			valData += float(l);																				//	Добавляем к полученному значению текущий результат.
			valData /= float(averaging);																		//	Делим полученное значение на коэффициент усреднения.
			return (long) valData;																				//	Выводим усреднённый результат.
}																												//
