// bingo.cpp
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <random>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

const string RESET = "\033[0m";
const string GREEN = "\033[92m";
const string RED = "\033[91m";
const string YELLOW = "\033[93m";
const string BLUE = "\033[94m";
const string BOLD = "\033[1m";

string colorize(const string& text, const string& color) {
    return color + text + RESET;
}

string getHomeDir() {
    const char* home = getenv("HOME");
    if (!home) home = getenv("USERPROFILE");
    return string(home);
}

class BingoCard {
public:
    int numbers[5][5];
    bool marked[5][5];

    BingoCard() {
        generate();
    }

    void generate() {
        // Колонки B:1-15, I:16-30, N:31-45, G:46-60, O:61-75
        int ranges[5][2] = {{1,15},{16,30},{31,45},{46,60},{61,75}};
        random_device rd;
        mt19937 gen(rd());
        for (int col=0; col<5; ++col) {
            vector<int> nums;
            for (int i=ranges[col][0]; i<=ranges[col][1]; ++i) nums.push_back(i);
            shuffle(nums.begin(), nums.end(), gen);
            for (int row=0; row<5; ++row) {
                numbers[row][col] = nums[row];
                marked[row][col] = false;
            }
        }
        marked[2][2] = true; // FREE
    }

    bool mark(int num) {
        for (int r=0; r<5; ++r)
            for (int c=0; c<5; ++c)
                if (numbers[r][c] == num) {
                    marked[r][c] = true;
                    return true;
                }
        return false;
    }

    bool checkWin(int& type, int& idx) {
        // Строки
        for (int r=0; r<5; ++r) {
            bool ok = true;
            for (int c=0; c<5; ++c) if (!marked[r][c]) ok = false;
            if (ok) { type=0; idx=r; return true; }
        }
        // Столбцы
        for (int c=0; c<5; ++c) {
            bool ok = true;
            for (int r=0; r<5; ++r) if (!marked[r][c]) ok = false;
            if (ok) { type=1; idx=c; return true; }
        }
        // Диагонали
        bool ok1=true, ok2=true;
        for (int i=0; i<5; ++i) {
            if (!marked[i][i]) ok1=false;
            if (!marked[i][4-i]) ok2=false;
        }
        if (ok1) { type=2; idx=0; return true; }
        if (ok2) { type=2; idx=1; return true; }
        return false;
    }

    void display(bool hide=false) {
        if (hide) {
            for (int r=0; r<5; ++r) {
                for (int c=0; c<5; ++c) {
                    if (marked[r][c]) cout << colorize("XX", GREEN) << " ";
                    else cout << "?? ";
                }
                cout << endl;
            }
            return;
        }
        for (int r=0; r<5; ++r) {
            for (int c=0; c<5; ++c) {
                if (marked[r][c])
                    cout << colorize(to_string(numbers[r][c]) + " ", GREEN);
                else
                    cout << numbers[r][c] << " ";
            }
            cout << endl;
        }
    }
};

struct Player {
    string name;
    BingoCard card;
    bool won;
};

class BingoGame {
public:
    string mode;
    vector<Player> players;
    vector<int> called;
    int currentNumber;
    string statsFile;

    BingoGame(string m) : mode(m), currentNumber(0) {
        statsFile = getHomeDir() + "/.bingo_stats.json";
        loadStats();
        setupPlayers();
    }

    void loadStats() {
        // Упрощённо – парсинг JSON не будем делать, оставим пустым для простоты.
    }

    void saveStats() {
        // Для C++ пропустим сохранение статистики.
    }

    void setupPlayers() {
        if (mode == "single") {
            players.push_back({"Игрок", BingoCard(), false});
            players.push_back({"Компьютер", BingoCard(), false});
        } else {
            players.push_back({"Игрок 1", BingoCard(), false});
            players.push_back({"Игрок 2", BingoCard(), false});
        }
    }

    int callNumber() {
        vector<int> available;
        for (int n=1; n<=75; ++n) {
            if (find(called.begin(), called.end(), n) == called.end())
                available.push_back(n);
        }
        if (available.empty()) return -1;
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(0, available.size()-1);
        int num = available[dis(gen)];
        called.push_back(num);
        currentNumber = num;
        return num;
    }

    void markAll(int num) {
        for (auto& p : players) p.card.mark(num);
    }

    int checkWinner() {
        for (size_t i=0; i<players.size(); ++i) {
            int type, idx;
            if (players[i].card.checkWin(type, idx)) return i;
        }
        return -1;
    }

    void displayState() {
        cout << colorize("==================================================", BOLD) << endl;
        cout << colorize("Вызванное число: " + (currentNumber ? to_string(currentNumber) : "—"), YELLOW) << endl;
        for (size_t i=0; i<players.size(); ++i) {
            cout << colorize("\n" + players[i].name + ":", BOLD) << endl;
            bool hide = (mode == "single" && i == 1);
            players[i].card.display(hide);
        }
        cout << colorize("==================================================", BOLD) << endl;
    }

    void play() {
        cout << colorize("🎲 Добро пожаловать в Бинго!", BOLD) << endl;
        if (mode == "single") cout << "Игра против компьютера." << endl;
        else cout << "Игра для двух игроков." << endl;
        cout << "Нажимайте Enter для вызова следующего числа." << endl;
        cout << "Для выхода введите q.\n" << endl;

        string cmd;
        while (true) {
            displayState();
            getline(cin, cmd);
            if (cmd == "q") { cout << "Выход." << endl; break; }
            if (!cmd.empty()) continue;
            int num = callNumber();
            if (num == -1) {
                cout << colorize("Все числа вызваны! Ничья.", YELLOW) << endl;
                break;
            }
            markAll(num);
            int winner = checkWinner();
            if (winner != -1) {
                string name = players[winner].name;
                cout << colorize("🎉 Победитель: " + name + "!", GREEN) << endl;
                displayState();
                break;
            }
            this_thread::sleep_for(chrono::milliseconds(500));
        }
    }
};

int main(int argc, char* argv[]) {
    string mode = "single";
    bool showStats = false, resetStats = false;
    for (int i=1; i<argc; ++i) {
        string arg = argv[i];
        if (arg == "two") mode = "two";
        else if (arg == "-s" || arg == "--stats") showStats = true;
        else if (arg == "-r" || arg == "--reset") resetStats = true;
        else if (arg == "-h" || arg == "--help") {
            cout << "Usage: bingo [single|two] [-s] [-r]" << endl;
            return 0;
        }
    }
    if (resetStats) {
        string f = getHomeDir() + "/.bingo_stats.json";
        if (fs::exists(f)) fs::remove(f);
        cout << "Статистика сброшена." << endl;
        return 0;
    }
    if (showStats) {
        cout << "Статистика (упрощённо) – не реализована в C++." << endl;
        return 0;
    }
    BingoGame game(mode);
    game.play();
    return 0;
}
