# Simple real-world rspec tests to make sure the wrapper is functioning properly
# with a real, connected device.

# These warnings are safe to ignore:
# --- get driver name: could not get bound driver: No data available
# --- Disconnected OS driver: could not set config 1: Device or resource busy

require 'rubyk8055'
include USB

describe "connecting" do
  before(:each) do
    @r = RubyK8055.new
  end
  it 'should be able to connect to a device on port 0' do
    @r.connect
    @r.connected.should == true
    @r.disconnect
    @r.connected.should == false
    @r.connect 0
    @r.board_address.should == 0
  end
end

describe RubyK8055 do
  before(:all) do
    @r = RubyK8055.new
    @r.connect
  end

  it 'should be able to read digital inputs' do
    1.upto(2) do |i|
      v = @r.get_digital(i)
      v.should >= 0 and v.should <= 1
    end
  end

  it 'should be able to set digital outputs' do
    1.upto(8) do |i|
      @r.set_digital(i, true).should == true
    end
    @r.clear_all_digital.should == true
    @r.set_all_digital.should == true
  end

  it 'should be able to write to all digital outputs at once' do
    @r.write_all_digital(231).should == true
  end

  it 'should be able to read analog inputs' do
    1.upto(2) do |i|
      v = @r.get_analog(i)
      v.should >= 0 and v.should <= 255
    end
  end

  it 'should be able to set analog outputs' do
    1.upto(2) do |i|
      @r.set_analog(i, 150).should == true
      @r.set_analog_min(i).should == true
      @r.set_analog_max(i).should == true
    end
    @r.clear_all_analog.should == true
    @r.set_all_analog.should == true
  end

  it 'should be able to return an array of all inputs' do
    inp_arr = @r.all_inputs
    inp_arr.size.should == 9
  end

  it 'should be able to return a formatted string of all inputs' do
    @r.to_s.count(";").should == 8
  end

  it 'should be able to read the counters' do
    1.upto(2) do |i|
      v = @r.read_counter(i)
      v.should >= 0
    end
  end

  it 'should be able to reset the counters' do
    1.upto(2) do |i|
      v = @r.reset_counter(i)
      v = @r.read_counter(i)
      v.should == 0
    end
  end

  it 'should be able to set the debounce time for a counter' do
    1.upto(2) do |i|
      v = @r.set_debounce(i, 750).should == true
    end
  end

  it 'should be able to clear all values and disconnect' do
    @r.clear_all_digital
    @r.clear_all_analog
    @r.disconnect
  end
end

