# Simple real-world rspec tests to make sure the wrapper is functioning properly
# with a real, connected device.

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
  end
end

describe RubyK8055 do
  before(:each) do
    @r = RubyK8055.new
  end
  it 'should be able to connect to a device on port 0' do
    @r.connect
    @r.connected.should == true
    @r.disconnect
    @r.connected.should == false
    @r.connect 0
  end
end

