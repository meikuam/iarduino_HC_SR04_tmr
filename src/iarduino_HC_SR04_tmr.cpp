#include "iarduino_HC_SR04_tmr.h"

//		ОПРЕДЕЛЯЕМ ПЕРЕМЕННЫЕ STATIC:																			//	
static volatile	uint8_t		iarduino_HC_SR04_tmr::sumObj			= 0;										//	Определяем переменную для хранения количества подключённых датчиков.
static volatile	uint8_t*	iarduino_HC_SR04_tmr::pinTRIG_PRT	[4]	= {0,0,0,0};								//	Определяем массив для хранения указателя на адрес  регистра выходных значений вывода TRIG.
static volatile	uint8_t*	iarduino_HC_SR04_tmr::pinECHO_PRT	[4]	= {0,0,0,0};								//	Определяем массив для хранения указателя на адрес  регистра входных  значений вывода ECHO.
static volatile	uint8_t		iarduino_HC_SR04_tmr::pinTRIG_MSK	[4]	= {0,0,0,0};								//	Определяем массив для хранения маски вывода TRIG в регистре выходных значений.
static volatile	uint8_t		iarduino_HC_SR04_tmr::pinECHO_MSK	[4]	= {0,0,0,0};								//	Определяем массив для хранения маски вывода ECHO в регистре входных  значений.
static volatile	uint16_t	iarduino_HC_SR04_tmr::cntECHO_HIGH	[4]	= {0,0,0,0};								//	Определяем массив для подсчёта количества прерываний вызваных при наличии «1» на выводе ECHO.
static volatile	uint16_t	iarduino_HC_SR04_tmr::cntECHO_LOW	[4]	= {0,0,0,0};								//	Определяем массив для подсчёта количества прерываний вызваных при наличии «0» на выводе ECHO.
static volatile	uint16_t	iarduino_HC_SR04_tmr::valECHO_DATA	[4]	= {0,0,0,0};								//	Определяем массив для хранения количества прерываний вызваных при наличии «1» на выводе ECHO.
static volatile	bool		iarduino_HC_SR04_tmr::flgECHO_LEVEL	[4]	= {0,0,0,0};								//	Определяем массив для хранения предыдущего логического уровня на выводе ECHO.
static volatile	bool		iarduino_HC_SR04_tmr::state			[4]	= {0,0,0,0};								//	Определяем массив для хранения состояния датчика (вкл/выкл).
																												//
//		КОНСТРУКТОР КЛАССА:																						//
		iarduino_HC_SR04_tmr::iarduino_HC_SR04_tmr(uint8_t trig, uint8_t echo){									//	Параметры:				«trig» - номер вывода TRIG, «echo» - номер вывода ECHO.
		//	Если подключено менее 4 датчиков:																	//
			if(sumObj<4){																						//	Если уже создано менее 4 экземпляров класса iarduino_HC_SR04_tmr, то ...
				numObj = sumObj;																				//	Сохраняем номер текущего экземпляра класса iarduino_HC_SR04_tmr.
				sumObj++;																						//	Увеличиваем количество созданных экземпляров класса iarduino_HC_SR04_tmr.
			//	Сохраняем адреса портов и маски выводов:														//
				pinTRIG_PRT[numObj] = portOutputRegister (digitalPinToPort(trig));								//	Сохраняем адрес регистра выходных значений вывода «trig».
				pinECHO_PRT[numObj] = portInputRegister  (digitalPinToPort(echo));								//	Сохраняем адрес регистра входных  значений вывода «echo».
				pinTRIG_MSK[numObj] = digitalPinToBitMask(digitalPinToPort(trig));								//	Сохраняем маску вывода «trig» в регистре выходных значений.
				pinECHO_MSK[numObj] = digitalPinToBitMask(digitalPinToPort(echo));								//	Сохраняем маску вывода «echo» в регистре входных  значений.
			//	Конфигурируем выводы:																			//
				pinMode(trig, OUTPUT);																			//	Конфигурируем вывод «trig» как выход.
				pinMode(echo, INPUT );																			//	Конфигурируем вывод «echo» как вход.
			//	Устанавливаем «0» на выводе «trig»:																//
				*pinTRIG_PRT[numObj] &= ~pinTRIG_MSK[numObj];													//	Устанавливаем низкий логический уровень на выводе «trig».
			//	Настраиваем и запускаем 2 таймер:																//
				if(sumObj==1){																					//	Настраиваем таймер только 1 раз, при создании первого экземпляра класса iarduino_HC_SR04_tmr.
					funcSetTimer2(20000);																		//	Настраиваем таймер на работу с частотой прерываний 20'000 Гц (каждые 50мкс).
				}																								//
			//	Включаем опрос датчика:																			//
				state[numObj] = true;																			//	Разрешаем опрос датчика.
			}																									//
}																												//
																												//
//		ВКЛЮЧЕНИЕ И ОТКЛЮЧЕНИЕ ДАТЧИКА:																			//	Возвращаемое значение:	нет.
void	iarduino_HC_SR04_tmr::work(bool f){																		//	Параметр:				«f» - флаг.
			state			[numObj]=f;																			//
			cntECHO_HIGH	[numObj]=0;																			//	Сбрасываем количество прерываний вызваных при наличии «1» на выводе ECHO.
			cntECHO_LOW		[numObj]=0;																			//	Сбрасываем количество прерываний вызваных при наличии «0» на выводе ECHO.
			valECHO_DATA	[numObj]=0;																			//	Сбрасываем количество сохранённых прерываний вызваных при наличии «1» на выводе ECHO.
			flgECHO_LEVEL	[numObj]=0;																			//	Сбрасываем предыдущий логический уровень вывода ECHO.
}																												//
																												//
//		УСТАНОВКА ЗНАЧЕНИЙ РЕГИСТРОВ ВТОРОГО ТАЙМЕРА:															//	Возвращаемое значение:	нет.
void	iarduino_HC_SR04_tmr::funcSetTimer2(uint32_t freq){														//	Параметр:				«freq» - частота прерываний таймера в Гц.
			uint16_t pre;																						//	Объявляем переменную «pre» для определения значения предделителя: 1,8,32,64,128,256,1024.
			uint8_t  bits_CS2;																					//	Объявляем переменную «bitCS2X» для определения значений битов CS22-CS20 в регистре TCCR2B.
			if(F_CPU/255/  1<freq)	{pre=   1; bits_CS2=1;}else													//	Если требуются прерывания с частотой выше 62'745 Гц, то устанавливаем предделитель «pre» в    1, следовательно (при F_CPU=16 МГц), прерывания будут следовать с частотой от 62'745 Гц (при OCR2A=254) до 16'000'000 Гц (при OCR2A=  0), точность настройки ± Гц.
			if(F_CPU/255/  8<freq)	{pre=   8; bits_CS2=2;}else													//	Если требуются прерывания с частотой выше  7'843 Гц, то устанавливаем предделитель «pre» в    8, следовательно (при F_CPU=16 МГц), прерывания будут следовать с частотой от  7'843 Гц (при OCR2A=254) до     62'500 Гц (при OCR2A= 30), точность настройки ± Гц.
			if(F_CPU/255/ 32<freq)	{pre=  32; bits_CS2=3;}else													//	Если требуются прерывания с частотой выше  1'960 Гц, то устанавливаем предделитель «pre» в   32, следовательно (при F_CPU=16 МГц), прерывания будут следовать с частотой от  1'960 Гц (при OCR2A=254) до      7'812 Гц (при OCR2A= 63), точность настройки ± Гц.
			if(F_CPU/255/ 64<freq)	{pre=  64; bits_CS2=4;}else													//	Если требуются прерывания с частотой выше    980 Гц, то устанавливаем предделитель «pre» в   64, следовательно (при F_CPU=16 МГц), прерывания будут следовать с частотой от    980 Гц (при OCR2A=254) до      1'953 Гц (при OCR2A=127), точность настройки ± Гц.
			if(F_CPU/255/128<freq)	{pre= 128; bits_CS2=5;}else													//	Если требуются прерывания с частотой выше    490 Гц, то устанавливаем предделитель «pre» в  128, следовательно (при F_CPU=16 МГц), прерывания будут следовать с частотой от    490 Гц (при OCR2A=254) до        976 Гц (при OCR2A=127), точность настройки ± Гц.
			if(F_CPU/255/256<freq)	{pre= 256; bits_CS2=6;}else													//	Если требуются прерывания с частотой выше    245 Гц, то устанавливаем предделитель «pre» в  256, следовательно (при F_CPU=16 МГц), прерывания будут следовать с частотой от    245 Гц (при OCR2A=254) до        488 Гц (при OCR2A=127), точность настройки ± Гц.
									{pre=1024; bits_CS2=7;}														//	Если требуются прерывания с частотой ниже    245 Гц, то устанавливаем предделитель «pre» в 1024, следовательно (при F_CPU=16 МГц), прерывания будут следовать с частотой от     61 Гц (при OCR2A=255) до        244 Гц (при OCR2A= 63), точность настройки ± Гц.
			TCCR2A	= 0<<COM2A1	| 0<<COM2A0	| 0<<COM2B1	| 0<<COM2B0	| 1<<WGM21	| 0<<WGM20;						//	Биты COM2... = «0» (каналы A и B таймера отключены), биты WGM21 и WGM20 = «10» (таймер 2 в режиме CTC).
			TCCR2B	= 0<<FOC2A	| 0<<FOC2B	| 0<<WGM22	| bits_CS2;												//	Биты FOC2... = «0» (без принудительной установки результата сравнения), бит WGM22 = «0» (таймер 2 в режиме CTC), биты CS22-CS20 = bits_CS2 (биты определяют значение предделителя).
			OCR2A	= (uint8_t)(F_CPU/(pre*freq))-1;															//	Значение регистра сравнения OCR2A настраивается под частоту переполнения счётного регистра TCNT2.  freq=F_CPU/(предделитель*(OCR2A+1)) => OCR2A = (F_CPU/(предделитель*freq))-1
			TIMSK2	= 0<<OCIE2B	| 1<<OCIE2A	| 0<<TOIE2;															//	Разрешаем прерывание по совпадению счётного регистра TCNT2 и регистра сравнения OCR2A.
			SREG	= 1<<7;																						//	Устанавливаем флаг глобального разрешения прерываний.
}																												//
																												//
/*
static volatile	uint8_t		iarduino_HC_SR04_tmr::sumObj			= 0;					//	Определяем переменную для хранения количества подключённых датчиков.
static volatile	uint8_t*	iarduino_HC_SR04_tmr::pinTRIG_PRT	[4]	= {0,0,0,0};			//	Определяем массив для хранения указателя на адрес  регистра выходных значений вывода TRIG.
static volatile	uint8_t*	iarduino_HC_SR04_tmr::pinECHO_PRT	[4]	= {0,0,0,0};			//	Определяем массив для хранения указателя на адрес  регистра входных  значений вывода ECHO.
static volatile	uint8_t		iarduino_HC_SR04_tmr::pinTRIG_MSK	[4]	= {0,0,0,0};			//	Определяем массив для хранения маски вывода TRIG в регистре выходных значений.
static volatile	uint8_t		iarduino_HC_SR04_tmr::pinECHO_MSK	[4]	= {0,0,0,0};			//	Определяем массив для хранения маски вывода ECHO в регистре входных  значений.
static volatile	uint16_t	iarduino_HC_SR04_tmr::cntECHO_HIGH	[4]	= {0,0,0,0};			//	Определяем массив для подсчёта количества прерываний вызваных при наличии «1» на выводе ECHO.
static volatile	uint16_t	iarduino_HC_SR04_tmr::cntECHO_LOW	[4]	= {0,0,0,0};			//	Определяем массив для подсчёта количества прерываний вызваных при наличии «0» на выводе ECHO.
static volatile	uint16_t	iarduino_HC_SR04_tmr::valECHO_DATA	[4]	= {0,0,0,0};			//	Определяем массив для хранения количества прерываний вызваных при наличии «1» на выводе ECHO.
static volatile	bool		iarduino_HC_SR04_tmr::flgECHO_LEVEL	[4]	= {0,0,0,0};			//	Определяем массив для хранения предыдущего логического уровня на выводе ECHO.
static volatile	bool		iarduino_HC_SR04_tmr::state			[4]	= {0,0,0,0};			//	Определяем массив для хранения состояния датчика (вкл/выкл).
*/
																												//
//		ОБРАБОТКА ПРЕРЫВАНИЙ 2 ТАЙМЕРА:																			//	Возвращаемое значение:	нет.
		ISR(TIMER2_COMPA_vect){																					//	Параметр:				Вектор прерывания по совпадению счётного регистра TCNT2 и регистра сравнения OCR2A.
			volatile uint8_t	i=iarduino_HC_SR04_tmr::sumObj;													//	Определяем переменную «i» присвоив ей количество используемых датчиков.
			volatile bool		start = false;																	//	Определяем флаг указывающий на то, что был запущен один из датчиков.
			volatile bool		echo;																			//	Объявляем переменную «echo» для хранения логического уровня на выводе ECHO.
			while( i-- ){																						//	Проходим по всем датчикам в обратном порядке.
				if( iarduino_HC_SR04_tmr::state[i] ){															//	Если датчик «i» включён.
				//	Читаем логический уровень:																	//
					echo = (*iarduino_HC_SR04_tmr::pinECHO_PRT[i]&iarduino_HC_SR04_tmr::pinECHO_MSK[i])==0?0:1;	//	Читаем логический уровень с вывода ECHO.
				//	Приращаем счетчики прерываний таймера:														//
					if(echo){iarduino_HC_SR04_tmr::cntECHO_HIGH[i]++;}											//	Если на выводе ECHO установлена «1», то приращаем счётчик «cntECHO_HIGH».
					else    {iarduino_HC_SR04_tmr::cntECHO_LOW [i]++;}											//	Если на выводе ECHO установлен  «0», то приращаем счётчик «cntECHO_LOW».
				//	Запускаем датчик:																			//
					if((iarduino_HC_SR04_tmr::cntECHO_LOW[i]>1000)&&(start==false)){							//	Если счетчик прерываний вызваных при наличии «0» на выводе ECHO превысил значение 1000 и флаг «start» сброшен. 1'000 прерываний * (1 / 20'000 Гц) = 50 мс.
						iarduino_HC_SR04_tmr::cntECHO_LOW[i]=0;      start= true;								//	Сбрасываем счетчик прерываний вызваных при наличии «0» на выводе ECHO и устанавливаем флаг «start».
						*iarduino_HC_SR04_tmr::pinTRIG_PRT[i] |= iarduino_HC_SR04_tmr::pinTRIG_MSK[i];			//	Устанавливаем «1» на выводе TRIG.
						delayMicroseconds(10);																	//	Ждём 10 мкс
						*iarduino_HC_SR04_tmr::pinTRIG_PRT[i] &=~iarduino_HC_SR04_tmr::pinTRIG_MSK[i];			//	Устанавливаем «0» на выводе TRIG.
					}																							//
				//	Обрабатываем измнение уровей на выводе ECHO:												//
					if( iarduino_HC_SR04_tmr::flgECHO_LEVEL[i]!=echo ){											//	Если текущий логический уровень вывода ECHO не совпадает с предыдущим.
						if(echo){																				//	Если уровень на вывода ECHO сменился с «0» на «1», то ...
							iarduino_HC_SR04_tmr::cntECHO_HIGH[i]=1;											//	Сбрасываем счётчик количества прерываний вызваных при наличии «1» на выводе ECHO.
							iarduino_HC_SR04_tmr::cntECHO_LOW [i]=0;											//	Сбрасываем счётчик количества прерываний вызваных при наличии «0» на выводе ECHO.
						}else{																					//	Если уровень на вывода ECHO сменился с «1» на «0», то ...
							iarduino_HC_SR04_tmr::valECHO_DATA[i]=iarduino_HC_SR04_tmr::cntECHO_HIGH[i];		//	Сохраняем значение счётчика количества прерываний вызваных при наличии «1» на выводе ECHO в переменную valECHO_DATA.
							iarduino_HC_SR04_tmr::cntECHO_LOW [i]=1;											//	Сбрасываем счётчик количества прерываний вызваных при наличии «0» на выводе ECHO.
						}																						//
					}																							//
				//	Сохраняем логический уровень:																//
					iarduino_HC_SR04_tmr::flgECHO_LEVEL[i]=echo;												//	Сохраняем текущий логический уровень вывода ECHO как предыдущий.
				}																								//
			}																									//
}																												//
																												//
//		определение расстояния
long	iarduino_HC_SR04_tmr::distance(int8_t i){						//	i: текущая температура в °C
	return (long) cntECHO_HIGH[0];
//	return (long) cntECHO_LOW[0];
/*
		long		j=funDuration();								//	Считываем длительность импульса на выводе ECHO
		if(j<400){	j=funDuration();}								//	Повторно считываем длительность импульса (для подавления "глюков" некоторых датчиков)
					j=j*sqrt(273+i)/1000;							//	Определяем расстояние: L = Echo * корень(t+273) / 1000
		if(j>400){	j=400;}											//	Ограничиваемся максимально допустимым расстоянием
		if(averaging<=0){valDATA=float(j);}else{					//	Если усреднение результата не требуется, то текущий результат и есть выводимый результат, иначе ...
			valDATA *= float(averaging);							//	Умножаем предыдущий результат на коэффициент усреднения - 1
			valDATA += j;											//	Добавляем к полученному значению текущий результат
			valDATA /= float(averaging)+1.0;						//	Делим полученное значение на коэффициент усреднения
		}															//
		return long(valDATA);										//	Выводим результат
*/
}
/*
//		определение длительности
long	iarduino_HC_SR04_tmr::funDuration(){
		digitalWrite(pinTRIG, HIGH);								//	Устанавливаем уровень логической «1» на выводе TRIG
		delayMicroseconds(10);										//	Ждём 10 мкс
		digitalWrite(pinTRIG, LOW);									//	Устанавливаем уровень логического «0» на выводе TRIG
		return pulseIn(pinECHO, HIGH);								//	Считываем длительность импульса на выводе ECHO
}
*/