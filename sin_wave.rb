#
# Plot sin waves
#

led_count = 10    # 10 leds
period    = 2     # 2 second period

scale = led_count/2.0
resolution = 50.0
frequency = 1.0/period
height = 2**led_count
omega = 2.0*Math::PI*frequency
slices = period * resolution
log2 = Math.log(2)

line=""
last=""
lastt=1

puts "uint16_t sin_wave[] = {"
(1..slices).each do |t|
  theta = omega * (t/resolution)
  h = (scale * Math.cos(theta) + scale).round
  
  # Now encode into binary
  value = 0
  (1..h).each do
    value = (value << 1) | 1
  end
  
  if (line.split(',').length > 8) or ((last != value) and (last != ""))
    puts "\t#{line.ljust(60)}// #{lastt}"
    line = ""
    lastt = t+1
  end
  
  item = "0x#{value.to_s(16)}"
  line = "#{line}#{item.rjust(5)}, "
  last = value
  
  #puts "t=#{t},h=#{h.round},value=#{value} (#{value.to_s(2)})"
end

puts "\t#{line.ljust(60)}// #{lastt}" if line.length
puts "\t0xFFFF"
puts "};"
