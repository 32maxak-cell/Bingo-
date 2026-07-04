// bingo.js
#!/usr/bin/env node
'use strict';

const fs = require('fs');
const path = require('path');
const os = require('os');
const readline = require('readline');

const COLORS = {
    reset: '\x1b[0m',
    green: '\x1b[92m',
    red: '\x1b[91m',
    yellow: '\x1b[93m',
    blue: '\x1b[94m',
    bold: '\x1b[1m'
};

function colorize(text, color) {
    return COLORS[color] + text + COLORS.reset;
}

class BingoCard {
    constructor() {
        this.numbers = Array.from({length:5}, () => Array(5).fill(0));
        this.marked = Array.from({length:5}, () => Array(5).fill(false));
        this.generate();
    }

    generate() {
        const ranges = [[1,15],[16,30],[31,45],[46,60],[61,75]];
        for (let col=0; col<5; col++) {
            let nums = [];
            for (let i=ranges[col][0]; i<=ranges[col][1]; i++) nums.push(i);
            // перемешиваем
            for (let i=nums.length-1; i>0; i--) {
                const j = Math.floor(Math.random() * (i+1));
                [nums[i], nums[j]] = [nums[j], nums[i]];
            }
            for (let row=0; row<5; row++) {
                this.numbers[row][col] = nums[row];
            }
        }
        this.marked[2][2] = true; // FREE
    }

    mark(num) {
        for (let r=0; r<5; r++) {
            for (let c=0; c<5; c++) {
                if (this.numbers[r][c] === num) {
                    this.marked[r][c] = true;
                    return true;
                }
            }
        }
        return false;
    }

    checkWin() {
        // строки
        for (let r=0; r<5; r++) {
            if (this.marked[r].every(v => v)) return true;
        }
        // столбцы
        for (let c=0; c<5; c++) {
            let ok = true;
            for (let r=0; r<5; r++) if (!this.marked[r][c]) ok = false;
            if (ok) return true;
        }
        // диагонали
        let ok1 = true, ok2 = true;
        for (let i=0; i<5; i++) {
            if (!this.marked[i][i]) ok1 = false;
            if (!this.marked[i][4-i]) ok2 = false;
        }
        if (ok1 || ok2) return true;
        return false;
    }

    display(hide) {
        if (hide) {
            for (let r=0; r<5; r++) {
                let row = [];
                for (let c=0; c<5; c++) {
                    row.push(this.marked[r][c] ? colorize('XX', 'green') : '??');
                }
                console.log(row.join(' '));
            }
            return;
        }
        for (let r=0; r<5; r++) {
            let row = [];
            for (let c=0; c<5; c++) {
                if (this.marked[r][c]) {
                    row.push(colorize(String(this.numbers[r][c]).padStart(2), 'green'));
                } else {
                    row.push(String(this.numbers[r][c]).padStart(2));
                }
            }
            console.log(row.join(' '));
        }
    }
}

class BingoGame {
    constructor(mode) {
        this.mode = mode;
        this.players = [];
        this.called = [];
        this.current = 0;
        this.setupPlayers();
    }

    setupPlayers() {
        if (this.mode === 'single') {
            this.players = [
                { name: 'Игрок', card: new BingoCard(), won: false },
                { name: 'Компьютер', card: new BingoCard(), won: false }
            ];
        } else {
            this.players = [
                { name: 'Игрок 1', card: new BingoCard(), won: false },
                { name: 'Игрок 2', card: new BingoCard(), won: false }
            ];
        }
    }

    callNumber() {
        const available = [];
        for (let n=1; n<=75; n++) {
            if (!this.called.includes(n)) available.push(n);
        }
        if (available.length === 0) return -1;
        const idx = Math.floor(Math.random() * available.length);
        const num = available[idx];
        this.called.push(num);
        this.current = num;
        return num;
    }

    markAll(num) {
        for (const p of this.players) {
            p.card.mark(num);
        }
    }

    checkWinner() {
        for (let i=0; i<this.players.length; i++) {
            if (this.players[i].card.checkWin()) return i;
        }
        return -1;
    }

    displayState() {
        console.log(colorize('='.repeat(50), 'bold'));
        console.log(colorize(`Вызванное число: ${this.current || '—'}`, 'yellow'));
        for (let i=0; i<this.players.length; i++) {
            const p = this.players[i];
            console.log(colorize(`\n${p.name}:`, 'bold'));
            const hide = (this.mode === 'single' && i === 1);
            p.card.display(hide);
        }
        console.log(colorize('='.repeat(50), 'bold'));
    }

    async play() {
        const rl = readline.createInterface({
            input: process.stdin,
            output: process.stdout
        });
        const question = (q) => new Promise(resolve => rl.question(q, resolve));

        console.log(colorize('🎲 Добро пожаловать в Бинго!', 'bold'));
        if (this.mode === 'single') console.log('Игра против компьютера.');
        else console.log('Игра для двух игроков.');
        console.log('Нажимайте Enter для вызова следующего числа.');
        console.log('Для выхода введите q.\n');

        while (true) {
            this.displayState();
            const cmd = await question('> ');
            if (cmd.trim().toLowerCase() === 'q') {
                console.log('Выход.');
                rl.close();
                break;
            }
            if (cmd.trim() !== '') continue;
            const num = this.callNumber();
            if (num === -1) {
                console.log(colorize('Все числа вызваны! Ничья.', 'yellow'));
                break;
            }
            this.markAll(num);
            const winner = this.checkWinner();
            if (winner !== -1) {
                const name = this.players[winner].name;
                console.log(colorize(`🎉 Победитель: ${name}!`, 'green'));
                this.displayState();
                break;
            }
            await new Promise(r => setTimeout(r, 500));
        }
        rl.close();
    }
}

async function main() {
    let mode = 'single';
    let showStats = false, resetStats = false;
    const args = process.argv.slice(2);
    for (const arg of args) {
        if (arg === 'two') mode = 'two';
        else if (arg === '-s' || arg === '--stats') showStats = true;
        else if (arg === '-r' || arg === '--reset') resetStats = true;
        else if (arg === '-h' || arg === '--help') {
            console.log('Usage: node bingo.js [single|two] [-s] [-r]');
            return;
        }
    }
    if (resetStats) {
        const f = path.join(os.homedir(), '.bingo_stats.json');
        if (fs.existsSync(f)) fs.unlinkSync(f);
        console.log('Статистика сброшена.');
        return;
    }
    if (showStats) {
        console.log('Статистика не реализована в JS для простоты.');
        return;
    }
    const game = new BingoGame(mode);
    await game.play();
}

main().catch(console.error);
