(defun faint-new-test ()
  "Appends boilerplate code to new Faint unit tests.

Adds the test-system includes, encoding comment and the function
to hold the checks.

The name of the test function is adapted from the file name, so
that 'test-some-stuff.cpp' gets the function 'void
test_some_stuff(){...}', as required by the test system."
  (interactive)
  (save-excursion
    (beginning-of-buffer)
    (let((testfuncname
          (replace-regexp-in-string ".cpp" ""
          (replace-regexp-in-string "-" "_"
            (replace-regexp-in-string "\\." "_" (buffer-name))))))
      (insert "// -*- coding: us-ascii-unix -*-\n")
      (insert "#include \"test-sys/test.hh\"\n\n")
      (insert "\n")
      (insert "void " testfuncname "(){\n  using namespace faint;\n\n}")))
  (goto-line 3))
