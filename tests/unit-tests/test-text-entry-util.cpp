// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "tests/test-util/print-objects.hh"

#include "editors/text-entry-util.hh"
#include "text/char-constants.hh"
#include "text/text-buffer.hh"
#include "util-wx/key-codes.hh"

void test_text_entry_util(){
  using namespace faint;
  {
    TextBuffer text("Hello world");
    text.caret(text.size());
    EQUAL(text.get()[0], utf8_char("H"));
    EQUAL(text.get()[10], utf8_char("d"));

    // Enter "a"
    VERIFY(handle_key_press(KeyPress(None, key::A, utf8_char("a")), text));
    EQUAL(text.get()[10], utf8_char("d")); // Previous unmodified
    EQUAL(text.get()[11], utf8_char("a")); // New

    // Invalid character (null)
    VERIFY(!handle_key_press(KeyPress(None, key::A, chars::utf8_null), text));
    EQUAL(text.get()[11], utf8_char("a")); // Unmodified

    // Invalid character (replacement)
    VERIFY(!handle_key_press(KeyPress(None, key::A,
          chars::replacement_character), text));
    EQUAL(text.get()[11], utf8_char("a"));

    // Navigation
    VERIFY(handle_key_press(KeyPress(None, key::up, chars::utf8_null), text));
    EQUAL(text.caret(), 0);
    EQUAL(text.get()[11], utf8_char("a"));

    VERIFY(handle_key_press(KeyPress(None, key::down, chars::utf8_null), text));
    EQUAL(text.caret(), 12);
    EQUAL(text.get()[11], utf8_char("a"));

    // Selection
    VERIFY(handle_key_press(KeyPress(Shift, key::left, chars::utf8_null), text));
    EQUAL(text.caret(), 11);
    EQUAL(text.get_sel_range().from, 11);
    EQUAL(text.get_sel_range().to, 12);

    // Transpose
    EQUAL(text.get(), "Hello worlda");
    text.caret(5);
    VERIFY(handle_key_press(KeyPress(Ctrl, key::T, chars::utf8_null), text));
    EQUAL(text.get(), "Hell oworlda");
  }

  {
    TextBuffer text("Hello world");
    text.caret(text.size());
    VERIFY(handle_key_press(KeyPress(None, key::enter, chars::eol), text));
    EQUAL(text.size(), 12);
    EQUAL(text.get(), "Hello world\n");
  }

  {
    // Select all, no text
    TextBuffer text("");
    EQUAL(text.all(), CaretRange(0,0));
    text.select(text.all());
    text.del();
  }
}
