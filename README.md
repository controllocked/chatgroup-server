## 🇷🇺 Русский

### 📌 Описание
Это простой многопользовательский чат-сервер на языке C, использующий `poll()` для одновременного обслуживания нескольких клиентов. Сервер принимает соединения, запрашивает никнейм и пересылает сообщения от одного клиента ко всем остальным.

### ⚙️ Особенности
- Поддержка до 100 клиентов одновременно  
- Проверка уникальности никнейма  
- Автоматическое отключение клиентов, не отправивших ник в течение 10 секунд  
- Цветной вывод с использованием ANSI  
- Чистая и понятная структура кода  

### 🧱 Зависимости
- Linux/Unix-подобная ОС  
- GCC компилятор  
- Заголовочные файлы: `<poll.h>`, `<sys/socket.h>`, `<netinet/in.h>`, и т.д.  

### 🛠️ Сборка
```
gcc -o chat_server main.c sockutils.c
```

### 🚀 Запуск
```
./chat_server <порт>
```

Пример:
```
./chat_server 12345
```

### 🧑‍💻 Использование
1. Подключитесь через `telnet` или любой TCP-клиент:
    ```
    telnet localhost 12345
    ```
2. Введите никнейм.  
3. Отправляйте сообщения - они будут передаваться другим пользователям.

### 📝 Примечания
- Никнейм должен состоять только из латинских букв (до 32 символов).  
- Клиенты без ника отключаются через 10 секунд после подключения.

---

## 🇬🇧 English

### 📌 Description
This is a simple multi-user chat server in C using `poll()` for handling multiple clients concurrently. It accepts new connections, requests a nickname, and broadcasts messages from one client to all others.

### ⚙️ Features
- Supports up to 100 concurrent clients  
- Ensures unique nicknames  
- Disconnects clients who don't send a nickname within 10 seconds  
- ANSI-colored message output  
- Clean and modular code structure  

### 🧱 Dependencies
- Linux/Unix-like OS  
- GCC compiler  
- Header files: `<poll.h>`, `<sys/socket.h>`, `<netinet/in.h>`, etc.  

### 🛠️ Build
```
gcc -o chat_server main.c sockutils.c
```

### 🚀 Run
```
./chat_server <port>
```

Example:
```
./chat_server 12345
```

### 🧑‍💻 Usage
1. Connect using `telnet` or any TCP client:
    ```
    telnet localhost 12345
    ```
2. Enter your nickname when prompted.  
3. Send messages - they will be broadcast to all other users.

### 📝 Notes
- Nicknames must consist of Latin letters only (max 32 characters).  
- Clients who do not send a nickname within 10 seconds are disconnected.

---

👨‍💻 Автор: Vadim Li
