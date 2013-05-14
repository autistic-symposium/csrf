require 'spec_helper'

describe LogParser::Log do

  it "should receives a file and parses it's content" do
    log_file  = File.new('spec/fixtures/log')
    log       = LogParser::Log.new(log_file)
    # Verify wether they have the same number of lines.
    log.total_entries.should == 37
  end

  it "should open the LogEntry for the line" do
    log_file  = File.new('spec/fixtures/log')
    log       = LogParser::Log.new(log_file)
    file      = File.open('spec/fixtures/log').map {|line| line}
    # The returned line in get_entry(2) has to be the same as from the log.
    log.get_entry(2).original_line.should == file[2]
  end

end
