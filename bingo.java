// bingo.java
import java.io.*;
import java.nio.file.*;
import java.util.*;
import java.util.stream.*;

public class bingo {
    private static final String RESET = "\u001B[0m";
    private static final String GREEN = "\u001B[92m";
    private static final String RED = "\u001B[91m";
    private static final String YELLOW = "\u001B[93m";
    private static final String BLUE = "\u001B[94m";
    private static final String BOLD = "\u001B[1m";

    private static String colorize(String text, String color) {
        return color + text + RESET;
    }

    static class BingoCard {
        int[][] numbers = new int[5][5];
        boolean[][] marked = new boolean[5][5];

        BingoCard() {
            generate();
        }

        void generate() {
            int[][] ranges = {{1,15},{16,30},{31,45},{46,60},{61,75}};
            Random rnd = new Random();
            for (int col=0; col<5; col++) {
                List<Integer> nums = new ArrayList<>();
                for (int i=ranges[col][0]; i<=ranges[col][1]; i++) nums.add(i);
                Collections.shuffle(nums, rnd);
                for (int row=0; row<5; row++) {
                    numbers[row][col] = nums.get(row);
                }
            }
            marked[2][2] = true;
        }

        boolean mark(int num) {
            for (int r=0; r<5; r++)
                for (int c=0; c<5; c++)
                    if (numbers[r][c] == num) {
                        marked[r][c] = true;
                        return true;
                    }
            return false;
        }

        boolean checkWin() {
            // строки
            for (int r=0; r<5; r++) {
                boolean ok = true;
                for (int c=0; c<5; c++) if (!marked[r][c]) ok = false;
                if (ok) return true;
            }
            // столбцы
            for (int c=0; c<5; c++) {
                boolean ok = true;
                for (int r=0; r<5; r++) if (!marked[r][c]) ok = false;
                if (ok) return true;
            }
            // диагонали
            boolean ok1=true, ok2=true;
            for (int i=0; i<5; i++) {
                if (!marked[i][i]) ok1 = false;
                if (!marked[i][4-i]) ok2 = false;
            }
            return ok1 || ok2;
        }

        void display(boolean hide) {
            if (hide) {
                for (int r=0; r<5; r++) {
                    for (int c=0; c<5; c++) {
                        System.out.print(marked[r][c] ? colorize("XX", GREEN)+" " : "?? ");
                    }
                    System.out.println();
                }
                return;
            }
            for (int r=0; r<5; r++) {
                for (int c=0; c<5; c++) {
                    if (marked[r][c])
                        System.out.print(colorize(String.format("%2d", numbers[r][c]), GREEN)+" ");
                    else
                        System.out.print(String.format("%2d", numbers[r][c])+" ");
                }
                System.out.println();
            }
        }
    }

    static class BingoGame {
        String mode;
        List<Player> players = new ArrayList<>();
        List<Integer> called = new ArrayList<>();
        int current = 0;

        class Player {
            String name;
            BingoCard card;
            boolean won;
            Player(String n) { name=n; card=new BingoCard(); won=false; }
        }

        BingoGame(String m) {
            mode = m;
            setupPlayers();
        }

        void setupPlayers() {
            if (mode.equals("single")) {
                players.add(new Player("Игрок"));
                players.add(new Player("Компьютер"));
            } else {
                players.add(new Player("Игрок 1"));
                players.add(new Player("Игрок 2"));
            }
        }

        int callNumber() {
            List<Integer> available = new ArrayList<>();
            for (int n=1; n<=75; n++) {
                if (!called.contains(n)) available.add(n);
            }
            if (available.isEmpty()) return -1;
            Random rnd = new Random();
            int num = available.get(rnd.nextInt(available.size()));
            called.add(num);
            current = num;
            return num;
        }

        void markAll(int num) {
            for (Player p : players) p.card.mark(num);
        }

        int checkWinner() {
            for (int i=0; i<players.size(); i++) {
                if (players.get(i).card.checkWin()) return i;
            }
            return -1;
        }

        void displayState() {
            System.out.println(colorize("==================================================", BOLD));
            System.out.println(colorize("Вызванное число: " + (current!=0 ? current : "—"), YELLOW));
            for (int i=0; i<players.size(); i++) {
                Player p = players.get(i);
                System.out.println(colorize("\n" + p.name + ":", BOLD));
                boolean hide = (mode.equals("single") && i==1);
                p.card.display(hide);
            }
            System.out.println(colorize("==================================================", BOLD));
        }

        void play() throws IOException {
            BufferedReader reader = new BufferedReader(new InputStreamReader(System.in));
            System.out.println(colorize("🎲 Добро пожаловать в Бинго!", BOLD));
            if (mode.equals("single")) System.out.println("Игра против компьютера.");
            else System.out.println("Игра для двух игроков.");
            System.out.println("Нажимайте Enter для вызова следующего числа.");
            System.out.println("Для выхода введите q.\n");

            while (true) {
                displayState();
                System.out.print("> ");
                String cmd = reader.readLine().trim();
                if (cmd.equals("q")) { System.out.println("Выход."); break; }
                if (!cmd.isEmpty()) continue;
                int num = callNumber();
                if (num == -1) {
                    System.out.println(colorize("Все числа вызваны! Ничья.", YELLOW));
                    break;
                }
                markAll(num);
                int winner = checkWinner();
                if (winner != -1) {
                    String name = players.get(winner).name;
                    System.out.println(colorize("🎉 Победитель: " + name + "!", GREEN));
                    displayState();
                    break;
                }
                try { Thread.sleep(500); } catch (InterruptedException e) {}
            }
            reader.close();
        }
    }

    public static void main(String[] args) throws IOException {
        String mode = "single";
        boolean showStats = false, resetStats = false;
        for (String arg : args) {
            if (arg.equals("two")) mode = "two";
            else if (arg.equals("-s") || arg.equals("--stats")) showStats = true;
            else if (arg.equals("-r") || arg.equals("--reset")) resetStats = true;
            else if (arg.equals("-h") || arg.equals("--help")) {
                System.out.println("Usage: java bingo [single|two] [-s] [-r]");
                return;
            }
        }
        if (resetStats) {
            String f = System.getProperty("user.home") + "/.bingo_stats.json";
            try { Files.deleteIfExists(Paths.get(f)); } catch (Exception e) {}
            System.out.println("Статистика сброшена.");
            return;
        }
        if (showStats) {
            System.out.println("Статистика не реализована в Java для простоты.");
            return;
        }
        BingoGame game = new BingoGame(mode);
        game.play();
    }
}
