# frozen_string_literal: true

require 'socket'
s = TCPServer.new('localhost', 3000)
c = s.accept
udpsocks = []
tcpsocks = []
next_sock = 0
puts 'Connection established'
loop do
  ch = c.getc
  next if ch.nil?

  cmd = ch.ord
  c.print [192, 168, 0, 10].pack('CCCC') if cmd.zero?
  if cmd == 1
    sock = UDPSocket.new
    sock.bind('10.0.0.180', 0)
    c.print [next_sock, sock.addr[1]].pack('L<L<')
    udpsocks[next_sock] = sock
    next_sock += 1
  end
  if cmd == 4
    info = c.read(12).unpack('L<CCCCS<S<')
    msg = c.read(info[6])
    udpsocks[info[0]].send(msg, 0, "#{info[1]}.#{info[2]}.#{info[3]}.#{info[4]}", info[5])
  end
  if cmd == 5
    id = c.read(4).unpack1('L<')
    msg = udpsocks[id].recv(65_536)
    if msg == ''
      c.print [0].pack('L<')
    else
      c.print [msg.length].pack('L<')
      c.print msg
    end
  end
  if cmd == 6
    id = c.read(4).unpack1('L<')
    udpsocks[id] = nil
  end
  if cmd == 7
    info = c.read(6).unpack('CCCCS<')
    sock = TCPSocket.new("#{info[0]}.#{info[1]}.#{info[2]}.#{info[3]}", info[4])
    c.print [next_sock].pack('L<')
    tcpsocks[next_sock] = sock
    next_sock += 1
  end
  if cmd == 8
    info = c.read(6).unpack('L<S<')
    msg = c.read(info[1])
    tcpsocks[info[0]].print msg
  end
  next unless cmd == 9

  id = c.read(4).unpack1('L<')
  if !tcpsocks[id].ready? && tcpsocks[id].closed?
    c.print [0].pack('L<')
  else
    msg = tcpsocks[id].gets
    c.print [msg.length].pack('L<')
    c.print msg
  end
end
