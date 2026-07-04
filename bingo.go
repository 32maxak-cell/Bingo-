// bingo.go
package main

import (
	"bufio"
	"encoding/json"
	"fmt"
	"math/rand"
	"os"
	"path/filepath"
	"strconv"
	"strings"
	"time"
)

const (
	reset  = "\033[0m"
	green  = "\033[92m"
	red    = "\033[91m"
	yellow = "\033[93m"
	blue   = "\033[94m"
	bold   = "\033[1m"
)

func colorize(text, color string) string {
	return color + text + reset
}

type BingoCard struct {
	Numbers [5][5]int
	Marked  [5][5]bool
}

func NewBingoCard() *BingoCard {
	c := &BingoCard{}
	c.generate()
	return c
}

func (c *BingoCard) generate() {
	ranges := [5][2]int{{1, 15}, {16, 30}, {31, 45}, {46, 60}, {61, 75}}
	rand.Seed(time.Now().UnixNano())
	for col := 0; col < 5; col++ {
		nums := rand.Perm(ranges[col][1] - ranges[col][0] + 1)
		for row := 0; row < 5; row++ {
			c.Numbers[row][col] = ranges[col][0] + nums[row]
			c.Marked[row][col] = false
		}
	}
	c.Marked[2][2] = true // FREE
}

func (c *BingoCard) mark(num int) bool {
	for r := 0; r < 5; r++ {
		for col := 0; col < 5; col++ {
			if c.Numbers[r][col] == num {
				c.Marked[r][col] = true
				return true
			}
		}
	}
	return false
}

func (c *BingoCard) checkWin() (bool, string, int) {
	// Строки
	for r := 0; r < 5; r++ {
		ok := true
		for col := 0; col < 5; col++ {
			if !c.Marked[r][col] {
				ok = false
				break
			}
		}
		if ok {
			return true, "row", r
		}
	}
	// Столбцы
	for col := 0; col < 5; col++ {
		ok := true
		for r := 0; r < 5; r++ {
			if !c.Marked[r][col] {
				ok = false
				break
			}
		}
		if ok {
			return true, "col", col
		}
	}
	// Диагонали
	ok1, ok2 := true, true
	for i := 0; i < 5; i++ {
		if !c.Marked[i][i] {
			ok1 = false
		}
		if !c.Marked[i][4-i] {
			ok2 = false
		}
	}
	if ok1 {
		return true, "diag", 0
	}
	if ok2 {
		return true, "diag", 1
	}
	return false, "", 0
}

func (c *BingoCard) display(hide bool) {
	if hide {
		for r := 0; r < 5; r++ {
			for col := 0; col < 5; col++ {
				if c.Marked[r][col] {
					fmt.Print(colorize("XX", green) + " ")
				} else {
					fmt.Print("?? ")
				}
			}
			fmt.Println()
		}
		return
	}
	for r := 0; r < 5; r++ {
		for col := 0; col < 5; col++ {
			if c.Marked[r][col] {
				fmt.Printf("%s%2d%s ", colorize("", green), c.Numbers[r][col], colorize("", reset))
			} else {
				fmt.Printf("%2d ", c.Numbers[r][col])
			}
		}
		fmt.Println()
	}
}

type Player struct {
	Name string
	Card *BingoCard
	Won  bool
}

type BingoGame struct {
	Mode    string
	Players []Player
	Called  []int
	Current int
}

func NewBingoGame(mode string) *BingoGame {
	g := &BingoGame{Mode: mode}
	g.setupPlayers()
	return g
}

func (g *BingoGame) setupPlayers() {
	if g.Mode == "single" {
		g.Players = []Player{
			{"Игрок", NewBingoCard(), false},
			{"Компьютер", NewBingoCard(), false},
		}
	} else {
		g.Players = []Player{
			{"Игрок 1", NewBingoCard(), false},
			{"Игрок 2", NewBingoCard(), false},
		}
	}
}

func (g *BingoGame) callNumber() int {
	available := []int{}
	for n := 1; n <= 75; n++ {
		found := false
		for _, c := range g.Called {
			if c == n {
				found = true
				break
			}
		}
		if !found {
			available = append(available, n)
		}
	}
	if len(available) == 0 {
		return -1
	}
	idx := rand.Intn(len(available))
	num := available[idx]
	g.Called = append(g.Called, num)
	g.Current = num
	return num
}

func (g *BingoGame) markAll(num int) {
	for i := range g.Players {
		g.Players[i].Card.mark(num)
	}
}

func (g *BingoGame) checkWinner() int {
	for i, p := range g.Players {
		if win, _, _ := p.Card.checkWin(); win {
			return i
		}
	}
	return -1
}

func (g *BingoGame) displayState() {
	fmt.Println(colorize("==================================================", bold))
	if g.Current != 0 {
		fmt.Println(colorize(fmt.Sprintf("Вызванное число: %d", g.Current), yellow))
	} else {
		fmt.Println(colorize("Вызванное число: —", yellow))
	}
	for i, p := range g.Players {
		fmt.Println(colorize("\n"+p.Name+":", bold))
		hide := (g.Mode == "single" && i == 1)
		p.Card.display(hide)
	}
	fmt.Println(colorize("==================================================", bold))
}

func (g *BingoGame) play() {
	fmt.Println(colorize("🎲 Добро пожаловать в Бинго!", bold))
	if g.Mode == "single" {
		fmt.Println("Игра против компьютера.")
	} else {
		fmt.Println("Игра для двух игроков.")
	}
	fmt.Println("Нажимайте Enter для вызова следующего числа.")
	fmt.Println("Для выхода введите q.\n")

	scanner := bufio.NewScanner(os.Stdin)
	for {
		g.displayState()
		fmt.Print("> ")
		if !scanner.Scan() {
			break
		}
		cmd := strings.TrimSpace(scanner.Text())
		if cmd == "q" {
			fmt.Println("Выход.")
			break
		}
		if cmd != "" {
			continue
		}
		num := g.callNumber()
		if num == -1 {
			fmt.Println(colorize("Все числа вызваны! Ничья.", yellow))
			break
		}
		g.markAll(num)
		winner := g.checkWinner()
		if winner != -1 {
			name := g.Players[winner].Name
			fmt.Println(colorize(fmt.Sprintf("🎉 Победитель: %s!", name), green))
			g.displayState()
			break
		}
		time.Sleep(500 * time.Millisecond)
	}
}

func main() {
	mode := "single"
	showStats := false
	resetStats := false
	for _, arg := range os.Args[1:] {
		switch arg {
		case "two":
			mode = "two"
		case "-s", "--stats":
			showStats = true
		case "-r", "--reset":
			resetStats = true
		case "-h", "--help":
			fmt.Println("Usage: bingo [single|two] [-s] [-r]")
			return
		}
	}
	if resetStats {
		f := filepath.Join(os.Getenv("HOME"), ".bingo_stats.json")
		os.Remove(f)
		fmt.Println("Статистика сброшена.")
		return
	}
	if showStats {
		fmt.Println("Статистика не реализована в Go для простоты.")
		return
	}
	game := NewBingoGame(mode)
	game.play()
}
