// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "commands/command.hh"
#include "commands/command-bunch.hh"
#include "util/merge-conditions.hh"

void test_command_merge(){
  using namespace faint;

  class CmdA : public Command{
  public:
    CmdA() : Command(CommandType::RASTER){}
    void Do(CommandContext&){}
    utf8_string Name() const{ return "CmdA"; }
  };

  class CmdB : public Command{
  public:
    CmdB() : Command(CommandType::RASTER){}
    void Do(CommandContext&){}
    utf8_string Name() const{ return "CmdB"; }
  };

  {
    // No merge condition given, no command should merge/append
    // for this command bunch.
    auto nonMerging = command_bunch(CommandType::RASTER,
      bunch_name("Test"),
      std::make_unique<CmdA>(),
      std::make_unique<CmdB>());

    NOT(nonMerging->ShouldMerge(CmdA(), true));
    NOT(nonMerging->ShouldMerge(CmdA(), false));
  }

  {
    // Commands of type CmdA should merge (well, append) once.
    auto appendA = command_bunch(CommandType::RASTER,
      bunch_name("Test"),
      std::make_unique<CmdA>(),
      std::make_unique<CmdB>(),
      append_once_if_type<CmdA>());

    VERIFY(appendA->ShouldMerge(CmdA(), true));
    NOT(appendA->ShouldMerge(CmdA(), false)); // Different frame
    NOT(appendA->ShouldMerge(CmdB(), true));

    // Test appending CmdA
    auto a = std::make_unique<CmdA>();
    appendA->Merge(std::move(a));

    // Only append once
    NOT(appendA->ShouldMerge(CmdA(), true));
  }

  {
    // Test "AssumeName"
    const auto true_f = [](const Command&) -> bool{ return true; };
    const auto bunchName = bunch_name("The bunch");

    auto assumer = command_bunch(CommandType::RASTER,
      bunchName,
      std::make_unique<CmdA>(),
      std::make_unique<CmdB>(),
      append_once_if(true_f, AssumeName::Yes));

    assumer->Merge(std::make_unique<CmdA>());
    EQUAL(assumer->Name(), CmdA().Name());
    NOT_EQUAL(assumer->Name(), bunchName.Get());

    auto keeper = command_bunch(CommandType::RASTER,
      bunch_name("The bunch"),
      std::make_unique<CmdA>(),
      std::make_unique<CmdB>(),
      append_once_if(true_f, AssumeName::No));

    keeper->Merge(std::make_unique<CmdA>());
    NOT_EQUAL(keeper->Name(), CmdA().Name());
    EQUAL(keeper->Name(), bunchName.Get());
  }
}
