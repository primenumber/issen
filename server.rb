require 'open3'
require_relative 'simple_chat/simple_chat'

prog_path = "./issen"
id = "92ee48b55dccff4fbd55c272cf45783cbbf7acbfda61d0822bd7e5663be09da50e45e13f171cddc65348d1c8919aaa383bf116fb4c42cf3864f11895ba07fde8"
scc = SimpleChatClient.new('ws://localhost:14141')

name = nil
black = nil
white = nil

scc.onconnect do
  puts "connected"
  scc.init(id)
end

scc.onlogin do |user|
  puts "logined"
  puts user.to_s
  name = user['name']
  scc.send_message(JSON.generate({
    "type" => "ready"
  }), ["othello"])
end

scc.onerror do |m|
  puts "error"
  puts m
end

Open3.popen2(prog_path) do |stdin, stdout, wait_thr|
  scc.onmessage do |name, message, type, tags|
    if tags.include? "othello" then
      if name == black || name == white then
        stdin.puts message if valid?(message)
        data = stdout.gets
        scc.send(data, "othello")
      elsif name == "eye" then
        obj = JSON.parse(message)
        case obj['type']
        when "command" then
          case obj['message']
          when "start" then
            black = obj['black']
            white = obj['white']
          end
        end
      end
    end
  end
  loop do
    sleep 1
  end
end
