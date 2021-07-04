#! /usr/bin/env ruby
# frozen_string_literal: true
require 'tempfile'
usbdisks = `for devlink in /dev/disk/by-id/usb*; do readlink -f ${devlink}; done`
raws = usbdisks.split("\n")
parts = []
raws.each do |disk|
  parts.push disk if /(\w+)\d+/.match disk
end
raws.each do |disk|
  raws.delete("/dev/#{$1}") if /(\w+)\d+/.match disk
end
if raws.length == 0
  puts 'No disks detected. Aborting.'
  exit 1
end
if parts.length > 1
  puts 'Multiple partitions detected.'
  i = 0
  parts.each do |part|
    puts "#{i}: #{part}"
    i += 1
  end
  print 'Please choose partition to install to:'
  num = gets.chomp.to_i
  part = parts[num]
  /(\w+)\d+/.match part
  raw = "/dev/#{$1}"
elsif parts.length == 0
  puts 'No partitions detected.'
  i = 0
  raws.each do |raw|
    puts "#{i}: #{raw}"
    i += 1
  end
  print 'Please choose disk to install to:'
  num = gets.chomp.to_i
  raw = raws[num]
  print 'Getting disk size...'
  origsize = `sudo blockdev --getsize64 #{raw}`.chomp.to_i
  unit = 'MB'
  fullsize = (origsize / 1024.to_f) / 1024
  if (fullsize % 1024 == 0) || (fullsize > 2048)
    fullsize = fullsize / 1024.to_f
    unit = 'GB'
  end
  fullsize = fullsize.floor(1)
  puts "#{fullsize}#{unit}"
  print 'Partition size: (return for whole disk)'
  size = gets.chomp.downcase
  if size.match(/(\d)+m/)) || size.match(/(\d+)mb/))
    size = size.to_i * 1024 * 1024
  elsif size.match(/(\d)+g/)) || size.match(/(\d+)gb/))
    size = size.to_i * 1024 * 1024 * 1024
  elsif size == ''
    size = origsize
  else
    puts 'Could not parse size. Aborting.'
    exit 1
  end
  sfdisk_input = "label: dos\n- #{size / 1024}KiB L"
  tmp = Tempfile.new('myosinstall')
  tmp.puts sfdisk_input
  tmp.close
  print 'Partitioning...'
  `cat #{tmp.path} | sudo sfdisk #{raw} 2>/dev/null`
  puts 'Done'
  tmp.unlink
  exit 1
  puts `sudo mkfs.ext2 #{raw}`
  part = "#{raw}1"
  raw = raw.to_s
else
  part = parts[0]
  /(\w+)\d+/.match parts[0]
  raw = "/dev/#{$1}"
end
`mkdir -p usb`
print 'Mounting the disk...'
`sudo mount #{part} usb`
puts 'Done'
if !File.exist?('usb/boot')
  puts 'Installing GRUB...'
  `sudo grub-install --boot-directory=usb/boot #{raw}`
end
print 'Installing the OS...'
`sudo cp -r /vagrant/sysroot/* usb`
puts 'Done'
print 'Umounting disk...'
`sudo umount usb`
puts 'Done'
