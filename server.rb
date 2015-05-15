require 'open3'
require_relative 'simple_chat/simple_chat'

prog_path = "./issen"
id = "92ee48b55dccff4fbd55c272cf45783cbbf7acbfda61d0822bd7e5663be09da50e45e13f171cddc65348d1c8919aaa383bf116fb4c42cf3864f11895ba07fde8"
$scc = SimpleChatClient.new('ws://localhost:14141')

$name = nil
$black = nil
$white = nil

$scc.onconnect do
  puts "connected"
  $scc.init(id)
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

def valid?
  true
end

$pstdin, $pstdout, $pwait_thr = Open3.popen2(prog_path)
$scc.onmessage do |name, message, type, tags|
  puts "n: #{name}, m: #{message}, t: #{tags}"
  if tags.include? "othello" then
    if (name == $black || name == $white) && name != $name then
      $pstdin.puts message if valid?(message)
      puts "play~~"
      data = $pstdout.gets
      $scc.send_message(data, "othello")
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
            data = $pstdout.gets
            puts data
            $scc.send_message(data, "othello")
          else
            $pstdin.puts "White"
          end
        end
      end
    end
  end
end

p $pwait_thr.value
