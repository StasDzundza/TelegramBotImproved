Доброго дня, надсилаю виконану першу лабораторну роботу (друга також скоро буде)
За основу для рефакторингу взяв код своєї курсової роботи - telegram bot на c++, що дозволяє перекладати текст (як звичайний, так і той, що знаходиться на певних зорбраження)
https://github.com/StasDzundza/TelegramBot - посилання на старий код
https://github.com/StasDzundza/TelegramBotImproved - посилання на репозиторій, де я зробив рефакторинг

Зроблені зміни:
- Перехід на систему збірки Cmake (раніше був QMake). Це було зроблено, тому що CMake більш поширений та зручніший у користуванні, в тому числі  у подальному внесенні нових змін.
- Змінено деякі специфічні речі, що дозволяли коду запускатись тільки на ОС Windows. Зараз він успішно компілюється та запускається і на MacOS
- Виправлено всі warnings, про які повідомляв компілятор. Встановлено флаг збірки, який визначає будь-який ворнінг як помилку. (щоб писати гарний код у майбутньому)
- Перейменовано всі необхідні файли, змінні класів, методів та інших типів даних, щоб відповідати єдиному стандарту (я вибрав стандарт від Google)
- У деяких місцях видалено дефолтний конструктор SomeClass()=default; оскільки він є зайвим бо генерується компілятором за замовчуванням.
- Змінено старі типи конектів сигналів фреймворку Qt на оновлені.
- Видалено декілька зайвих умовних операторів if-else.
- Додано const qualifiers до всіх змінних, які ніде не змінюються в процесі виконання програми.
- Об'єднано класи TextReader та FileWriter (у якого був сього один метод, що є не зовсім доречним) в єдиний клас TextUtils (тепер цей клас містить методи для роботи з текстом)
- Видалено методи toString() у деяких класів. Натомість створено файл PrettyPrinters в якому містяться декілька перегружених методів toString для різних типів даних.
- Винесено всі константи, які були полями певних класів до анонімного неймспейсу (це гарна практика від Google)
- Видалено багато зайвих полів класу, що могли використовуватись тільки раз.
- Перетворено всі власні типи для сутностей, які визначає Telegram з класі до структур, що спростило роботу з ними, знизило кількість файлів та, відповідно, коду.
- Покращено парсинг json відповідей, які присилає telegram server.
- У багатьох місцях прибрано використання вказівників (які ще й не видалялись, що призводило до втрат пам'яті). Це зробио код надійнішим, ефективнішим та читабельнішим.
- У деяких місцях використано stringstream замість примутивної та незручної конкатенації текстових рядків.
- Спрощено сигнатури конструкторів деяких файлів.
- Прибрано секретний токен телеграм бота із публічного репозиторію (хто має токен - може робити з ботом будь-що). Тепер він парситься із змінної середовища. Завдяки цьому програма стала захищенішою.
- Структура директорій була змінена і стала дуже компактною та зрозумілою (в корені ще мала бути папка lib, але гітхаб не дозволив її запушити, бо там знаходяться файли трохи більшого розміру)
- Додано файл .clang-format з власними параметрами(максимальна довжина рядка, відступи...), який використовується для автоматичного форматування коду утилітою clang-format (дозволить зробити код візуально красивим та читабельнішим)

В резульататі пророблених змін код значно зменшився в об'ємі, але ні трохи не втратив функціоналу, а навпаки став тільки ефективнішим(за рахунок виправлень по втратах пам'яті) та читабельнішим (за рахунок дуже багатьох змін).

Також появились речі, які дозволять в майбутньому писати тільки кращий код (трактування компілятором warnings як помилок та інтегрування clang-format)
