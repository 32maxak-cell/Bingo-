#!/usr/bin/env ruby
# bingo.rb
# encoding: UTF-8

require 'json'
require 'fileutils'
require 'set'

COLORS = {
  reset: "\e[0m",
  green: "\e[92m",
  red: "\e[91m",
  yellow: "\e[93m",
  blue: "\e[94m",
  bold: "\e[1m"
}

def colorize(text, color)
  "#{COLORS[color]}#{text}#{COLORS[:reset]}"
end

class BingoCard
  attr_reader :numbers, :marked

  def initialize
    @numbers = Array.new(5) { Array.new(5, 0) }
    @marked = Array.new(5) { Array.new(5, false) }
    generate
  end

  def generate
    ranges = [[1,15],[16,30],[31,45],[46,60],[61,75]]
    ranges.each_with_index do |(low, high), col|
      nums = (low..high).to_a.shuffle
      5.times { |row| @numbers[row][col] = nums[row] }
    end
    @marked[2][2] = true
  end

  def mark(num)
    5.times do |r|
      5.times do |c|
        if @numbers[r][c] == num
          @marked[r][c] = true
          return true
        end
      end
    end
    false
  end

  def check_win
    # строки
    5.times do |r|
      return true if @marked[r].all?
    end
    # столбцы
    5.times do |c|
      ok = true
      5.times { |r| ok = false unless @marked[r][c] }
      return true if ok
    end
    # диагонали
    ok1 = ok2 = true
    5.times do |i|
      ok1 = false unless @marked[i][i]
      ok2 = false unless @marked[i][4-i]
    end
    ok1 || ok2
  end

  def display(hide=false)
    if hide
      5.times do |r|
        row = []
        5.times do |c|
          row << (@marked[r][c] ? colorize('XX', :green) : '??')
        end
        puts row.join(' ')
      end
      return
    end
    5.times do |r|
      row = []
      5.times do |c|
        if @marked[r][c]
          row << colorize(@numbers[r][c].to_s.rjust(2), :green)
        else
          row << @numbers[r][c].to_s.rjust(2)
        end
      end
      puts row.join(' ')
    end
  end
end

class BingoGame
  attr_reader :mode, :players, :called, :current

  def initialize(mode)
    @mode = mode
    @called = []
    @current = nil
    setup_players
  end

  def setup_players
    if @mode == 'single'
      @players = [
        { name: 'Игрок', card: BingoCard.new, won: false },
        { name: 'Компьютер', card: BingoCard.new, won: false }
      ]
    else
      @players = [
        { name: 'Игрок 1', card: BingoCard.new, won: false },
        { name: 'Игрок 2', card: BingoCard.new, won: false }
      ]
    end
  end

  def call_number
    available = (1..75).reject { |n| @called.include?(n) }
    return nil if available.empty?
    num = available.sample
    @called << num
    @current = num
    num
  end

  def mark_all(num)
    @players.each { |p| p[:card].mark(num) }
  end

  def check_winner
    @players.each_with_index do |p, i|
      return i if p[:card].check_win
    end
    nil
  end

  def display_state
    puts colorize('=' * 50, :bold)
    puts colorize("Вызванное число: #{@current || '—'}", :yellow)
    @players.each_with_index do |p, i|
      puts colorize("\n#{p[:name]}:", :bold)
      hide = (@mode == 'single' && i == 1)
      p[:card].display(hide)
    end
    puts colorize('=' * 50, :bold)
  end

  def play
    puts colorize('🎲 Добро пожаловать в Бинго!', :bold)
    if @mode == 'single'
      puts 'Игра против компьютера.'
    else
      puts 'Игра для двух игроков.'
    end
    puts 'Нажимайте Enter для вызова следующего числа.'
    puts 'Для выхода введите q.'

    loop do
      display_state
      print '> '
      cmd = gets.chomp.strip
      break if cmd == 'q'
      next unless cmd.empty?
      num = call_number
      if num.nil?
        puts colorize('Все числа вызваны! Ничья.', :yellow)
        break
      end
      mark_all(num)
      winner = check_winner
      if winner
        name = @players[winner][:name]
        puts colorize("🎉 Победитель: #{name}!", :green)
        display_state
        break
      end
      sleep 0.5
    end
  end
end

def main
  mode = 'single'
  show_stats = false
  reset_stats = false
  ARGV.each do |arg|
    case arg
    when 'two' then mode = 'two'
    when '-s', '--stats' then show_stats = true
    when '-r', '--reset' then reset_stats = true
    when '-h', '--help'
      puts 'Usage: ruby bingo.rb [single|two] [-s] [-r]'
      return
    end
  end
  if reset_stats
    f = File.join(Dir.home, '.bingo_stats.json')
    File.delete(f) if File.exist?(f)
    puts 'Статистика сброшена.'
    return
  end
  if show_stats
    puts 'Статистика не реализована в Ruby для простоты.'
    return
  end
  game = BingoGame.new(mode)
  game.play
end

main if __FILE__ == $0
