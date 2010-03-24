require 'rubyk8055'
include USB
@r = RubyK8055.new
@r.connect

delay = 0.025

while true
  2.times do
    1.upto 10 do |i|
      if i <= 8
        @r.set_digital i, true
        sleep delay
        @r.set_digital i, false
      else
        @r.set_analog_max i - 8
        sleep delay
        @r.set_analog_min i - 8
      end
    end
  end

  2.times do
    1.upto 10 do |i|
      if i <= 8
        @r.set_digital i, true
        sleep delay
      else
        @r.set_analog_max i - 8
        sleep delay
      end
    end
    @r.clear_all_digital
    @r.clear_all_analog
  end

  2.times do
    10.downto 1 do |i|
      if i <= 8
        @r.set_digital i, true
        sleep delay
        @r.set_digital i, false
      else
        @r.set_analog_max i - 8
        sleep delay
        @r.set_analog_min i - 8
      end
    end
  end

  5.times do
    1.upto 10 do |i|
      if i <= 8
        @r.set_digital i, true
        sleep delay
      else
        @r.set_analog_max i - 8
        sleep delay
      end
    end

    1.upto 10 do |i|
      if i <= 8
        @r.set_digital i, false
        sleep delay
      else
        @r.set_analog_min i - 8
        sleep delay
      end
    end
  end

end

