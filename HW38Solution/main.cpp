#include <iostream>
#include <string>
#include <memory>
#include <windows.h>

using namespace std;

/*
==========================================================================================================================================
                                            ДОМАШНЄ ЗАВДАННЯ 38 тема: розумні покажчики.
==========================================================================================================================================
*/

//   3. Навіщо в інтерфейсі IGem віртуальний деструктор? Що піде не так, якщо його прибрати?
class IGem
{
public:
    virtual ~IGem() = default;
    virtual string getName() const = 0;
    virtual int getBonus() const = 0;          // додаткова шкода
    virtual string getEffect() const = 0;      // опис ефекту
};

class Ruby : public IGem
{
public:
    ~Ruby()
    {
        cout << "Рубін зруйновано" << endl;
    }

    string getName() const override
    {
        return "Рубін";
    }

    int getBonus() const override
    {
        return 10;
    }

    string getEffect() const override
    {
        return "вогонь";
    }
};

class Sapphire : public IGem
{
public:
    ~Sapphire()
    {
        cout << "Сапфір зруйновано" << endl;
    }

    string getName() const override
    {
        return "Сапфір";
    }

    int getBonus() const override
    {
        return 6;
    }

    string getEffect() const override
    {
        return "холод, ціль сповільнена";
    }
};

class Weapon
{
private:
    string name;
    int baseDamage;
    unique_ptr<IGem> socket;                     // композиція: зброя володіє каменем у сокеті

public:
    Weapon(const string& name, int baseDamage) : name(name), baseDamage(baseDamage)
    {}

    /*
      1. Чому unique_ptr не можна скопіювати? Що зберігається в полі socket меча після removeGem()?
         Чому всередині insertGem потрібен move, а при виклику sword.insertGem(createGem("ruby")) — ні?

      2. У який момент і хто руйнує рубін, коли в меч вставляють сапфір? Чому в класі Weapon не потрібен
         деструктор з delete? Що сталося із сапфіром на кроці 6 і з рубіном на кроці 7?

      4. Чим відрізняються unique_ptr, shared_ptr і weak_ptr? Який з них беруть за замовчуванням і чому?
    */
    void insertGem(unique_ptr<IGem> gem)
    {
        socket = move(gem);
        cout << name << ": вставлено " << socket->getName() << endl;
    }

    unique_ptr<IGem> removeGem()
    {
        if (!socket)
        {
            cout << name << ": сокет порожній" << endl;
            return nullptr;
        }
        cout << name << ": вийнято " << socket->getName() << endl;
        return move(socket);                     // віддаємо власність, сокет стає порожнім
    }

    void attack() const
    {
        if (socket)
        {
            int bonus = socket->getBonus();
            cout << name << " б'є: " << baseDamage << " + " << bonus << " = " << baseDamage + bonus
                << " шкоди (" << socket->getEffect() << ")" << endl;
        }
        else
        {
            cout << name << " б'є: " << baseDamage << " шкоди" << endl;
        }
    }
};

unique_ptr<IGem> createGem(const string& type)
{
    if (type == "sapphire")
    {
        return make_unique<Sapphire>();
    }
    return make_unique<Ruby>();
}

int main()
{
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    // 1
    Weapon sword("Меч", 25);
    Weapon axe("Сокира", 40);
    sword.attack();

    // 2
    sword.insertGem(createGem("ruby"));
    sword.attack();

    // 3
    sword.insertGem(createGem("sapphire"));     // рубін руйнується тут
    sword.attack();

    // 4
    unique_ptr<IGem> gem = sword.removeGem();
    if (gem)
    {
        cout << "У сумці: " << gem->getName() << endl;
    }
    sword.attack();

    // 5
    axe.insertGem(move(gem));
    axe.attack();
    if (!gem)
    {
        cout << "gem тепер порожній" << endl;
    }

    // 6
    axe.removeGem();                            // тимчасовий unique_ptr знищується в кінці рядка — сапфір руйнується
    axe.removeGem();

    // 7
    {
        Weapon dagger("Кинджал", 12);
        dagger.insertGem(createGem("ruby"));
        dagger.attack();
    }                                           // кинджал знищено — рубін разом з ним
    cout << "Кинджал продано торговцю" << endl;

    return 0;
}

/*

  1. Чому unique_ptr не можна скопіювати? Що зберігається в полі socket меча після removeGem()?
     Чому всередині insertGem потрібен move, а при виклику sword.insertGem(createGem("ruby")) — ні?
  2. У який момент і хто руйнує рубін, коли в меч вставляють сапфір? Чому в класі Weapon не потрібен
     деструктор з delete? Що сталося із сапфіром на кроці 6 і з рубіном на кроці 7?
  3. Навіщо в інтерфейсі IGem віртуальний деструктор? Що піде не так, якщо його прибрати?
  4. Чим відрізняються unique_ptr, shared_ptr і weak_ptr? Який з них беруть за замовчуванням і чому?
  
  ВІДПОВІДІ НА ЧАСТИНУ «ПОЯСНИ»

  1. 

  2. 

  3. 

  4. 

*/
