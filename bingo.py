# bingo.py
#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import random
import json
import time
from pathlib import Path

# ANSI-цвета
COLORS = {
    'reset': '\033[0m',
    'green': '\033[92m',
    'red': '\033[91m',
    'yellow': '\033[93m',
    'blue': '\033[94m',
    'bold': '\033[1m'
}

def colorize(text, color):
    return f"{COLORS.get(color, '')}{text}{COLORS['reset']}"

class BingoCard:
    def __init__(self):
        self.numbers = [[0]*5 for _ in range(5)]
        self.marked = [[False]*5 for _ in range(5)]
        self.generate()

    def generate(self):
        # Колонки: B:1-15, I:16-30, N:31-45, G:46-60, O:61-75
        ranges = [(1,15), (16,30), (31,45), (46,60), (61,75)]
        for col in range(5):
            nums = random.sample(range(ranges[col][0], ranges[col][1]+1), 5)
            for row in range(5):
                self.numbers[row][col] = nums[row]
        # Центр - FREE (всегда отмечен)
        self.marked[2][2] = True

    def mark(self, num):
        for r in range(5):
            for c in range(5):
                if self.numbers[r][c] == num:
                    self.marked[r][c] = True
                    return True
        return False

    def check_win(self):
        # Строки
        for r in range(5):
            if all(self.marked[r][c] for c in range(5)):
                return True, 'row', r
        # Столбцы
        for c in range(5):
            if all(self.marked[r][c] for r in range(5)):
                return True, 'col', c
        # Диагонали
        if all(self.marked[i][i] for i in range(5)):
            return True, 'diag', 0
        if all(self.marked[i][4-i] for i in range(5)):
            return True, 'diag', 1
        return False, None, None

    def display(self, hide=False):
        if hide:
            # Показываем только первый ряд (как в реальном бинго)
            # Но для простоты покажем всю карточку, но скроем цифры
            for r in range(5):
                row = []
                for c in range(5):
                    if self.marked[r][c]:
                        row.append(colorize('XX', 'green'))
                    else:
                        row.append('??')
                print(' '.join(row))
            return
        for r in range(5):
            row = []
            for c in range(5):
                if self.marked[r][c]:
                    row.append(colorize(f'{self.numbers[r][c]:2}', 'green'))
                else:
                    row.append(f'{self.numbers[r][c]:2}')
            print(' '.join(row))

class BingoGame:
    def __init__(self, mode='single'):
        self.mode = mode
        self.players = []
        self.called = []
        self.current_number = None
        self.stats_file = Path.home() / '.bingo_stats.json'
        self.stats = self.load_stats()
        self.setup_players()

    def load_stats(self):
        if self.stats_file.exists():
            with open(self.stats_file, 'r') as f:
                return json.load(f)
        return {}

    def save_stats(self):
        with open(self.stats_file, 'w') as f:
            json.dump(self.stats, f, indent=2)

    def setup_players(self):
        if self.mode == 'single':
            self.players = [('Игрок', BingoCard(), False), ('Компьютер', BingoCard(), False)]
        else:  # two
            self.players = [('Игрок 1', BingoCard(), False), ('Игрок 2', BingoCard(), False)]

    def call_number(self):
        available = [n for n in range(1, 76) if n not in self.called]
        if not available:
            return None
        num = random.choice(available)
        self.called.append(num)
        self.current_number = num
        return num

    def mark_all(self, num):
        for player in self.players:
            player[1].mark(num)

    def check_winner(self):
        for i, (name, card, won) in enumerate(self.players):
            if card.check_win()[0]:
                return i
        return None

    def display_state(self):
        print(colorize('='*50, 'bold'))
        print(colorize(f'Вызванное число: {self.current_number if self.current_number else "—"}', 'yellow'))
        for idx, (name, card, won) in enumerate(self.players):
            print(colorize(f'\n{name}:', 'bold'))
            if self.mode == 'single' and idx == 1:  # компьютер скрыт
                card.display(hide=True)
            else:
                card.display()
        print(colorize('='*50, 'bold'))

    def play(self):
        print(colorize('🎲 Добро пожаловать в Бинго!', 'bold'))
        if self.mode == 'single':
            print('Игра против компьютера.')
        else:
            print('Игра для двух игроков.')
        print('Нажимайте Enter для вызова следующего числа.')
        print('Для выхода введите q.\n')

        while True:
            self.display_state()
            cmd = input('> ').strip().lower()
            if cmd == 'q':
                print('Выход.')
                break
            if cmd != '':
                continue
            num = self.call_number()
            if num is None:
                print(colorize('Все числа вызваны! Ничья.', 'yellow'))
                break
            self.mark_all(num)
            winner = self.check_winner()
            if winner is not None:
                name = self.players[winner][0]
                print(colorize(f'🎉 Победитель: {name}!', 'green'))
                # Подсветка выигрышной линии
                self.display_state()
                # Обновление статистики
                if name not in self.stats:
                    self.stats[name] = {'wins': 0, 'losses': 0}
                self.stats[name]['wins'] += 1
                for i, (n, _, _) in enumerate(self.players):
                    if i != winner:
                        if n not in self.stats:
                            self.stats[n] = {'wins': 0, 'losses': 0}
                        self.stats[n]['losses'] += 1
                self.save_stats()
                print(colorize(f'Статистика сохранена.', 'blue'))
                break
            time.sleep(0.5)

def main():
    mode = 'single'
    show_stats = False
    reset_stats = False
    args = sys.argv[1:]
    for arg in args:
        if arg == 'two':
            mode = 'two'
        elif arg == '-s' or arg == '--stats':
            show_stats = True
        elif arg == '-r' or arg == '--reset':
            reset_stats = True
        elif arg == '-h' or arg == '--help':
            print('Usage: bingo.py [single|two] [-s] [-r]')
            return
    if reset_stats:
        stats_file = Path.home() / '.bingo_stats.json'
        if stats_file.exists():
            stats_file.unlink()
        print('Статистика сброшена.')
        return
    if show_stats:
        stats_file = Path.home() / '.bingo_stats.json'
        if stats_file.exists():
            with open(stats_file, 'r') as f:
                stats = json.load(f)
                print(colorize('📊 Статистика:', 'bold'))
                for name, data in stats.items():
                    print(f'  {name}: победы {data["wins"]}, поражения {data["losses"]}')
        else:
            print('Статистика пуста.')
        return
    game = BingoGame(mode)
    game.play()

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print(colorize('\nИгра прервана.', 'yellow'))
        sys.exit(0)
