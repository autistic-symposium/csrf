require 'spec_helper'

describe LogParser::LogEntry do

  it "should initialize with a log line" do
    line      = "in GET 1     139 /phpBB2-2.0.5/viewtopic.php?sid=1&topic_id=-1 union select ord(substring(user_password,1,1)) from phpbb_users where user_id=3/*&view=newest"
    log_entry = LogParser::LogEntry.new(line)
    log_entry.original_line.should == line
  end

  it "should return the line's io type" do
    line  = "out realpath 1      46 /var/www/html/phpBB2-2.0.5/./includes/auth.php"
    entry = LogParser::LogEntry.new(line)
    entry.io_type.should == :out
  end

  it "should return the line's command type" do
    line  = "in GET 1     139 /phpBB2-2.0.5/viewtopic.php?sid=1&topic_id=-1 union select ord(substring(user_password,1,1)) from phpbb_users where user_id=3/*&view=newest"
    entry = LogParser::LogEntry.new(line)
    entry.command_type.should == :GET
  end

  it "should return the line's parameter by index (e.g. 1)" do
    line  = "in parameter 2       3       1 sid 1"
    entry = LogParser::LogEntry.new(line)
    entry.parameter(0).should == "sid"
    entry.parameter(1).should == "1"
  end

  it "should return the line's parameter by index (e.g. 2)" do
    line  = "in GET 1     139 /phpBB2-2.0.5/viewtopic.php?sid=1&topic_id=-1 union select ord(substring(user_password,1,1)) from phpbb_users where user_id=3/*&view=newest"
    entry = LogParser::LogEntry.new(line)
    entry.parameter(0).should == "/phpBB2-2.0.5/viewtopic.php?sid=1&topic_id=-1 union select ord(substring(user_password,1,1)) from phpbb_users where user_id=3/*&view=newest"
  end

  it "should return the line's parameter by index (e.g. 3)" do
    line  = "out realpath 1      46 /var/www/html/phpBB2-2.0.5/./includes/auth.php"
    entry = LogParser::LogEntry.new(line)
    entry.parameter(0).should == "/var/www/html/phpBB2-2.0.5/./includes/auth.php"
  end

end
