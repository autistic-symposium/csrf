require 'spec_helper'

describe LogParser::SqlInfo do

  before(:each) do
    @update_1 = "UPDATE phpbb_sessions SET session_user_id = -1, session_start = 1319238613, session_time = 1319238613, session_page = 0, session_logged_in = 0 WHERE session_id = '1' AND session_ip = '7f000001'"
    @update_2 = "UPDATE phpbb_sessions SET session_user_id = -10, session_start = 1319238613, session_time = 1319238613, session_page = 0, session_logged_in = 0 WHERE session_id = '2' AND session_ip = '7f000001'"
    @update_3 = "UPDATE phpbb_sessions SET user_id = -10, session_start = 1319238613, session_time = 1319238613, session_page = 0, session_logged_in = 0 WHERE session_id = '2' AND session_ip = '7f000001'"
  end

  it "should return true when comparing the same sql structure" do
    parser1 = described_class.new
    parser1.send(:parse, @update_1)
    parser2 = described_class.new
    parser2.send(:parse, @update_2)
    parser1.should == parser2
  end

  it "should return true when comparing the same sql structure" do
    parser1 = described_class.new
    parser1.send(:parse, @update_1)
    parser2 = described_class.new
    parser2.send(:parse, @update_3)
    parser1.should_not == parser2
  end

end
