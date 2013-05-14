require 'spec_helper'

describe Runtime do

  it "should run" do
    Runtime.respond_to?(:run).should be_true
  end

end
