#!/usr/local/bin/ruby

# Sinatra http client for K8055 Interface board. Requires 'rubyk8055' and 'sinatra'.
require 'rubygems'
require 'sinatra'
require 'rubyk8055'

configure do
  # Initialize the rubyk8055 class and clear all outputs.
  $k8055 = USB::RubyK8055.new
  $k8055.connect
  $k8055.clear_all_digital
  $k8055.clear_all_analog
  $out_arr = [0]*10   # outputs (8x digital, 2x analog)
end

def _layout(msg="")
  data = $k8055.to_s

  set_digital_links = []
  8.times do |i|
    i += 1
    set_digital_links << "<a href='/set/digital/#{i}'>#{i}</a>"
  end
  clear_digital_links = []
  8.times do |i|
    i += 1
    clear_digital_links << "<a href='/clear/digital/#{i}'>#{i}</a>"
  end

  page = %Q{<html>
    <body>
      <h2>Ruby K8055 USB Interface Board - HTTP Server</h2>
      <BR>
      <center>
      <table>
        <tbody>
          <tr>
            <td> <b>all inputs</b> :: </td>
            <td> #{data} </td>
          </tr>
          <tr>
            <td> <b>digital outputs</b> :: </td>
            <td> [#{$out_arr[0,8].join(",")}] </td>
          <tr>
          </tr>
            <td> <b>analog outputs</b> :: </td>
            <td> [#{$out_arr[8,10].join(",")}] </td>
          </tr>
        </tbody>
      </table>
      </center>
      <hr>
      <BR>

      <p>Set Digital Output :: #{set_digital_links.join(" | ")}</p>
      <p>Clear Digital Output :: #{clear_digital_links.join(" | ")}</p>

      <hr>

      <form action="/set/analog" method="post">
        <P>
        <LABEL for="value_1">Channel 1 value:</LABEL>
        <INPUT type="text" name="value_1" value=#{$out_arr[8]}><BR>
        <LABEL for="value_2">Channel 2 value:</LABEL>
        <INPUT type="text" name="value_2" value=#{$out_arr[9]}><BR>
        <INPUT type="submit" value="Set Analog Inputs">
        </P>
     </form>

      <hr>

      <a href='/test'>Run Test Program</a>

      <hr>

      <BR><BR>
      <h3>~~msg~~</h3>
    </body>
  </html>
  }.gsub("~~msg~~", msg)
  return page
end

# -------------------------------------

get '/' do
  _layout
end

get '/data' do
  $k8055.to_s
end

get '/clear_all' do |n|
  $k8055.clear_all_digital
  $k8055.clear_all_analog
  $out_arr = [0]*10
  _layout "Cleared all digital and analog outputs."
end

get '/set/digital/:channel' do |channel|
  $k8055.set_digital channel.to_i, true
  $out_arr[channel.to_i - 1] = 1
  _layout "Set digital output [#{channel}] to [on]"
end

get '/clear/digital/:channel' do |channel|
  $k8055.set_digital channel.to_i, false
  $out_arr[channel.to_i - 1] = 0
  _layout "Set digital output [#{channel}] to [off]"
end

get '/set/analog/:channel/:value' do |channel, value|
  $k8055.set_analog channel.to_i, value.to_i
  $out_arr[channel.to_i + 7] = value.to_i
  _layout "Set analog output [#{channel}] to [#{value}]"
end

post '/set/analog' do
  $k8055.set_analog 1, params[:value_1].to_i
  $k8055.set_analog 2, params[:value_2].to_i
  $out_arr[8] = params[:value_1].to_i
  $out_arr[9] = params[:value_2].to_i
  _layout "Set analog outputs to specified values."
end

get '/clear/analog/:channel' do |channel|
  $k8055.set_analog channel.to_i, 0
  $out_arr[channel.to_i + 7] = 0
  _layout "Set analog output [#{channel}] to [0]"
end

get '/test' do
  2.times do
    1.upto 10 do |i|
      if i <= 8
        $k8055.set_digital i, true
        sleep 0.03
        $k8055.set_digital i, false
      else
        $k8055.set_analog_max i - 8
        sleep 0.03
        $k8055.set_analog_min i - 8
      end
    end
  end
  2.times do
    1.upto 10 do |i|
      if i <= 8
        $k8055.set_digital i, true
        sleep 0.03
      else
        $k8055.set_analog_max i - 8
        sleep 0.03
      end
    end
    1.upto 10 do |i|
      if i <= 8
        $k8055.set_digital i, false
        sleep 0.03
      else
        $k8055.set_analog_min i - 8
        sleep 0.03
      end
    end
  end
  $out_arr = [0]*10
  _layout "Ran: test program."
end

