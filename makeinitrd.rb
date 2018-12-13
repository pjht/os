folder=ARGV[0]
output=ARGV[1]
outf=File.open(output,"w")
for name in Dir.glob("#{folder}/*")
  name=File.basename(name)
  contents=File.read("#{folder}/#{name}")
  outf.print [name.length].pack("L")
  outf.print name
  outf.print "\0"
  outf.print [contents.length].pack("L")
  outf.print contents
end
outf.print [0].pack("L")
outf.close
