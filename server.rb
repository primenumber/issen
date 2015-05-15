require 'open3'
require 'yaml'
require_relative 'simple_chat/simple_chat'

prog_path = "./issen"
$config = YAML.load_file('config.yml')
$scc = SimpleChatClient.new('ws://localhost:14141')

$name = nil
$black = nil
$white = nil

$scc.onconnect do
  puts "connected"
  $scc.init($config['id'])
end

$scc.onlogin do |user|
  puts "logined"
  puts user.to_s
  $name = user['name']
  $scc.send_message(JSON.generate({
    "type" => "ready"
  }), ["othello"])
end

$scc.onerror do |m|
  puts "error"
  puts m
end

def valid?(message)
  true
end

$pstdin, $pstdout, $pwait_thr = Open3.popen2(prog_path)
$scc.onmessage do |name, message, type, tags|
  puts "n: #{name}, m: #{message}, t: #{tags}"
  if tags.include? "othello" then
    if (name == $black || name == $white) && name != $name then
      $pstdin.puts message if valid?(message)
      puts "play~~"
      data = $pstdout.gets.chomp
      $scc.send_message(data, ["othello"])
    elsif name == "othello" then
      obj = JSON.parse(message)
      case obj['type']
      when "command" then
        case obj['message']
        when "start" then
          puts "Game Start"
          $black = obj['black']
          $white = obj['white']
          if $black == $name then
            $pstdin.puts "Black"
            data = $pstdout.gets.chomp
            $scc.send_message(data, ["othello"])
          else
            $pstdin.puts "White"
          end
        end
      end
    end
  end
end

p $pwait_thr.value
