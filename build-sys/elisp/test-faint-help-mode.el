;; Tests for faint-help-mode.
;;
;; Entry point: faint-test-help-mode

(require 'faint-help-mode)
(require 'faint-paths)

(defun test-within-command-at (pos)
  (save-excursion
    (goto-char pos)
    (faint-help-within-command-p)))

(defun test-faint-help-command-boundaries ()
  (with-current-buffer (generate-new-buffer "*test-faint-help*")
    (insert-file-contents-literally
     (concat (faint-elisp-test-data-dir) "/help-sample.txt"))

    (assert (equal (faint-help-command-boundaries 562) (cons 557 583)))

    ;; Just before command
    (assert (not (test-within-command-at 556)))

    ;; On backslash in \ref
    (assert (test-within-command-at 557))

    ;; Within parenthesis
    (assert (test-within-command-at 566))

    ;; On final character
    (assert (test-within-command-at 582))

    ;; Just past command
    (assert (not (test-within-command-at 583)))))

(defun faint-test-help-mode()
  (interactive)
  (test-faint-help-command-boundaries)
  (message "OK"))

(defun faint-print-help-within-command ()
  (interactive)
  (print (faint-help-within-command-p)))
