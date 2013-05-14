require 'spec_helper'

describe LogParser::WhiteList do

  before(:each) do
    require 'fileutils'
    FileUtils.cp 'spec/fixtures/white_list', 'spec/fixtures/white_list2'
  end

  let(:white_list) { LogParser::WhiteList.new('spec/fixtures/white_list2') }

  it "should be false for a non-listed sql" do
    sql = LogParser::SqlInfo.new("UPDATE phpbb_sessions SET session_user_id = -1, session_start = 1319238613, session_time = 1319238613, session_page = 0, session_logged_in = 0 WHERE session_id = '1' AND session_ip = '7f000001'")
    white_list.listed?(sql).should be_false
  end

  it "should be true for a listed sql" do
    sql = LogParser::SqlInfo.new("UPDATE users SET session_user_id = -1, session_start = 1319238613, session_time = 1319238613, session_page = 0, session_logged_in = 0 WHERE session_id = '1'")
    white_list.listed?(sql).should be_true
  end

  it "should add new queries" do
    sql = LogParser::SqlInfo.new("UPDATE phpbb_sessions SET session_user_id = -1, session_start = 1319238613, session_time = 1319238613, session_page = 0, session_logged_in = 0 WHERE session_id = '1' AND session_ip = '7f000001'")
    white_list.add!(sql)
    white_list.listed?(sql).should be_true
  end

  it "should write to file when adding new queries" do
    sql = LogParser::SqlInfo.new("UPDATE phpbb_sessions SET session_user_id = -1, session_start = 1319238613, session_time = 1319238613, session_page = 0, session_logged_in = 0 WHERE session_id = '1' AND session_ip = '7f000001'")
    white_list.add!(sql)
    new_white_list = LogParser::WhiteList.new('spec/fixtures/white_list2')
    new_white_list.listed?(sql).should be_true
  end

end
