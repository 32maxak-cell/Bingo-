// bingo.cs
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.Json;
using System.Threading;

class BingoCard
{
    public int[,] Numbers = new int[5,5];
    public bool[,] Marked = new bool[5,5];

    public BingoCard()
    {
        Generate();
    }

    public void Generate()
    {
        int[,] ranges = { {1,15}, {16,30}, {31,45}, {46,60}, {61,75} };
        Random rnd = new Random();
        for (int col=0; col<5; col++)
        {
            List<int> nums = new List<int>();
            for (int i = ranges[col,0]; i <= ranges[col,1]; i++) nums.Add(i);
            nums = nums.OrderBy(x => rnd.Next()).ToList();
            for (int row=0; row<5; row++)
                Numbers[row,col] = nums[row];
        }
        Marked[2,2] = true;
    }

    public bool Mark(int num)
    {
        for (int r=0; r<5; r++)
            for (int c=0; c<5; c++)
                if (Numbers[r,c] == num)
                {
                    Marked[r,c] = true;
                    return true;
                }
        return false;
    }

    public bool CheckWin()
    {
        // Строки
        for (int r=0; r<5; r++)
        {
            bool ok = true;
            for (int c=0; c<5; c++) if (!Marked[r,c]) ok = false;
            if (ok) return true;
        }
        // Столбцы
        for (int c=0; c<5; c++)
        {
            bool ok = true;
            for (int r=0; r<5; r++) if (!Marked[r,c]) ok = false;
            if (ok) return true;
        }
        // Диагонали
        bool ok1=true, ok2=true;
        for (int i=0; i<5; i++)
        {
            if (!Marked[i,i]) ok1 = false;
            if (!Marked[i,4-i]) ok2 = false;
        }
        return ok1 || ok2;
    }

    public void Display(bool hide)
    {
        if (hide)
        {
            for (int r=0; r<5; r++)
            {
                for (int c=0; c<5; c++)
                    Console.Write(Marked[r,c] ? Colorize("XX", "green")+" " : "?? ");
                Console.WriteLine();
            }
            return;
        }
        for (int r=0; r<5; r++)
        {
            for (int c=0; c<5; c++)
            {
                if (Marked[r,c])
                    Console.Write(Colorize(Numbers[r,c].ToString().PadLeft(2), "green")+" ");
                else
                    Console.Write(Numbers[r,c].ToString().PadLeft(2)+" ");
            }
            Console.WriteLine();
        }
    }

    static string Colorize(string text, string color)
    {
        string col = color switch
        {
            "green" => "\x1b[92m",
            "red" => "\x1b[91m",
            "yellow" => "\x1b[93m",
            "blue" => "\x1b[94m",
            "bold" => "\x1b[1m",
            _ => "\x1b[0m"
        };
        return col + text + "\x1b[0m";
    }
}

class BingoGame
{
    public string Mode;
    public List<(string name, BingoCard card, bool won)> Players = new List<(string, BingoCard, bool)>();
    public List<int> Called = new List<int>();
    public int CurrentNumber = 0;

    public BingoGame(string mode)
    {
        Mode = mode;
        SetupPlayers();
    }

    void SetupPlayers()
    {
        if (Mode == "single")
        {
            Players.Add(("Игрок", new BingoCard(), false));
            Players.Add(("Компьютер", new BingoCard(), false));
        }
        else
        {
            Players.Add(("Игрок 1", new BingoCard(), false));
            Players.Add(("Игрок 2", new BingoCard(), false));
        }
    }

    int CallNumber()
    {
        var available = Enumerable.Range(1,75).Where(n => !Called.Contains(n)).ToList();
        if (available.Count == 0) return -1;
        Random rnd = new Random();
        int num = available[rnd.Next(available.Count)];
        Called.Add(num);
        CurrentNumber = num;
        return num;
    }

    void MarkAll(int num)
    {
        foreach (var p in Players) p.card.Mark(num);
    }

    int CheckWinner()
    {
        for (int i=0; i<Players.Count; i++)
            if (Players[i].card.CheckWin()) return i;
        return -1;
    }

    void DisplayState()
    {
        Console.WriteLine(Colorize("==================================================", "bold"));
        Console.WriteLine(Colorize($"Вызванное число: {(CurrentNumber!=0 ? CurrentNumber.ToString() : "—")}", "yellow"));
        for (int i=0; i<Players.Count; i++)
        {
            var p = Players[i];
            Console.WriteLine(Colorize($"\n{p.name}:", "bold"));
            bool hide = (Mode == "single" && i == 1);
            p.card.Display(hide);
        }
        Console.WriteLine(Colorize("==================================================", "bold"));
    }

    static string Colorize(string text, string color) => BingoCard.Colorize(text, color);

    public void Play()
    {
        Console.WriteLine(Colorize("🎲 Добро пожаловать в Бинго!", "bold"));
        if (Mode == "single") Console.WriteLine("Игра против компьютера.");
        else Console.WriteLine("Игра для двух игроков.");
        Console.WriteLine("Нажимайте Enter для вызова следующего числа.");
        Console.WriteLine("Для выхода введите q.\n");

        while (true)
        {
            DisplayState();
            string cmd = Console.ReadLine();
            if (cmd == "q") { Console.WriteLine("Выход."); break; }
            if (!string.IsNullOrEmpty(cmd)) continue;
            int num = CallNumber();
            if (num == -1) { Console.WriteLine(Colorize("Все числа вызваны! Ничья.", "yellow")); break; }
            MarkAll(num);
            int winner = CheckWinner();
            if (winner != -1)
            {
                string name = Players[winner].name;
                Console.WriteLine(Colorize($"🎉 Победитель: {name}!", "green"));
                DisplayState();
                break;
            }
            Thread.Sleep(500);
        }
    }

    static void Main(string[] args)
    {
        string mode = "single";
        bool showStats = false, resetStats = false;
        foreach (var arg in args)
        {
            if (arg == "two") mode = "two";
            else if (arg == "-s" || arg == "--stats") showStats = true;
            else if (arg == "-r" || arg == "--reset") resetStats = true;
            else if (arg == "-h" || arg == "--help")
            {
                Console.WriteLine("Usage: bingo [single|two] [-s] [-r]");
                return;
            }
        }
        if (resetStats)
        {
            string f = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.UserProfile), ".bingo_stats.json");
            if (File.Exists(f)) File.Delete(f);
            Console.WriteLine("Статистика сброшена.");
            return;
        }
        if (showStats)
        {
            Console.WriteLine("Статистика не реализована в C# для простоты.");
            return;
        }
        BingoGame game = new BingoGame(mode);
        game.Play();
    }
}
