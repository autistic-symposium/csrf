require 'spec_helper'

shared_examples_for "strategy sql" do
  let(:parser) { described_class.new }

  before(:all) do
    @update_example = "UPDATE phpbb_sessions
                SET session_user_id = -1, session_start = 1319238613, session_time = 1319238613, session_page = 0, session_logged_in = 0
                WHERE session_id = '1'
                        AND session_ip = '7f000001'"
    @normalized_update_example = "UPDATE phpbb_sessions SET session_user_id = -1, session_start = 1319238613, session_time = 1319238613, session_page = 0, session_logged_in = 0 WHERE session_id = '1' AND session_ip = '7f000001'"
  end

  it "should respond to parse" do
    parser.private_methods.include?(:parse).should be_true
  end

  it "should receive a sql as argument" do
    parser.send(:parse, @update_example.dup)
    parser.original_sql.should == @update_example
  end

  it "should normalize the sql" do
    parser.send(:parse, @update_example.dup)
    parser.normalized_sql.should == @normalized_update_example
  end

  describe "check parsed objects" do

    before(:all) do
      @update_example = {
        sql: "UPDATE phpbb_sessions SET session_user_id = -1, session_start = 1319238613, session_time = 1319238613, session_page = 0, session_logged_in = 0 WHERE session_id = '1' AND session_ip = '7f000001'",
        type: :update,
        table_name: :phpbb_sessions,
        columns: [:session_user_id, :session_start, :session_time, :session_page, :session_logged_in].sort,
        where: "session_id=ANDsession_ip="
      }

      @insert_example = {
        sql: "INSERT INTO phpbb_sessions (session_id, session_user_id, session_start, session_time, session_ip, session_page, session_logged_in) VALUES ('322e0872deb82ca8ea0da4a24a3472a8', -1, 1319243049, 1319243049, '7f000001', 0, 0)",
        type: :insert,
        table_name: :phpbb_sessions,
        columns: [:session_id, :session_user_id, :session_start, :session_time, :session_ip, :session_page, :session_logged_in].sort,
        where: nil
      }

      @delete_example = {
        sql: "DELETE users WHERE id = 1 OR name = 'joao'",
        type: :delete,
        table_name: :users,
        columns: nil,
        where: "id=ORname="
      }

    end

    it "should parse a update query" do
      parse_query(parser, @update_example)
    end

    it "should parse a insert query" do
      parse_query(parser, @insert_example)
    end

    it "should parse a delete query" do
      parse_query(parser, @delete_example)
    end


    private

    def parse_query(parser_object, example_object)
      parser_object.send(:parse, example_object[:sql])
      parser_object.sql_type.should       == example_object[:type]
      parser_object.table_name.should     == example_object[:table_name]
      parser_object.columns.should        == example_object[:columns]
      parser_object.where.should          == example_object[:where]
    end

  end

end
