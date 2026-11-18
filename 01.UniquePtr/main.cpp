#include <iostream>
#include <string>
#include <memory>
#include <windows.h>

using namespace std;

/*
==========================================================================================================================================
                                              Розумні покажчики. unique_ptr
==========================================================================================================================================

  Проблеми звичайних («сирих») покажчиків на динамічну пам'ять:
      - забули delete — витік пам'яті;
      - delete двічі для однієї адреси — аварійне завершення програми;
      - між new і delete вилетів виняток — delete не виконується, пам'ять втрачено;
      - з покажчика не видно, хто ВЛАСНИК об'єкта, тобто хто повинен викликати delete.

  РОЗУМНИЙ ПОКАЖЧИК (smart pointer) — об'єкт-обгортка над звичайним покажчиком, який сам звільняє пам'ять у своєму деструкторі.
  Деструктор викликається завжди, коли розумний покажчик знищується, у тому числі під час розкрутки стека після винятку. Це принцип RAII.
  Розумні покажчики оголошені в бібліотеці <memory>.

  unique_ptr<T> — розумний покажчик з ЄДИНИМ власником:
      - об'єкт належить рівно одному unique_ptr; коли той знищується — знищується і об'єкт;
      - КОПІЮВАТИ unique_ptr не можна (конструктор копіювання і оператор = видалені через delete),
        інакше два власники звільнили б одну пам'ять двічі;
      - ПЕРЕМІЩУВАТИ можна: move передає власність, а старий unique_ptr стає порожнім (nullptr);
      - займає стільки ж пам'яті і працює так само швидко, як звичайний покажчик.

  unique_ptr — вибір за замовчуванням для об'єкта в динамічній пам'яті. У сучасному C++ new і delete вручну майже не пишуть.

  Основні операції:
      make_unique<T>(аргументи)      створює об'єкт у динамічній пам'яті; аргументи передаються конструктору T
      ptr->method()   *ptr            доступ до об'єкта, як у звичайного покажчика
      if (ptr)                        true, якщо покажчик не порожній
      ptr.reset()                     знищити об'єкт достроково, покажчик стає порожнім
      unique_ptr<T> b = move(a);      передати власність від a до b

  ----------------------------------------------------------------------------------------------------------------------------------------

  Поле unique_ptr — сучасний спосіб записати КОМПОЗИЦІЮ: клас одноосібно володіє частиною, і частина знищується
  разом з ним автоматично, без delete у деструкторі.
  Особливо потрібне, коли частина задана ІНТЕРФЕЙСОМ: об'єкт абстрактного класу створити не можна, тому поле
  зберігає unique_ptr<IІнтерфейс>, а всередині лежить об'єкт будь-якого класу-нащадка.
  Таку частину можна замінити під час роботи програми: при присвоєнні нового unique_ptr старий об'єкт знищується.

*/

// Інтерфейс: спосіб доставки сповіщень.
// ПРИНЦИП ІНВЕРСІЇ ЗАЛЕЖНОСТЕЙ (DIP, літера D у SOLID): Client залежить від абстракції, а не від EmailNotifier чи TelegramNotifier
class INotifier
{
public:
    virtual ~INotifier() = default;              // віртуальний деструктор: unique_ptr<INotifier> знищить нащадка правильно
    virtual void send(const string& message) const = 0;
};

class EmailNotifier : public INotifier
{
private:
    string email;

public:
    explicit EmailNotifier(const string& email) : email(email)
    {}

    ~EmailNotifier()
    {
        cout << "  [Email-канал " << email << " закрито]" << endl;
    }

    void send(const string& message) const override
    {
        cout << "[Email " << email << "] " << message << endl;
    }
};

class TelegramNotifier : public INotifier
{
private:
    string nick;

public:
    explicit TelegramNotifier(const string& nick) : nick(nick)
    {}

    ~TelegramNotifier()
    {
        cout << "  [Telegram-канал " << nick << " закрито]" << endl;
    }

    void send(const string& message) const override
    {
        cout << "[Telegram " << nick << "] " << message << endl;
    }
};

// Клієнт інтернет-магазину
// ПАТЕРН СТРАТЕГІЯ (Strategy): спосіб доставки — взаємозамінна поведінка за спільним інтерфейсом
class Client
{
private:
    string name;
    unique_ptr<INotifier> notifier;              // КОМПОЗИЦІЯ: клієнт володіє каналом сповіщень

public:
    // ІН'ЄКЦІЯ ЗАЛЕЖНОСТІ ЧЕРЕЗ КОНСТРУКТОР (constructor injection): клієнт не створює канал сам, а отримує ззовні
    Client(const string& name, unique_ptr<INotifier> notifier)
        : name(name), notifier(move(notifier))   // забираємо власність у параметра
    {}

    // ІН'ЄКЦІЯ ЗАЛЕЖНОСТІ ЧЕРЕЗ СЕТЕР (setter injection); заміна поведінки під час роботи — суть патерну Стратегія
    void setNotifier(unique_ptr<INotifier> newNotifier)
    {
        notifier = move(newNotifier);            // старий канал знищується автоматично
    }

    void notify(const string& message) const
    {
        notifier->send(name + ", " + message);
    }
};

// ПРОСТА ФАБРИКА (Simple Factory, фабрична функція): обирає конкретний клас, повертає інтерфейс.
// Не плутати з патернами GoF «Фабричний метод» і «Абстрактна фабрика» — вони будуються на ієрархії класів-творців
unique_ptr<INotifier> createNotifier(const string& channel, const string& address)
{
    if (channel == "telegram")
    {
        return make_unique<TelegramNotifier>(address);
    }
    return make_unique<EmailNotifier>(address);
}

int main()
{
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    // ======================================== Основний синтаксис unique_ptr ========================================

    unique_ptr<INotifier> email = make_unique<EmailNotifier>("test@gmail.com");   // створення
    email->send("перевірка ->");                                                  // виклик методу
    (*email).send("перевірка *");                                                 // розіменування

    // unique_ptr<INotifier> copy = email;                 // ПОМИЛКА компіляції: копіювати не можна
    unique_ptr<INotifier> other = move(email);             // переміщення: власність переходить до other

    if (!email)                                            // перевірка на порожність
    {
        cout << "email тепер порожній" << endl;
    }

    other.reset();                                         // знищити об'єкт достроково
    cout << "other порожній: " << (other == nullptr) << endl;

    // ============================================== Програма ==============================================

    Client olena("Олена", createNotifier("email", "olena@gmail.com"));
    olena.notify("ваше замовлення відправлено");

    cout << "Олена змінила налаштування сповіщень:" << endl;
    olena.setNotifier(createNotifier("telegram", "@olena_k"));   // email-канал закривається тут
    olena.notify("замовлення чекає у відділенні");

    cout << "--- Кінець main ---" << endl;
    return 0;                                                    // telegram-канал закривається разом з olena
}

/*
==========================================================================================================================================
                                                  ІНСАЙТИ / ЛАЙФХАКИ / BEST PRACTICES
==========================================================================================================================================

  1. Без віртуального деструктора в інтерфейсі unique_ptr<INotifier> викличе лише деструктор INotifier, і частина
     об'єкта-нащадка не звільниться. Для інтерфейсу достатньо  virtual ~INotifier() = default;

  2. Клас з полем unique_ptr автоматично стає НЕКОПІЙОВАНИМ: Client copy = olena; не скомпілюється, бо не копіюється поле.
     Переміщувати такий клас можна. Якщо копія справді потрібна, доведеться написати конструктор копіювання самостійно.

  3. Якщо функція лише КОРИСТУЄТЬСЯ об'єктом, передавайте їй посилання (const INotifier&), а не unique_ptr.
     Приймати unique_ptr за значенням має сенс лише тоді, коли функція ЗАБИРАЄ власність, як setNotifier.

  4. Для динамічного масиву є форма unique_ptr<int[]> arr = make_unique<int[]>(10); — вона викликає delete[].
     Але на практиці замість динамічних масивів використовують vector (тема STL).

  5. ЗАСТАРІЛО: auto_ptr — перша спроба розумного покажчика в C++98. Під час КОПІЮВАННЯ він непомітно передавав
     власність і залишав оригінал порожнім, що породжувало помилки. Видалений зі стандарту в C++17,
     у сучасному коді не використовується; його повністю замінив unique_ptr.

==========================================================================================================================================
                                                              ПРАКТИКА
==========================================================================================================================================

  Завдання. Герой зі зброєю.
  Інтерфейс IWeapon і клас Sword уже готові — скопіюйте їх у свій файл:

      class IWeapon
      {
      public:
          virtual ~IWeapon() = default;
          virtual void attack() const = 0;
      };

      class Sword : public IWeapon
      {
      private:
          int damage;

      public:
          explicit Sword(int damage) : damage(damage)
          {}

          ~Sword()
          {
              cout << "Меч зламано" << endl;
          }

          void attack() const override
          {
              cout << "Удар мечем: " << damage << " шкоди" << endl;
          }
      };

    1. Напишіть клас Bow (нащадок IWeapon) за зразком Sword: деструктор виводить «Лук зламано»,
       attack() — «Постріл з лука: 15 шкоди».
    2. Напишіть клас Hero:
        поля:   string name, unique_ptr<IWeapon> weapon;
        конструктор з ім'ям і unique_ptr<IWeapon> (як Client у прикладі);
        метод void equip(unique_ptr<IWeapon> newWeapon) — замінює зброю (як setNotifier);
        метод void attack() const — виводить «Артур атакує!» і викликає weapon->attack().
    3. У main:
        - створіть героя «Артур» з мечем: make_unique<Sword>(25), і викличте attack();
        - дайте герою лук через equip(make_unique<Bow>(15)) і знову викличте attack();
        - виведіть «Кінець бою». Подивіться, у який момент зламався меч, а в який — лук.

  Бонус. Напишіть фабрику  unique_ptr<IWeapon> createWeapon(const string& type)  (як createNotifier):
         "bow" — повертає Bow з шкодою 15, інакше — Sword з шкодою 25. Використайте її в main замість make_unique.
*/
