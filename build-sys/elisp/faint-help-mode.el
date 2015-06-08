;; Major mode for editing Faint-graphics-editor help-source-files.

(provide 'faint-help-mode)


(defun faint-help-command-boundaries (pos)
  "Returns the (start . end)-boundaries of the faint-help-command
at POS, if within a command, else nil.

Faint-help-commands look like this: \commandname(args)"
  (save-excursion
    (goto-char (+ pos 1))
    ;; Find the opening back-slash
    (if (search-backward "\\" 0 t)
        (let ((start (point)))
          ;; Find the argument list
          (if (looking-at "\\\\[a-z]+?(.*?)")
              (cons start (match-end 0))
            nil))
      nil)))


(defun faint-help-within-command-p ()
  "t if POINT is within a faint-help command"
  (let ((bounds (faint-help-command-boundaries (point))))
    (if bounds
        (and (<= (car bounds) (point)) (< (point) (cdr bounds)))
      nil)))


(defun faint-help-fill-no-break-p ()
  "Predicate for text segments that should not be split
in fill-paragraph in faint-help-mode."
  (faint-help-within-command-p))


(define-derived-mode faint-help-mode text-mode
  "Faint help mode"
  "Major mode for Faint-graphics-editor help-files."

  ;; Do not break within commands like \ref(some-reference) with
  ;; fill-paragraph (M-q)
  (add-hook 'fill-nobreak-predicate 'faint-help-fill-no-break-p nil t)

  (set (make-local-variable 'paragraph-start) "\f\\|[ \t]*$")

  ;; Consider headings as paragraph separators
  (set (make-local-variable 'paragraph-separate) "\\(=.*?=$\\)\\|[ \t\f]*$"))


;; Match help-files to this mode
(add-to-list 'auto-mode-alist '("\\help/source/.*.txt\\'" . faint-help-mode))
